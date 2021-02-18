// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <img_transform.hpp>
#include <util.hpp>

using namespace cv;
using namespace std;

vector<Point> start_points;
Mat input_file, input_file_bnw, input_display;
const auto dot_color = Scalar(255, 0, 0), line_color = Scalar(255, 0, 0),
           fill_color = Scalar(100, 100, 0);

const auto fill_alpha = 0.4;
const auto original_window = "original", transformed_window = "transformed",
           cropped_window = "cropped";

void initialize_images();
void mouse_callback(int event, int x, int y, int, void*);

int main(int argc, char* argv[]) {
  initialize_images();

  display_window(original_window, input_display, mouse_callback);
  if (start_points.size() != 4) {
    cout << "[-] User did not select 4 points, Exiting." << endl;
    return -1;
  }

  Mat transformed_image, cropped_img;
  transform_image(input_file_bnw, transformed_image, start_points);
  display_window(transformed_window, transformed_image);

  crop_end_pts(transformed_image, cropped_img);
  display_window(cropped_window, cropped_img);

  return 0;
}

void initialize_images() {
  input_file = imread("input_files/empty.jpg", IMREAD_COLOR);
  cvtColor(input_file, input_file_bnw, COLOR_BGR2GRAY);
  cvtColor(input_file, input_display, COLOR_BGR2BGRA);
}

void mouse_callback(int event, int x, int y, int, void*) {
  if (event == EVENT_LBUTTONDOWN) {
    printf("[+] Point selected: %d , %d \n", x, y);
    auto pt = Point(x, y);
    start_points.push_back(pt);

    circle(input_display, pt, 5, dot_color, -1);

    if (start_points.size() > 1) {
      const int n = start_points.size();
      line(input_display, start_points[n - 1], start_points[n - 2], line_color);
    }

    if (start_points.size() == 4) {
      line(input_display, start_points[3], start_points[0], line_color);
      display_polygon(input_display, start_points, fill_color, fill_alpha,
                      original_window);

    } else {
      imshow(original_window, input_display);
    }
  }
}
