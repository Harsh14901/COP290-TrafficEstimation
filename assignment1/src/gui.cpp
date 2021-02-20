#include <arg_parser.hpp>
#include <gui.hpp>

using namespace std;
using namespace cv;

Window::Window(string window_name, Mat& src)
    : window_name(window_name), src(src) {
  namedWindow(window_name, WINDOW_GUI_NORMAL);
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

void SelectionWindow::show() {
  cout << "[#] Use left mouse to select a point and right mouse to delete the "
          "last point!!\n"
       << "[#] Also don't worry too much about the order of points, we have got "
          "your back!!\n";
  Window::show();
}

void SelectionWindow::mouse_callback(int event, int x, int y, int flags,
                                     void* userData) {
  auto window = (SelectionWindow*)userData;
  if (event == EVENT_LBUTTONDOWN) {
    if (window->start_points.size() == 4) {
      return;
    }
    if (arg_parser.get_bool_argument_value("debug")) {
      printf("[+] Point selected: %d , %d \n", x, y);
    }
    auto pt = Point(x, y);

    window->start_points.push_back(pt);
    window->add_point(pt);
    window->display_image();
  } else if (event == EVENT_RBUTTONDOWN) {
    if (arg_parser.get_bool_argument_value("debug")) {
      cout << "[-] Removing last point" << endl;
    }
    if (!window->start_points.empty()) {
      window->start_points.pop_back();
      window->render_display();
      window->display_image();
    }
  }
}

void SelectionWindow::make_ccw_points() {
  assert(this->start_points.size() == 4);
  auto debug = [&]() {
    if (arg_parser.get_bool_argument_value("debug")) {
      cout << "[#] Counter clockwise points" << endl;
      for (auto& pt : this->start_points) {
        cout << "[#] x: " << pt.x << ", y: " << pt.y << endl;
      }
    }
  };
  auto x_sort = [](Point& a, Point& b) { return a.x < b.x; };
  auto y_sort = [](Point& a, Point& b) { return a.y < b.y; };
  auto y_rev_sort = [](Point& a, Point& b) { return a.y > b.y; };
  sort(this->start_points.begin(), this->start_points.end(), x_sort);
  sort(this->start_points.begin(), this->start_points.begin() + 2, y_sort);
  sort(this->start_points.begin() + 2, this->start_points.end(), y_rev_sort);

  debug();
}

void SelectionWindow::add_point(Point& pt) {
  circle(this->display, pt, 5, this->colors[0], -1);

  if (this->start_points.size() > 1) {
    const int n = this->start_points.size();
    line(this->display, this->start_points[n - 1], this->start_points[n - 2],
         this->colors[1]);
  }

  if (this->start_points.size() == 4) {
    this->display = this->src.clone();
    this->make_ccw_points();

    for (unsigned int i = 0; i < this->start_points.size(); i++) {
      circle(this->display, this->start_points[i], 5, this->colors[0], -1);
      if (i != this->start_points.size() - 1) {
        line(this->display, this->start_points[i], this->start_points[i + 1],
             this->colors[1]);
      } else {
        line(this->display, this->start_points[3], this->start_points[0],
             this->colors[1]);
      }
    }
  }
}

void SelectionWindow::display_image() {
  if (this->start_points.size() == 4) {
    this->display_polygon();
  } else {
    imshow(this->window_name, this->display);
  }
}
void SelectionWindow::render_display() {
  this->display = this->src.clone();
  vector<Point> temp(this->start_points);
  this->start_points.clear();

  for (auto& pt : temp) {
    this->start_points.push_back(pt);
    this->add_point(pt);
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