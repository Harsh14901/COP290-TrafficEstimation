
#include <img_transform.hpp>

using namespace std;
using namespace cv;

vector<Point2f> get_end_points(Mat src) {
  vector<Point2f> end_points;
  end_points.push_back(Point2f(472, 52));
  end_points.push_back(Point2f(472, 830));
  end_points.push_back(Point2f(800, 830));
  end_points.push_back(Point2f(800, 52));

  return end_points;
}

Mat transform_image(Mat src, vector<Point2f> start_points) {
  return transform_image(src, start_points, get_end_points(src));
}

Mat transform_image(Mat src, vector<Point2f> start_points,
                    vector<Point2f> end_points) {
  Mat homo = findHomography(start_points, end_points);

  Mat transformed_img;
  warpPerspective(src, transformed_img, homo, src.size());

  return transformed_img;
}

Mat crop_end_pts(Mat src) {
  vector<Point2f> end_pts = get_end_points(src);
  Rect crop_rect;

  crop_rect.x = end_pts[0].x;
  crop_rect.y = end_pts[0].y;

  crop_rect.width = end_pts[3].x - end_pts[0].x;
  crop_rect.height = end_pts[1].y - end_pts[0].y;

  return src(crop_rect);
}
