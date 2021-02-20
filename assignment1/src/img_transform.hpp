#ifndef IMG_TRANSFORM_H
#define IMG_TRANSFORM_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

const vector<Point> get_end_points(const Mat &src,const vector<Point> &start_points,bool auto_points);
void transform_image(const Mat& src, Mat& dst,
                     const vector<Point>& start_points,bool auto_points);
void transform_image(const Mat& src, Mat& dst,
                     const vector<Point>& start_points,
                     const vector<Point>& end_points);
void crop_end_pts(const Mat& src, Mat& dst,vector<Point> &start_points,bool auto_points);
vector<Point> get_points_in_order(vector<Point> src_pts);


#endif
