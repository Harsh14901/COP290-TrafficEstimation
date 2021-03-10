
#include <arg_parser.hpp>
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

const vector<Point> get_end_points(const Mat &src,
                                   const vector<Point> &start_points,
                                   float ratio) {
  bool auto_points = arg_parser.get_bool_argument_value("autoselect-points") || arg_parser.get_bool_argument_value("skip_initial");
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

  const vector<Point> end_points{
      get_division(ratio, Point(472, 52), start_points[0]),
      get_division(ratio, Point(472, 830), start_points[1]),
      get_division(ratio, Point(800, 830), start_points[2]),
      get_division(ratio, Point(800, 52), start_points[3]),
  };
  return end_points;
}

void transform_image(const Mat &src, Mat &dst,
                     const vector<Point> &start_points, float ratio) {
  transform_image(src, dst, start_points,
                  get_end_points(src, start_points, ratio));
}

void transform_image(const Mat &src, Mat &dst,
                     const vector<Point> &start_points,
                     const vector<Point> &end_points) {
  // if (arg_parser.get_bool_argument_value("debug")) {
  //   cout << "[#] Taking end points as: " << endl;
  //   cout << end_points[0].x << " " << end_points[0].y << endl;
  //   cout << end_points[1].x << " " << end_points[1].y << endl;
  //   cout << end_points[2].x << " " << end_points[2].y << endl;
  //   cout << end_points[3].x << " " << end_points[3].y << endl;
  // }

  auto homo = findHomography(start_points, end_points);

  warpPerspective(src, dst, homo, src.size());
}

void crop_end_pts(const Mat &src, Mat &dst, vector<Point> &start_points) {
  auto end_pts = get_end_points(src, start_points);

  int x_min = min(end_pts[0].x, end_pts[1].x);
  int y_min = min(end_pts[0].y, end_pts[3].y);
  int x_max = max(end_pts[2].x, end_pts[3].x);
  int y_max = max(end_pts[2].y, end_pts[1].y);

  auto crop_rect = Rect{x_min, y_min, x_max - x_min, y_max - y_min};

  dst = src(crop_rect);
}
