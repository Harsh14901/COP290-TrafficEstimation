
#include <img_transform.hpp>
#include <arg_parser.hpp>

using namespace std;
using namespace cv;


void swap_points(Point &a,Point &b){
	Point c = b;
	b = a;
	a = c;
}

// Assumes all 4 points are present
vector<Point> get_points_in_order(vector<Point> src_pts){


	vector<Point> x_sorted;
	for(int i=0;i<4;i++){
		x_sorted.push_back(src_pts[i]);
	}
	for(int i=0;i<3;i++){
		for(int j=i;j<3;j++){
			if(x_sorted[j].x>x_sorted[j+1].x){
				swap_points(x_sorted[j+1],x_sorted[j]);
				// auto temp = Point(x_sorted[j+1].x,x_sorted[j+1].y);
				// x_sorted[j+1] = Point(x_sorted[j].x,x_sorted[j].y);
				// x_sorted[j] = temp;
			}
		}
	}

	if(x_sorted[0].y>x_sorted[1].y){
				swap_points(x_sorted[0],x_sorted[1]);
	}
	if(x_sorted[3].y>x_sorted[2].y){
		swap_points(x_sorted[2],x_sorted[3]);
	}

	// double min = src_pts[0].x+src_pts[0].y;
	// int min_idx = 0;

	// for(int i=1;i<4;i++){
	// 	double val = src_pts[i].x+src_pts[i].y;
	// 	if(val<min){
	// 		min_idx = i;
	// 		min = val;
	// 	}
	// }

	// vector<Point> final_points;
	// for(int i=0;i<4;i++){
	// 	final_points.push_back(Point(src_pts[(i+min_idx) % 4].x,src_pts[(i+min_idx) % 4].y));
	// }
	return x_sorted;
}

const Point get_centeroid(const vector<Point> &points)
{
	int x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4;
	int y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4;
	return Point(x, y);
}

const vector<Point> get_end_points(const Mat &src, const vector<Point> &start_points)
{
	bool auto_points = arg_parser.get_bool_argument_value("auto_points");
	if (auto_points)
	{
		const Point centroid = get_centeroid(start_points);

		double width_top = sqrt(pow(start_points[0].x - start_points[3].x, 2) + pow(start_points[0].y - start_points[3].y, 2));
		double width_bot = sqrt(pow(start_points[1].x - start_points[2].x, 2) + pow(start_points[1].y - start_points[2].y, 2));

		double height_left = sqrt(pow(start_points[0].x - start_points[1].x, 2) + pow(start_points[0].y - start_points[1].y, 2));
		double height_right = sqrt(pow(start_points[3].x - start_points[2].x, 2) + pow(start_points[3].y - start_points[2].y, 2));

		double min_width = min(width_top, width_bot);
		double max_height = max(height_left, height_right);
		
		double x = centroid.x;
		double y = centroid.y;
		double rat = 1.2;
		max_height/=rat;
		min_width/=rat;
		
		const vector<Point> end_points{
			Point(int(x - min_width / 2), int(y - max_height / 2)),
			Point(int(x - min_width / 2), int(y + max_height / 2)),
			Point(int(x + min_width / 2), int(y + max_height / 2)),
			Point(int(x + min_width / 2), int(y - max_height / 2)),
		};
		return end_points;
	}

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
	transform_image(src, dst, start_points, get_end_points(src, start_points));
}

void transform_image(const Mat &src, Mat &dst,
					 const vector<Point> &start_points,
					 const vector<Point> &end_points)
{

	cout << end_points[0].x << " " << end_points[0].y << endl;
	cout << end_points[1].x << " " << end_points[1].y << endl;
	cout << end_points[2].x << " " << end_points[2].y << endl;
	cout << end_points[3].x << " " << end_points[3].y << endl;
	auto homo = findHomography(start_points, end_points);

	warpPerspective(src, dst, homo, src.size());

}

void crop_end_pts(const Mat &src, Mat &dst, vector<Point> &start_points)
{
	auto end_pts = get_end_points(src, start_points);

	int x_min = min(end_pts[0].x, end_pts[1].x);
	int y_min = min(end_pts[0].y, end_pts[3].y);
	int x_max = max(end_pts[2].x, end_pts[3].x);
	int y_max = max(end_pts[2].y, end_pts[1].y);

	auto crop_rect = Rect{x_min, y_min, x_max - x_min, y_max - y_min};

	dst = src(crop_rect);
}
