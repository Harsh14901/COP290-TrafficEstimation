
#include <img_transform.hpp>

using namespace std;
using namespace cv;

const Point get_centroid(const vector<Point> &points) {
  int x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4;
  int y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4;
  return Point(x, y);
}

Point get_division(float ratio, Point p1, Point p2) {
  return Point(int(p1.x * ratio + (1 - ratio) * p2.x),
               int(p1.y * ratio + (1 - ratio) * p2.y));
}
void scale_pts(vector<Point> &end_points, const Size &resolution) {
  for (auto &pt : end_points) {
    pt.x *= float(resolution.width) / float(base_resolution.width);
    pt.y *= float(resolution.height) / float(base_resolution.height);
  }
}

const vector<Point> get_end_points(const Mat &src, float ratio,
                                   const Size &resolution) {
  bool auto_points = arg_parser.get_bool_argument_value("autoselect-points") ||
                     arg_parser.get_bool_argument_value("skip-initial");
  if (auto_points) {
    const Point centroid = get_centroid(start_points);

    double width_top = sqrt(pow(start_points[0].x - start_points[3].x, 2) +
                            pow(start_points[0].y - start_points[3].y, 2));
    double width_bot = sqrt(pow(start_points[1].x - start_points[2].x, 2) +
                            pow(start_points[1].y - start_points[2].y, 2));

    double height_left = sqrt(pow(start_points[0].x - start_points[1].x, 2) +
                              pow(start_points[0].y - start_points[1].y, 2));
    double height_right = sqrt(pow(start_points[3].x - start_points[2].x, 2) +
                               pow(start_points[3].y - start_points[2].y, 2));

    double min_width = min(width_top, width_bot);
    double max_height = max(height_left, height_right);

    double x = centroid.x;
    double y = centroid.y;
    double rat = 1.2;
    max_height /= rat;
    min_width /= rat;

    vector<Point> end_points{
        get_division(ratio,
                     Point(int(x - min_width / 2), int(y - max_height / 2)),
                     start_points[0]),
        get_division(ratio,
                     Point(int(x - min_width / 2), int(y + max_height / 2)),
                     start_points[1]),
        get_division(ratio,
                     Point(int(x + min_width / 2), int(y + max_height / 2)),
                     start_points[2]),
        get_division(ratio,
                     Point(int(x + min_width / 2), int(y - max_height / 2)),
                     start_points[3]),
    };
    return end_points;
  }

  vector<Point> end_points{
      get_division(ratio, Point(472, 52), start_points[0]),
      get_division(ratio, Point(472, 830), start_points[1]),
      get_division(ratio, Point(800, 830), start_points[2]),
      get_division(ratio, Point(800, 52), start_points[3]),
  };
  scale_pts(end_points, resolution);

  return end_points;
}

void transform_image(const Mat &src, Mat &dst, float ratio,
                     const Size &resolution) {
  auto end_points = get_end_points(src, ratio, resolution);
  transform_image(src, dst, end_points);
}

void transform_image(const Mat &src, Mat &dst,
                     const vector<Point> &end_points) {
  auto homo = findHomography(start_points, end_points);

  warpPerspective(src, dst, homo, src.size());
}

void crop_end_pts(const Mat &src, Mat &dst, const Size &resolution) {
  auto end_pts = get_end_points(src, 1, resolution);
  int x_min = min(end_pts[0].x, end_pts[1].x);
  int y_min = min(end_pts[0].y, end_pts[3].y);
  int x_max = max(end_pts[2].x, end_pts[3].x);
  int y_max = max(end_pts[2].y, end_pts[1].y);

  auto crop_rect = Rect{x_min, y_min, x_max - x_min, y_max - y_min};

  dst = src(crop_rect);
}

void get_start_points(vector<Point> &points) {
  points.push_back(Point(972, 264));
  points.push_back(Point(540, 1068));
  points.push_back(Point(1530, 1074));
  points.push_back(Point(1308, 264));
}

void make_scaled_rects(int splits, vector<Rect2d> &rects) {
  switch (splits) {
    case 2:

      rects.push_back(Rect2d{0.0, 0.0, 1.0, 1.0 / 2.0});
      rects.push_back(Rect2d{0.0, 1.0 / 2.0, 1.0, 1.0 / 2.0});
      break;
    case 3:
      rects.push_back(Rect2d{0.0, 0.0, 1.0, 1.0 / 3.0});
      rects.push_back(Rect2d{0.0, 1.0 / 3.0, 1.0, 1.0 / 3.0});
      rects.push_back(Rect2d{0.0, 2.0 / 3.0, 1.0, 1.0 / 3.0});
      break;
    case 4:
      rects.push_back(Rect2d{0.0, 0.0, 1.0, 1.0 / 4.0});
      rects.push_back(Rect2d{0.0, 1.0 / 4.0, 1.0, 1.0 / 4.0});
      rects.push_back(Rect2d{0.0, 2.0 / 4.0, 1.0, 1.0 / 4.0});
      rects.push_back(Rect2d{0.0, 3.0 / 4.0, 1.0, 1.0 / 4.0});
      break;
    case 6:
      rects.push_back(Rect2d{0.0, 0.0, 1.0 / 2.0, 1.0 / 3.0});
      rects.push_back(Rect2d{1.0/2.0, 0.0, 1.0 / 2.0, 1.0 / 3.0});
      rects.push_back(Rect2d{0.0, 1.0 / 3.0, 1.0 / 2.0, 1.0 / 3.0});
      rects.push_back(Rect2d{1.0/2.0, 1.0 / 3.0, 1.0 / 2.0, 1.0 / 3.0});
      rects.push_back(Rect2d{0.0, 2.0 / 3.0, 1.0 / 2.0, 1.0 / 3.0});
      rects.push_back(Rect2d{1.0/2.0, 2.0 / 3.0, 1.0 / 2.0, 1.0 / 3.0});
      break;
    default:
      rects.push_back(Rect2d{0, 0, 1.0, 1.0});
      break;
  }
}

void crop_frame(Mat &frame, Mat &dst, Rect2d &scaled_rect) {
  double rows = (double)frame.rows;
  double cols = (double)frame.cols;

  auto crop_rect = Rect2d{scaled_rect.x * cols, scaled_rect.y * rows,
                        scaled_rect.width * cols, scaled_rect.height * rows};
  dst = frame(crop_rect);
}
