#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>
#include <sys/stat.h>

#include <util/arg_parser.hpp>
#include <config.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

string get_image_name(string window_name, string output_dir);
void validate_directory(string output_dir);
void outputCSV(vector<pair<double, double>> &density, double frame_rate);
void save_images(Mat &transformed_img, Mat &cropped_img);
#endif
