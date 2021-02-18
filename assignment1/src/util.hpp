#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void display_window(const string& name, const Mat& img,
                    MouseCallback callback = NULL);
void display_polygon(const Mat& img, const InputArray& points,
                     const Scalar& color, double alpha = 1.0,
                     string window_name = "");

#endif
