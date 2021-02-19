#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>
#include <sys/stat.h>


using namespace std;
using namespace cv;

string get_image_name(string window_name, string output_dir);
void validate_directory(string output_dir);
#endif
