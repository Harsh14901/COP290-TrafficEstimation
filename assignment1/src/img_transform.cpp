
#include <img_transform.hpp>

using namespace std;
using namespace cv;

const vector<Point> get_end_points(const Mat &src)
{
	const vector<Point> end_points{
		Point(472, 52),
		Point(472, 830),
		Point(800, 830),
		Point(800, 52),
	};

	return end_points;
}

void transform_image(const Mat &src, Mat &dst,
					 const vector<Point> &start_points)
{
	transform_image(src, dst, start_points, get_end_points(src));
}

void transform_image(const Mat &src, Mat &dst,
					 const vector<Point> &start_points,
					 const vector<Point> &end_points)
{
	auto homo = findHomography(start_points, end_points);
	warpPerspective(src, dst, homo, src.size());
}

void crop_end_pts(const Mat &src, Mat &dst)
{
	auto end_pts = get_end_points(src);

	int x_min = min(end_pts[0].x, end_pts[1].x);
	int y_min = min(end_pts[0].y, end_pts[1].y);
	int x_max = max(end_pts[2].x, end_pts[3].x);
	int y_max = max(end_pts[2].y, end_pts[3].y);

	auto crop_rect = Rect{x_min, y_min, x_max - x_min, y_max - y_min};

	dst = src(crop_rect);
}
