#include <util.hpp>

using namespace std;
using namespace cv;

void display_window(const string &name, const Mat &img,
					MouseCallback callback)
{
	namedWindow(name, WINDOW_NORMAL);
	if (callback != NULL)
	{
		setMouseCallback(name, callback);
	}
	imshow(name, img);
	waitKey(0);
	destroyWindow(name);
}

void display_polygon(const Mat &img, const InputArray &points,
					 const Scalar &color, double alpha, string window_name)
{
	auto overlay = img.clone();
	Mat final_image;
	fillConvexPoly(overlay, points, color);
	addWeighted(overlay, alpha, img, 1 - alpha, 0, final_image);
	imshow(window_name, final_image);
}
