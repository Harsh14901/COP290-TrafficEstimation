#ifndef GUI_H
#define GUI_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Window{
  private:
    Mat display;
  public:
    string window_name;
    Mat& src;
    
    Window(string window_name, Mat& src);
    void show();
    void set_mouse_callback(const MouseCallback& callback);
};


void display_polygon(const Mat& img, const InputArray& points,
                     const Scalar& color, double alpha = 1.0,
                     string window_name = "");
#endif
