// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <pthread.h>

#include <chrono>
#include <config.hpp>
#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <opencv2/opencv.hpp>
#include <performance.hpp>
#include <util/util.hpp>

#include "util/arg_parser.hpp"
#include "util/tqdm.h"
using namespace cv;
using namespace std;

tqdm bar;
int frame_rate, frame_count;
Mat kernel, large_kernel, first_frame, prev_opt, bg_img;

vector<Point> start_points = vector<Point>();

void show_usage(string name);
bool handle_arguments(int argc, char* argv[]);
void initialize_elements(VideoCapture& cap, runtime_params& params);

void run(runtime_params& params, density_t& density);

int main(int argc, char* argv[]) {
  if (!handle_arguments(argc, argv)) {
    show_usage(argv[0]);
    return -1;
  }

  auto method = stoi(arg_parser.get_argument_value("method"));
  if (method != 0) {
    perform_analysis(run, method);
  } else {
    auto params = runtime_params{};
    auto density = density_t();

    run(params, density);

    outputCSV(density, frame_rate);
  }

  return 0;
}

bool handle_arguments(int argc, char* argv[]) {
  arg_parser.set_argument("input", "i", "./input_files/trafficvideo.mp4");
  arg_parser.set_argument("output", "o", "./output_files/density.csv");
  arg_parser.set_argument("method", "m", "0");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");
  arg_parser.set_standalone_argument("skip-initial", "s");
  arg_parser.set_standalone_argument("train", "t");

  return arg_parser.parse_arguments(argc, argv);
}
void show_usage(string name) {
  cerr << "Usage: " << name << "\n"
       << "Options:\n"
       << "\t-h, --help\t\tShow this help message\n"
       << "\t-i, --input\t\tSpecify the input file path. Default is "
          "./input_files/trafficvideo.mp4\n"
       << "\t-o, --output\t\tSpecify the output csv file. Default is "
          "./output_files/density.csv\n"
       << "\t-a, --autoselect-points \tSelect second set of points "
          "automatically\n"
       << "\t-d, --debug \t\tDisplay debug output\n"
       << "\t-f, --no-animation \t\tDo not display animation\n"
       << "\t-s, --skip-initial \t\tSkip initial selection of points\n"
       << "\t-t, --train-bg \t\tAuto train the background of video\n"
       << "\t-m, --method \t\tRuntime analysis method {1,2,3,4,5}\n"

       << endl;
}

void initialize_elements(VideoCapture& cap, runtime_params& params,
                         cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub) {
  string file_name = arg_parser.get_argument_value("input");
  cout << "[+] Loading File: " << file_name << endl;
  cap.open(file_name);
  if (!cap.isOpened()) {
    cerr << "[-] Unable to open the video" << endl;
    throw "Could not load video";
  }

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

  int frame_idx = 0;
  while (true) {

    for (int i = 0; i < args->wait_for_threads; i++) {
      sem_wait(args->consumer_ready);
    }

    bar.progress(frame_idx, frame_count);

    Mat frame;
    cap.read(frame);

    *frame_ptr = frame;

    for (int i = 0; i < args->wait_for_threads; i++) {
      sem_post(args->producer_ready);
    }
    frame_idx++;
    if (frame.empty()) {
      break;
    }
  }

  for (int i = 0; i < args->num_threads; i++) {
    sem_post(args->producer_ready);
    sem_post(args->sem_exit);
  }
  cout << "[#] Producer exiting" << endl;
  pthread_exit(NULL);
}

void* worker(void* arg) {
  struct worker_params* args = (struct worker_params*)arg;

  int fp = 0;
  auto params = args->params;
  auto num_threads = params->split_video;
  auto num_splits = params->split_frame;
  auto frames_processed = (num_splits == 1) ? args->frames_processed : &fp;
  // auto mutex_lock = args->mutex_lock;
  auto consumer_ready = args->consumer_ready;
  auto producer_ready = args->producer_ready;
  auto sem_exit = args->sem_exit;

  auto density_lock = args->density_lock;
  auto bg_sub = args->frame_get.bg_sub;
  auto cropping_rect = *(args->frame_get.cropping_rect);
  // auto cap = *(args->frame_get.cap);
  auto frame_ptr = args->frame_get.frame_ptr;

  auto pid = pthread_self();

  Mat last_frame;
  while (true) {
    // if (num_threads != 1)
    // pthread_mutex_lock(mutex_lock);

    Mat frame, cropped_frame;
    sem_wait(producer_ready);
    frame = *frame_ptr;
    // cap.read(frame);
    int i = ++(*frames_processed);
    sem_post(consumer_ready);


    // if (num_threads != 1)
    // pthread_mutex_unlock(mutex_lock);

    if (frame.empty() || i > frame_count) {
      printf("[#] Worker %ld breaking from loop\n", pid);
      // break;
      break;
    }

    if (i % (1 + params->skip_frames) != 0) {
      continue;
    }

    preprocess_frame(frame, params->resolution);
    crop_frame(frame, cropped_frame, cropping_rect);

    // TODO : bg_sub is applied after cropping end points as well as cropping
    // the frame if split_frame is used
    Mat fg_mask;
    bg_sub->apply(cropped_frame, fg_mask, learning_rate);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }

    reduce_noise(fg_mask, kernel);

    Mat new_opt, dynamic_img;
    pair<double, double> density_point;
    if (params->calc_dynamic_density) {
      // TODO check if this block works
      if (i < num_threads) {
        last_frame = frame;
      }
      opticalFlow(last_frame, frame, new_opt);

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
    if (num_splits != 1) pthread_mutex_lock(density_lock);

    args->density_store->at(i).first += density_point.first;
    args->density_store->at(i).second += density_point.second;

    // printf("%ld, density_point: (%f, %f), density_store[%d]: (%f, %f)\n",
    // pid, density_point.first, density_point.second, i,
    // args->density_store->at(i).first, args->density_store->at(i).second);

    if (num_splits != 1) pthread_mutex_unlock(density_lock);

    // if (arg_parser.get_bool_argument_value("debug")) {
    //   imshow("Dynamic Density", dynamic_img);
    //   imshow("Noise Reduction", fg_mask);
    //   if (waitKey(1) == 'n') {
    //     break;
    //   }
    // }
    
    
  
  }

  sem_wait(sem_exit);
  cout << "Worker exiting: " << pid << endl;
  pthread_exit(NULL);
}

void run(runtime_params& params, density_t& density) {
  vector<Rect2d> cropping_rects;
  // vector<VideoCapture> caps;
  vector<cv::Ptr<cv::BackgroundSubtractorMOG2>> bg_subs;
  vector<worker_params> thread_params;

  VideoCapture cap;
  cv::Ptr<cv::BackgroundSubtractorMOG2> bg_sub;
  make_scaled_rects(params.split_frame, cropping_rects);
  initialize_elements(cap, params, bg_sub);

  Mat fg_mask, frame;
  cout << "[+] Training BG subtractor ..." << endl;
  if (arg_parser.get_bool_argument_value("train")) {
    // TODO modify train_bgsub to take cropped frame
    // train_bgsub(bg_sub, cap, fg_mask, params.resolution);
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

  // for (int i = 0; i < params.split_frame; i++) {
  //   auto new_cap(cap);
  //   caps.push_back(new_cap);
  // }

  // For Multithreading, 2 approaches may be used:
  // Either run 4 threads doing the same thing but on diff data
  // Or storing pre-processed frames in some array and then supplying it to each
  // of 3 threads
  int frames_processed = 0;
  density.resize(frame_count);

  worker_params w_params;
  w_params.params = &params;
  w_params.density_store = &density;

  // auto mutex_lock = pthread_mutex_t();
  sem_t producer_ready, consumer_ready, sem_exit;
  auto density_lock = pthread_mutex_t();

  // w_params.mutex_lock = &mutex_lock;
  w_params.density_lock = &density_lock;
  w_params.consumer_ready = &consumer_ready;
  w_params.producer_ready = &producer_ready;
  w_params.sem_exit = &sem_exit;

  w_params.frames_processed = &frames_processed;

  for (int i = 0; i < params.split_frame; i++) {
    auto new_w_param(w_params);
    new_w_param.frame_get =
        worker_params::frame_getter{&cropping_rects[i], bg_subs[i], &frame};
    thread_params.push_back(new_w_param);
  }

  producer_params prod_params{
      &cap, &frame, &consumer_ready, &producer_ready, &sem_exit, 1, 1};
  // if (params.split_video == 1 && params.split_frame == 1) {
  //   cout << "[+] Running in single threaded mode" << endl;

  //   sem_init(&producer_ready, 0, 0);
  //   sem_init(&consumer_ready, 0, 1);
  //   auto err = pthread_create(&prod_thread, NULL, producer, (void
  //   *)&prod_params); if(err != 0){
  //     cout<<"[-] Cannot start producer thread"<<endl;
  //     return;
  //   }

  //   worker((void*)&thread_params[0]);
  //   pthread_join(prod_thread, NULL);

  // } else {
  // TODO here split_video is given priority
  bool video_split = params.split_video != 1;
  int num_threads = video_split ? params.split_video : params.split_frame;

  prod_params.wait_for_threads = (video_split) ? 1 : num_threads;
  prod_params.num_threads = num_threads;
  // if (pthread_mutex_init(&mutex_lock, NULL) != 0) {
  //   printf("[-] Mutex init has failed\n");
  //   return;
  // }
  sem_init(&producer_ready, 0, 0);
  sem_init(&sem_exit, 0, 0);
  sem_init(&consumer_ready, 0, (video_split) ? 1 : num_threads);

  if (pthread_mutex_init(&density_lock, NULL) != 0) {
    printf("[-] Mutex init has failed\n");
    return;
  }

  pthread_t tid[num_threads], prod_thread;
  int ii = 0;
  int error;

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
  if (!video_split) {
    for (auto& v : density) {
      v.first /= num_threads;
      v.second /= num_threads;
    }
  }

  // pthread_mutex_destroy(&mutex_lock);
  sem_destroy(&producer_ready);
  sem_destroy(&consumer_ready);
  sem_destroy(&sem_exit);
  pthread_mutex_destroy(&density_lock);
  // }

  bar.finish();
  cap.release();
  cv::destroyAllWindows();
  start_points.clear();
}