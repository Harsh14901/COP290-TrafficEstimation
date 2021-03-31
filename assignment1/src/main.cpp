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
vector<cv::Ptr<cv::BackgroundSubtractorMOG2>> bg_subs;

auto start = std::chrono::steady_clock::now();

double get_time() {
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

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
  auto complete_anly = arg_parser.get_bool_argument_value("complete_analysis");
  if (complete_anly) {
    complete_analysis(run);
  }
  if (method != 0) {
    perform_analysis(run, method);
  } else {
    runtime_params params;
    params.read_config_file(arg_parser.get_argument_value("config"));

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
  arg_parser.set_argument("config", "c", "./configs/config");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");
  arg_parser.set_standalone_argument("skip-initial", "s");
  arg_parser.set_standalone_argument("train", "t");
  arg_parser.set_standalone_argument("complete_analysis", "y");

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
  auto params = args->params;
  auto cropping_rects = args->cropping_rects;
  // auto frames_processed = args->frames_processed;
  // auto frame_div = args->frame_div;
  // auto div_num = args->div_num;
  int div_num = cropping_rects.size();
  // int frame_div = 0;
  Mat frame, cropped_frame;

  // *frame_div = -1;

  // auto refill = [&]() {
  //   cap.read(frame);
  //   if (!frame.empty()) {
  //     preprocess_frame(frame, params->resolution);
  //     crop_frame(frame, cropped_frame, cropping_rects[frame_div++]);
  //     if (frame_div == div_num) {
  //       frame_div = 0;
  //       *frames_processed = *frames_processed + 1;
  //     }
  //     *frame_ptr = cropped_frame;

  //   } else {
  //     *frame_ptr = frame;
  //   }

  // };

  while (true) {
    // printf("[=] Producer waiting for consumers at : %f s\n", get_time());
    // for (int i = 0; i < args->wait_for_threads; i++) {
    sem_wait(args->consumer_ready);
    // }
    // printf("[=] All consumers ready at : %f s\n", get_time());

    bar.progress(*(args->frames_processed), frame_count);

    // if (!frame.empty() && args->wait_for_threads != 1) {
    //   preprocess_frame(frame, args->params->resolution);
    // }
    // refill();
    *(args->frame_div) = *(args->frame_div) + 1;
    // printf("[=] Producer, set frame_div to: %d\n", *(args->frame_div));

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
    // printf("[=] Producer, frames processed are: %d\n",
    // *(args->frames_processed));

    // printf("[=] Producer posting as ready : %f s\n", get_time());

    // for (int i = 0; i < args->wait_for_threads; i++) {
    sem_post(args->producer_ready);
    // }

    // printf("[=] Producer ready : %f s\n", get_time());

    // frame_idx++;

    if (frame.empty()) {
      break;
    }
  }

  for (int i = 0; i < args->num_threads; i++) {
    sem_post(args->producer_ready);
    sem_post(args->sem_exit);
  }
  // cout << "[#] Producer exiting" << endl;
  pthread_exit(NULL);
}

void* worker(void* arg) {
  struct worker_params* args = (struct worker_params*)arg;

  auto params = args->params;
  auto num_threads = params->split_video;
  // auto num_splits = params->split_frame;
  // auto frames_processed = args->frames_processed;
  // auto frame_div = args->frame_div;
  auto consumer_ready = args->consumer_ready;
  auto producer_ready = args->producer_ready;
  auto sem_exit = args->sem_exit;

  // auto density_lock = args->density_lock;
  // auto bg_sub = args->bg_sub;
  // auto bg_subs = args->bg_subs;
  // auto cropping_rect = *(args->frame_get.cropping_rect);
  auto frame_ptr = args->frame_ptr;

  auto pid = pthread_self();

  Mat last_frame;
  while (true) {
    Mat frame;
    // printf("[#] Thread %ld: waiting for producer at time : %f s\n",
    // pid,get_time());

    sem_wait(producer_ready);
    // printf("[#] Thread %ld: producer ready : %f s\n", pid,get_time());

    frame = *frame_ptr;
    int i = *(args->frames_processed);
    int div = *(args->frame_div);
    // printf("[#] Consumer %ld processsing frame %d, div:%d\n", pid, i, div);
    // printf("[#] Thread %ld: posting to consumer ready at time : %f s\n",
    // pid,get_time());

    sem_post(consumer_ready);

    // printf("[#] Thread %ld: consumer ready at time : %f s\n",
    // pid,get_time());

    if (frame.empty() || i >= frame_count) {
      // printf("[#] Worker %ld breaking from loop\n", pid);
      break;
    }

    if (i % (1 + params->skip_frames) != 0) {
      args->density_store->at(i) = args->density_store->at(i - 1);
      continue;
    }
    // if(num_threads != 1 || (num_threads == 1 && num_splits == 1)){
    // preprocess_frame(frame, params->resolution);
    // }
    // else {
    // crop_frame(frame, cropped_frame, cropping_rect);

    // }
    if (params->split_video != 1) {
      preprocess_frame(frame, params->resolution);
    }
    // TODO : bg_sub is applied after cropping end points as well as cropping
    // the frame if split_frame is used
    Mat fg_mask;
    // cout<<"SEG FAULT START"<<endl;
    bg_subs[div]->apply(frame, fg_mask, learning_rate);
    // cout<<"SEG FAULT END"<<endl;

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }

    reduce_noise(fg_mask, kernel);

    Mat new_opt, dynamic_img;
    pair<double, double> density_point;
    if (params->calc_dynamic_density) {
      // TODO check if this block works
      if (last_frame.rows == 0) {
        last_frame = frame;
      }
      if (params->sparse_optical_flow) {
        sparseOpticalFlow(last_frame, frame, fg_mask, new_opt,
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
    // if (num_splits != 1) pthread_mutex_lock(density_lock);

    args->density_store->at(i).first += density_point.first;
    args->density_store->at(i).second += density_point.second;

    // printf("%ld, density_point: (%f, %f), density_store[%d]: (%f, %f)\n",
    // pid, density_point.first, density_point.second, i,
    // args->density_store->at(i).first, args->density_store->at(i).second);

    // if (num_splits != 1) pthread_mutex_unlock(density_lock);

    // if (arg_parser.get_bool_argument_value("debug")) {
    //   imshow("Dynamic Density", dynamic_img);
    //   imshow("Noise Reduction", fg_mask);
    //   if (waitKey(1) == 'n') {
    //     break;
    //   }
    // }
    // printf("[#] Thread %ld: finished execution at time : %f s\n",
    // pid,get_time());
  }

  sem_wait(sem_exit);
  // cout << "Worker exiting: " << pid << endl;
  pthread_exit(NULL);
}

void run(runtime_params& params, density_t& density) {
  vector<Rect2d> cropping_rects;
  // vector<cv::Ptr<cv::BackgroundSubtractorMOG2>> bg_subs;
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

  // For Multithreading, 2 approaches may be used:
  // Either run 4 threads doing the same thing but on diff data
  // Or storing pre-processed frames in some array and then supplying it to each
  // of 3 threads
  int frames_processed = 0, frame_div = -1;
  bool video_split = params.split_video != 1;
  int num_threads = video_split ? params.split_video : params.split_frame;
  sem_t producer_ready, consumer_ready, sem_exit;
  vector<density_t> density_store;

  density.resize(frame_count);

  worker_params w_params{&params,         &frames_processed, &frame_div,
                         &consumer_ready, &producer_ready,   &sem_exit,
                         &frame};
  // w_params.params = &params;
  // w_params.density_store = &density;

  // auto density_lock = pthread_mutex_t();

  // w_params.density_lock = &density_lock;
  // w_params.consumer_ready = &consumer_ready;
  // w_params.producer_ready = &producer_ready;
  // w_params.sem_exit = &sem_exit;

  // w_params.frames_processed = &frames_processed;

  for (int i = 0; i < params.split_frame; i++) {
    auto new_w_param(w_params);
    density_t density_div(frame_count);
    density_store.push_back(density_div);

    // new_w_param.bg_sub = bg_subs[i];
    new_w_param.density_store = &density_store.back();
    thread_params.push_back(new_w_param);
  }

  producer_params prod_params{
      &cap,           &frame,      &consumer_ready,   &producer_ready,
      &sem_exit,      num_threads, &frames_processed, &frame_div,
      cropping_rects, &params};

  // prod_params.wait_for_threads = (video_split) ? 1 : num_threads;
  // prod_params.num_threads = num_threads;
  sem_init(&producer_ready, 0, 0);
  sem_init(&sem_exit, 0, 0);
  sem_init(&consumer_ready, 0, 1);

  // if (pthread_mutex_init(&density_lock, NULL) != 0) {
  //   printf("[-] Mutex init has failed\n");
  //   return;
  // }

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
    cout << density.size() << endl;
  }

  sem_destroy(&producer_ready);
  sem_destroy(&consumer_ready);
  sem_destroy(&sem_exit);
  // pthread_mutex_destroy(&density_lock);

  bar.finish();
  cap.release();
  cv::destroyAllWindows();
  start_points.clear();
  bg_subs.clear();
}