#ifndef GUI_H
#define GUI_H

#include <config.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <util/arg_parser.hpp>

using namespace std;
using namespace cv;

class Window {
 protected:
  Mat display;

 public:
  string window_name;
  Mat& src;

  Window(string window_name, Mat& src);
  virtual void show();
  void set_mouse_callback(const MouseCallback& callback, void* userData = 0);
};

class SelectionWindow : public Window {
 private:
  static void mouse_callback(int event, int x, int y, int flags,
                             void* userData);
  void display_polygon();
  void add_point(Point& pt);
  void render_display();
  void display_image();
  void make_ccw_points();

 public:
  vector<Point> selected_points;
  /*
    colors[0] -> dot_color,
    colors[1] -> line_color,
    colors[2] -> fill_color
   */
  vector<Scalar> colors;
  double alpha = 0.4;

  SelectionWindow(string window_name, Mat& src, vector<Scalar> colors);
  SelectionWindow(string window_name, Mat& src, vector<Scalar> colors,
                  double aplha);

  void show() override;
};

class AnimatedWindow : public Window {
 private:
  Mat intermediate_img;

 public:
  vector<Point>& selected_points;
  int max_steps;
  int interval;

  AnimatedWindow(string window_name, Mat& src, vector<Point>& selected_points,
                 int max_steps = 120, int interval = 5);

  void show() override;
  void get_display(Mat& dst);
};

void select_start_points(const Mat& input,
                         const Size& resolution = base_resolution);

#endif
