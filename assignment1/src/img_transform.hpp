#ifndef IMG_TRANSFORM_H
#define IMG_TRANSFORM_H

#include <bits/stdc++.h>

#include <config.hpp>
#include <opencv2/opencv.hpp>
#include <util/arg_parser.hpp>
using namespace cv;
using namespace std;

const vector<Point> get_end_points(const Mat& src, float ratio = 1.0,
                                   const Size& resolution = base_resolution);
void transform_image(const Mat& src, Mat& dst, float ratio = 1.0,
                     const Size& resolution = base_resolution);
void transform_image(const Mat& src, Mat& dst, const vector<Point>& end_points);
void crop_end_pts(const Mat& src, Mat& dst, const Size& resolution);
void scale_pts(vector<Point>& end_points, const Size& resolution);
void get_start_points(vector<Point>& points);
#endif
