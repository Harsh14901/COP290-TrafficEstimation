#include <gui.hpp>

using namespace std;
using namespace cv;

Window::Window(string window_name, Mat& src)
    : window_name(window_name), src(src) {
  namedWindow(window_name);
  this->display = src.clone();
}

void Window::set_mouse_callback(const MouseCallback& callback, void* userData) {
  setMouseCallback(this->window_name, callback, userData);
}

void Window::show() {
  imshow(this->window_name, this->display);
  waitKey(0);
  destroyWindow(this->window_name);
}

SelectionWindow::SelectionWindow(string window_name, Mat& src,
                                 vector<Scalar> colors)
    : Window(window_name, src) {
  if (colors.size() < 3) {
    this->colors =
        vector<Scalar>{Scalar(0, 0, 0), Scalar(0, 0, 0), Scalar(0, 0, 0)};
  } else {
    this->colors = colors;
  }
  this->set_mouse_callback(SelectionWindow::mouse_callback, this);
}

SelectionWindow::SelectionWindow(string window_name, Mat& src,
                                 vector<Scalar> colors, double alpha)
    : SelectionWindow(window_name, src, colors) {
  this->alpha = alpha;
}

void SelectionWindow::mouse_callback(int event, int x, int y, int flags,
                                     void* userData) {
  auto window = (SelectionWindow*)userData;
  if (event == EVENT_LBUTTONDOWN) {
    printf("[+] Point selected: %d , %d \n", x, y);
    auto pt = Point(x, y);
    window->start_points.push_back(pt);

    circle(window->display, pt, 5, window->colors[0], -1);

    if (window->start_points.size() > 1) {
      const int n = window->start_points.size();
      line(window->display, window->start_points[n - 1],
           window->start_points[n - 2], window->colors[1]);
    }

    if (window->start_points.size() == 4) {
      line(window->display, window->start_points[3], window->start_points[0],
           window->colors[1]);
      window->display_polygon();
    } else {
      imshow(window->window_name, window->display);
    }
  }
}

void SelectionWindow::display_polygon() {
  auto overlay = this->display.clone();
  Mat final_image;
  fillConvexPoly(overlay, this->start_points, this->colors[2]);
  addWeighted(overlay, this->alpha, this->display, 1 - this->alpha, 0,
              final_image);
  imshow(this->window_name, final_image);
}