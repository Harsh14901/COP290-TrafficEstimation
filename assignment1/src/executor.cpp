#include <executor.hpp>

tqdm bar;
Mat kernel, large_kernel, prev_opt, bg_img;
vector<cv::Ptr<cv::BackgroundSubtractorMOG2>> bg_subs;
vector<Point> start_points = vector<Point>();
int frame_rate = 0;
int frame_count = 0;

void initialize_elements(VideoCapture& cap, runtime_params& params,
                         cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub) {
  string file_name = arg_parser.get_argument_value("input");
  cout << "[+] Loading File: " << file_name << endl;
  cap.open(file_name);
  if (!cap.isOpened()) {
    cerr << "[-] Unable to open the video" << endl;
    throw "Could not load video";
  }
  Mat first_frame;
  cap.read(first_frame);

  select_start_points(first_frame, params.resolution);

  frame_rate = cap.get(CAP_PROP_FPS);
  frame_count = cap.get(CAP_PROP_FRAME_COUNT);

  if (arg_parser.get_bool_argument_value("debug")) {
    cout << "[+] Frame rate is: " << frame_rate << endl;
    cout << "[+] Frame count is: " << frame_count << endl;
  }

  bg_img = imread("./input_files/empty.jpg", IMREAD_UNCHANGED);

  if (bg_img.empty() && !arg_parser.get_bool_argument_value("train")) {
    cerr << "[-] Unable to load the background file ./input_files/empty.jpg"
         << endl;
    throw "Background Image not found";
  }

  bg_sub = createBackgroundSubtractorMOG2(
      int(frame_count * training_percent / 100.0), 8.0);

  kernel = getStructuringElement(MORPH_ELLIPSE, kernel_shape);
  large_kernel = getStructuringElement(MORPH_ELLIPSE, large_kernel_shape);

  bar.set_theme_line();
}

void* producer(void* arg) {
  auto args = (struct producer_params*)arg;
  auto cap = *(args->cap_ptr);
  auto frame_ptr = args->frame_ptr;
  auto params = args->params;
  auto cropping_rects = args->cropping_rects;
  int div_num = cropping_rects.size();
  Mat frame, cropped_frame;

  while (true) {
    sem_wait(args->consumer_ready);

    bar.progress(*(args->frames_processed), frame_count);

    *(args->frame_div) = *(args->frame_div) + 1;

    if (*(args->frame_div) == div_num) {
      *(args->frame_div) = 0;
      *(args->frames_processed) = *(args->frames_processed) + 1;
    }

    if (*(args->frame_div) == 0) {
      cap.read(frame);
      if (!frame.empty() && params->split_video == 1) {
        preprocess_frame(frame, params->resolution);
      }
    }
    if (!frame.empty()) {
      crop_frame(frame, cropped_frame, cropping_rects[*(args->frame_div)]);
      *frame_ptr = cropped_frame;

    } else {
      *frame_ptr = frame;
    }
    sem_post(args->producer_ready);

    if (frame.empty()) {
      break;
    }
  }

  for (int i = 0; i < args->num_threads; i++) {
    sem_post(args->producer_ready);
    sem_post(args->sem_exit);
  }
  pthread_exit(NULL);
}

void* worker(void* arg) {
  struct worker_params* args = (struct worker_params*)arg;

  auto params = args->params;
  auto num_threads = params->split_video;
  auto consumer_ready = args->consumer_ready;
  auto producer_ready = args->producer_ready;
  auto sem_exit = args->sem_exit;
  auto frame_ptr = args->frame_ptr;

  Mat last_frame;
  while (true) {
    Mat frame;

    sem_wait(producer_ready);

    frame = *frame_ptr;
    int i = *(args->frames_processed);
    int div = *(args->frame_div);

    sem_post(consumer_ready);

    if (frame.empty() || i >= frame_count) {
      break;
    }

    if (i % (1 + params->skip_frames) != 0) {
      args->density_store->at(i) = args->density_store->at(i - 1);
      continue;
    }
    if (params->split_video != 1) {
      preprocess_frame(frame, params->resolution);
    }

    Mat fg_mask;
    bg_subs[div]->apply(frame, fg_mask, learning_rate);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }

    reduce_noise(fg_mask, kernel);

    Mat new_opt, dynamic_img;
    pair<double, double> density_point;
    if (params->calc_dynamic_density) {
      if (last_frame.rows == 0) {
        last_frame = frame;
      }
      if (params->sparse_optical_flow) {
        sparse_optical_flow(last_frame, frame, fg_mask, new_opt,
                          num_threads * params->skip_frames);
      } else {
        opticalFlow(last_frame, frame, new_opt);
      }

      last_frame = frame;
      if (i < num_threads) {
        prev_opt = new_opt;
      }

      bitwise_and(new_opt, prev_opt, dynamic_img);
      prev_opt = new_opt;

      density_point = compute_density(fg_mask, dynamic_img);
    } else {
      density_point = compute_density(fg_mask);
    }

    args->density_store->at(i).first += density_point.first;
    args->density_store->at(i).second += density_point.second;
  }

  sem_wait(sem_exit);
  pthread_exit(NULL);
}

void run(runtime_params& params, density_t& density) {
  vector<Rect2d> cropping_rects;
  vector<worker_params> thread_params;

  VideoCapture cap;
  cv::Ptr<cv::BackgroundSubtractorMOG2> bg_sub;
  make_scaled_rects(params.split_frame, cropping_rects);
  initialize_elements(cap, params, bg_sub);

  Mat fg_mask, frame;
  cout << "[+] Training BG subtractor ..." << endl;
  if (arg_parser.get_bool_argument_value("train")) {
    for (int i = 0; i < params.split_frame; i++) {
      train_bgsub(bg_sub, cap, fg_mask, params.resolution, cropping_rects[i]);
    }

  } else {
    for (int i = 0; i < params.split_frame; i++) {
      Mat cropped_bg_img;
      crop_frame(bg_img, cropped_bg_img, cropping_rects[i]);
      auto new_bg_sub(bg_sub);
      train_static_bgsub(new_bg_sub, cropped_bg_img, fg_mask,
                         params.resolution);
      bg_subs.push_back(new_bg_sub);
    }
  }

  cap.set(CAP_PROP_POS_FRAMES, 0);
  int frames_processed = 0, frame_div = -1;
  bool video_split = params.split_video != 1;
  int num_threads = video_split ? params.split_video : params.split_frame;
  sem_t producer_ready, consumer_ready, sem_exit;
  vector<density_t> density_store;

  density.resize(frame_count);

  worker_params w_params{&params,         &frames_processed, &frame_div,
                         &consumer_ready, &producer_ready,   &sem_exit,
                         &frame};

  for (int i = 0; i < params.split_frame; i++) {
    auto new_w_param(w_params);
    density_t density_div(frame_count);
    density_store.push_back(density_div);

    new_w_param.density_store = &density_store.back();
    thread_params.push_back(new_w_param);
  }

  producer_params prod_params{
      &cap,           &frame,      &consumer_ready,   &producer_ready,
      &sem_exit,      num_threads, &frames_processed, &frame_div,
      cropping_rects, &params};

  sem_init(&producer_ready, 0, 0);
  sem_init(&sem_exit, 0, 0);
  sem_init(&consumer_ready, 0, 1);

  pthread_t tid[num_threads], prod_thread;
  int ii = 0, error;

  error = pthread_create(&prod_thread, NULL, producer, (void*)&prod_params);
  if (error != 0) {
    cout << "[-] Cannot start producer thread, error code: " << to_string(error)
         << endl;
    return;
  }

  cout << "[+] Creating threads" << endl;
  while (ii < num_threads) {
    auto worker_args = (video_split) ? thread_params[0] : thread_params[ii];
    error = pthread_create(&(tid[ii]), NULL, &worker, (void*)&worker_args);
    if (error != 0)
      printf("[-] Consumer thread can't be created :[%s]", strerror(error));
    ii++;
    cout << "[+] Created consumer thread " << ii << endl;
  }

  pthread_join(prod_thread, NULL);

  for (int ii = 0; ii < num_threads; ii++) {
    pthread_join(tid[ii], NULL);
  }
  cout << "[+] Consumer threads joined successfully" << endl;
  if (!video_split) {
    for (int ii = 0; ii < frame_count; ii++) {
      double x_sum = 0.0, y_sum = 0.0;
      for (int j = 0; j < num_threads; j++) {
        x_sum += density_store[j][ii].first;
        y_sum += density_store[j][ii].second;
      }
      density[ii] = make_pair(x_sum / num_threads, y_sum / num_threads);
    }

  } else {
    density = density_store[0];
  }

  sem_destroy(&producer_ready);
  sem_destroy(&consumer_ready);
  sem_destroy(&sem_exit);

  bar.finish();
  cap.release();
  cv::destroyAllWindows();
  start_points.clear();
  bg_subs.clear();
}