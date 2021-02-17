#ifndef IMG_TRANSFORM_H
#define IMG_TRANSFORM_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

vector<Point2f> get_end_points(Mat src);
Mat transform_image(Mat src, vector<Point2f> start_points);
Mat transform_image(Mat src, vector<Point2f> start_points,
                   vector<Point2f> end_points);
Mat crop_end_pts(Mat src);

#endif
