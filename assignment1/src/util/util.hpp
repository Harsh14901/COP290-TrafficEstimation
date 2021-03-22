#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>
#include <sys/stat.h>

#include <config.hpp>
#include <opencv2/opencv.hpp>
#include <util/arg_parser.hpp>

using namespace std;
using namespace cv;

string get_image_name(string window_name, string output_dir);
void validate_directory(string output_dir);
void outputCSV(density_t &density, double frame_rate);
void outputCSV(string file_name, result_t &result, string header = "");
void save_images(Mat &transformed_img, Mat &cropped_img);
#endif
