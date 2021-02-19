#include <gui.hpp>

using namespace std;
using namespace cv;

Window::Window(string window_name, Mat& src) : window_name(window_name), src(src){
  namedWindow(window_name);
  this->display = src.clone();
}

void Window::set_mouse_callback(const MouseCallback& callback){
  setMouseCallback(this->window_name, callback);
}

void Window::show(){
  imshow(this->window_name, this->display);
	waitKey(0);
	destroyWindow(this->window_name);
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