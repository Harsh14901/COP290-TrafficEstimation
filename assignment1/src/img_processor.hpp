#include <bits/stdc++.h>

#include <config.hpp>
#include <img_transform.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void remove_black_borders(const Mat& src, Mat& dst);
void train_static_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub,
                        const Mat& bg_img, Mat& fg_mask, const Size& resolution);
void train_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub,
                 VideoCapture& cap, Mat& fg_mask, const Size& resolution);
void opticalFlow(const Mat& prvs, const Mat& next, Mat& dst);
void preprocess_frame(Mat& frame, const Size& resolution);
void reduce_noise(Mat& fg_mask, const Mat& kernel);

pair<double, double> compute_density(const Mat& fg_mask, Mat& dynamic_img);
pair<double, double> compute_density(const Mat& fg_mask);