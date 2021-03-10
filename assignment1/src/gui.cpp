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
  while (true) {
    if (waitKey(0) == 'n') {
      break;
    }
  }
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
  cout
      << "[#] Use left mouse to select a point and right mouse to delete the "
         "last point!!\n"
      << "[#] Also don't worry too much about the order of points, we have got "
         "your back!!\n";
  Window::show();
}

void SelectionWindow::mouse_callback(int event, int x, int y, int flags,
                                     void* userData) {
  auto window = (SelectionWindow*)userData;
  if (event == EVENT_LBUTTONDOWN) {
    if (window->selected_points.size() == 4) {
      return;
    }
    if (arg_parser.get_bool_argument_value("debug")) {
      printf("[+] Point selected: %d , %d \n", x, y);
    }
    auto pt = Point(x, y);

    window->selected_points.push_back(pt);
    window->add_point(pt);
    window->display_image();
  } else if (event == EVENT_RBUTTONDOWN) {
    if (arg_parser.get_bool_argument_value("debug")) {
      cout << "[-] Removing last point" << endl;
    }
    if (!window->selected_points.empty()) {
      window->selected_points.pop_back();
      window->render_display();
      window->display_image();
    }
  }
}

void SelectionWindow::make_ccw_points() {
  assert(this->selected_points.size() == 4);
  auto debug = [&]() {
    cout << "[#] Counter clockwise points" << endl;
    for (auto& pt : this->selected_points) {
      cout << "[#] x: " << pt.x << ", y: " << pt.y << endl;
    }
  };
  auto x_sort = [](Point& a, Point& b) { return a.x < b.x; };
  auto y_sort = [](Point& a, Point& b) { return a.y < b.y; };
  auto y_rev_sort = [](Point& a, Point& b) { return a.y > b.y; };
  sort(this->selected_points.begin(), this->selected_points.end(), x_sort);
  sort(this->selected_points.begin(), this->selected_points.begin() + 2,
       y_sort);
  sort(this->selected_points.begin() + 2, this->selected_points.end(),
       y_rev_sort);

  if (arg_parser.get_bool_argument_value("debug")) {
    debug();
  }
}

void SelectionWindow::add_point(Point& pt) {
  circle(this->display, pt, 5, this->colors[0], -1);

  if (this->selected_points.size() > 1) {
    const int n = this->selected_points.size();
    line(this->display, this->selected_points[n - 1],
         this->selected_points[n - 2], this->colors[1]);
  }

  if (this->selected_points.size() == 4) {
    this->display = this->src.clone();
    this->make_ccw_points();

    for (unsigned int i = 0; i < this->selected_points.size(); i++) {
      circle(this->display, this->selected_points[i], 5, this->colors[0], -1);
      if (i != this->selected_points.size() - 1) {
        line(this->display, this->selected_points[i],
             this->selected_points[i + 1], this->colors[1]);
      } else {
        line(this->display, this->selected_points[3], this->selected_points[0],
             this->colors[1]);
      }
    }
  }
}

void SelectionWindow::display_image() {
  if (this->selected_points.size() == 4) {
    this->display_polygon();
  } else {
    imshow(this->window_name, this->display);
  }
}
void SelectionWindow::render_display() {
  this->display = this->src.clone();
  vector<Point> temp(this->selected_points);
  this->selected_points.clear();

  for (auto& pt : temp) {
    this->selected_points.push_back(pt);
    this->add_point(pt);
  }
}

void SelectionWindow::display_polygon() {
  auto overlay = this->display.clone();
  Mat final_image;
  fillConvexPoly(overlay, this->selected_points, this->colors[2]);
  addWeighted(overlay, this->alpha, this->display, 1 - this->alpha, 0,
              final_image);
  imshow(this->window_name, final_image);
}

AnimatedWindow::AnimatedWindow(string window_name, Mat& src,
                               vector<Point>& selected_points, int max_steps,
                               int delay)
    : Window(window_name, src),
      selected_points(selected_points),
      max_steps(max_steps),
      interval(delay) {
  ;
}

void AnimatedWindow::show() {
  for (int i = 0; i < this->max_steps; i++) {
    transform_image(this->src, this->intermediate_img,
                    (float(i) + 1.0) / float(this->max_steps));
    remove_black_borders(this->intermediate_img, this->display);
    imshow(this->window_name, this->display);
    waitKey(this->interval);
  }
  Window::show();
}

void AnimatedWindow::get_display(Mat& dst) { dst = this->display.clone(); }

void select_start_points(const Mat& input) {
  if (!arg_parser.get_bool_argument_value("skip-initial")) {
    Mat input_file_bnw, input_display;

    cvtColor(input, input_file_bnw, COLOR_BGR2GRAY);
    cvtColor(input, input_display, COLOR_BGR2BGRA);
    auto selection_window = SelectionWindow(
        original_name, input_display, {dot_color, line_color, fill_color});
    selection_window.show();

    if (selection_window.selected_points.size() != 4) {
      cerr << "[-] User did not select 4 points, Exiting." << endl;
      throw "Invalid point selection";
    }

    auto steps = arg_parser.get_bool_argument_value("no-animation") ? 1 : 120;
    auto animated_window =
        AnimatedWindow(transformed_name, input_file_bnw,
                       selection_window.selected_points, steps);
    start_points.assign(selection_window.selected_points.begin(),
                        selection_window.selected_points.end());
    animated_window.show();
  } else {
    get_start_points(start_points);
  }
}
