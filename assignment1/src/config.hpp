#ifndef CONFIG_H
#define CONFIG_H
#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

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
typedef vector<pair<double, double>> density_t;
typedef density_t result_t;

extern vector<Point> start_points;

struct runtime_params {
  int skip_frames = 0;
  bool sparse_optical_flow = 0;
  pair<int, int> resolution = make_pair(1920, 1080);
  int split_frame = 1;
  int split_video = 1;
};

#endif
