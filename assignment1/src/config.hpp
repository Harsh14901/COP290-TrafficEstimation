#ifndef CONFIG_H
#define CONFIG_H
#include <bits/stdc++.h>
#include <semaphore.h>

#include <opencv2/opencv.hpp>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const auto training_percent = 1.0;
const auto learning_rate = 0;
const auto kernel_shape = Size(3, 3);
const auto large_kernel_shape = Size(5, 5);
const auto dot_color = Scalar(255, 0, 0), line_color = Scalar(255, 0, 0),
           fill_color = Scalar(100, 100, 0);
const auto original_name = "original", transformed_name = "transformed",
           cropped_name = "cropped";
const auto base_resolution = Size(1920, 1080);
typedef vector<pair<double, double>> density_t;
typedef density_t result_t;

extern vector<Point> start_points;

struct runtime_params {
  int skip_frames = 1;
  bool sparse_optical_flow = false;
  bool calc_dynamic_density = false;
  Size resolution = base_resolution;
  int split_frame = 2;
  int split_video = 1;  // Indicates number of threads to be used
};

struct worker_params {
  runtime_params* params;
  density_t* density_store;
  int* frames_processed;
  sem_t* consumer_ready;
  sem_t* producer_ready;
  sem_t* sem_exit;
  struct frame_getter {
    Rect2d* cropping_rect;
    cv::Ptr<cv::BackgroundSubtractorMOG2> bg_sub;
    Mat* frame_ptr;
  };
  frame_getter frame_get;
  pthread_mutex_t* density_lock;
};

struct producer_params {
  VideoCapture* cap_ptr;
  Mat* frame_ptr;
  sem_t* consumer_ready;
  sem_t* producer_ready;
  sem_t* sem_exit;
  int wait_for_threads;
  int num_threads;
};
#endif
