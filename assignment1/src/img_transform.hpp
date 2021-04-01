#ifndef IMG_TRANSFORM_H
#define IMG_TRANSFORM_H

#include <config.hpp>
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
void make_scaled_rects(int splits, vector<Rect2d>& rects);
void crop_frame(Mat& frame, Mat& dst, Rect2d& scaled_rect);
#endif
