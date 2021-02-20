#include <img_processor.hpp>

void remove_black_borders(const Mat &src, Mat &dst) {
  Mat bnw, gray;
  if (src.channels() == 3) {
    cvtColor(src, gray, COLOR_BGR2RGB);
  } else {
    gray = src;
  }
  threshold(gray, bnw, 0, 255, THRESH_BINARY);

  vector<vector<Point>> contours;
  findContours(bnw, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
  Rect crop_rect = boundingRect(contours[0]);
  dst = src(crop_rect);
}
