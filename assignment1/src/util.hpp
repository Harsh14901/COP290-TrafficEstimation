#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void display_window(string name, const Mat& img,
                    MouseCallback callback = NULL) {
  namedWindow(name, WINDOW_NORMAL);
  if (callback != NULL) {
    setMouseCallback(name, callback);
  }
  imshow(name, img);
  waitKey(0);
  destroyWindow(name);
}

#endif
