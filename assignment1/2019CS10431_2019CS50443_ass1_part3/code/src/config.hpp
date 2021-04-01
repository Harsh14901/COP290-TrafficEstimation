#ifndef CONFIG_H
#define CONFIG_H
#include <bits/stdc++.h>
#include <semaphore.h>

#include <fstream>
#include <iostream>
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
extern int frame_rate, frame_count;

typedef vector<pair<double, double>> density_t;
typedef vector<pair<string, string>> result_t;

extern vector<Point> start_points;

struct runtime_params {
  int skip_frames = 0;
  bool sparse_optical_flow = false;
  bool calc_dynamic_density = false;
  Size resolution = base_resolution;
  int split_frame = 1;  // Indicates number of splits to be made for each frame
  int split_video = 1;  // Indicates number of threads to be used for
                        // consecutively processing frames

  void set_values(int skip_frames, bool sparse_optical_flow,
                  bool calc_dynamic_density, int res_scale_factor,
                  int split_frame, int split_video);

  void read_config_file(string filename);
  void print_params();
};

struct worker_params {
  runtime_params* params;
  int* frames_processed;
  int* frame_div;
  sem_t* consumer_ready;
  sem_t* producer_ready;
  sem_t* sem_exit;
  Mat* frame_ptr;
  density_t* density_store;
};

struct producer_params {
  VideoCapture* cap_ptr;
  Mat* frame_ptr;
  sem_t* consumer_ready;
  sem_t* producer_ready;
  sem_t* sem_exit;
  int num_threads;
  int* frames_processed;
  int* frame_div;
  vector<Rect2d> cropping_rects;
  runtime_params* params;
};
#endif
