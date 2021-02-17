// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"


#include <util.hpp>
#include <img_transform.hpp>

using namespace cv;
using namespace std;

void mouse_callback(int event, int x, int y, int, void*);

vector<Point2f> start_points;

int main(int argc, char* argv[]) {
  Mat input_file = imread("input_files/empty.jpg");
  Mat input_file_bnw;

  cvtColor(input_file, input_file_bnw, COLOR_BGR2GRAY);

  display_window("original", input_file_bnw, mouse_callback);

  Mat transformed_img = transform_image(input_file_bnw, start_points);
  display_window("transformed", transformed_img);

  Mat cropped = crop_end_pts(transformed_img);
  display_window("cropped", cropped);

  return 0;
}

void mouse_callback(int event, int x, int y, int, void*) {
  if (event == EVENT_FLAG_LBUTTON) {
    printf("[+] Point detected at: %d , %d \n", x, y);
    start_points.push_back(Point2f(x, y));
  }
}

