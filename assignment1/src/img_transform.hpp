#ifndef IMG_TRANSFORM_H
#define IMG_TRANSFORM_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

const vector<Point> get_end_points(const Mat& src,
                                   const vector<Point>& start_points,float ratio = 1.0);
void transform_image(const Mat& src, Mat& dst,
                     const vector<Point>& start_points,float ratio=1.0);
void transform_image(const Mat& src, Mat& dst,
                     const vector<Point>& start_points,
                     const vector<Point>& end_points);
void crop_end_pts(const Mat& src, Mat& dst, vector<Point>& start_points);

#endif
