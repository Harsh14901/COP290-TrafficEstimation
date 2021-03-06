// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <util.hpp>

#include "arg_parser.hpp"
using namespace cv;
using namespace std;

VideoCapture cap;
Mat input_file, input_file_bnw, input_display;
const auto training_percent = 1.0;
const auto learning_rate = 0;
const auto kernel_shape = Size(3, 3);

const auto dot_color = Scalar(255, 0, 0), line_color = Scalar(255, 0, 0),
           fill_color = Scalar(100, 100, 0);

const auto original_name = "original", transformed_name = "transformed",
           cropped_name = "cropped";

vector<pair<double, double>> density;

void initialize_images();
void show_usage(string name);
bool handle_arguments(int argc, char* argv[]);
bool initialize_video();
void outputCSV(double frame_rate);
void save_images(Mat& transformed_img, Mat& cropped_img);

int main(int argc, char* argv[]) {
  if (!handle_arguments(argc, argv)) {
    show_usage(argv[0]);
    return -1;
  }

  if (!initialize_video()) {
    cerr << "[-] Unable to open the video" << endl;
    return -1;
  }

  initialize_images();
  auto selection_window = SelectionWindow(original_name, input_display,
                                          {dot_color, line_color, fill_color});
  selection_window.show();

  if (selection_window.start_points.size() != 4) {
    cout << "[-] User did not select 4 points, Exiting." << endl;
    return -1;
  }

  Mat intermediate_img, transformed_img, cropped_img;
  auto animated_window = AnimatedWindow(
      transformed_name, input_file_bnw, selection_window.start_points,
      (arg_parser.get_bool_argument_value("no-animation")) ? 1 : 120);
  animated_window.show();
  animated_window.get_display(transformed_img);

  auto frame_rate = cap.get(CAP_PROP_FPS);
  auto frame_count = cap.get(CAP_PROP_FRAME_COUNT);
  auto frame_area = contourArea(
      get_end_points(transformed_img, selection_window.start_points));

  cout << "[+] Frame rate is: " << frame_rate << endl;
  cout << "[+] Frame count is: " << frame_count << endl;

  Mat frame, fg_mask, frame_gray;
  Mat processed_img;

  auto bg_sub = createBackgroundSubtractorMOG2(
      int(frame_count * training_percent / 100.0));
  auto kernel = getStructuringElement(MORPH_ELLIPSE, kernel_shape);

  cout << "[+] Training BG subtractor ..." << endl;
  for (int i = 0; i < bg_sub->getHistory(); i++) {
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished before training" << endl;
      return -1;
    }
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    Mat temp;

    transform_image(frame_gray, temp, selection_window.start_points);
    remove_black_borders(temp, frame_gray);
    crop_end_pts(frame_gray, temp, selection_window.start_points);
    frame_gray = temp;

    bg_sub->apply(frame_gray, fg_mask, learning_rate);
  }

  for (int i = 1; i <= frame_count; i++) {
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished" << endl;
      break;
    }
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

    Mat temp;

    transform_image(frame_gray, temp, selection_window.start_points);
    remove_black_borders(temp, frame_gray);
    crop_end_pts(frame_gray, temp, selection_window.start_points);
    frame_gray = temp;

    bg_sub->apply(frame_gray, fg_mask, learning_rate);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }
    // TODO: Change the number of iterations of the three ops below.
    morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 1);
    morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);
    morphologyEx(fg_mask, processed_img, MORPH_DILATE, kernel, Point(-1, -1),
                 1);

    threshold(processed_img, fg_mask, 240, 255, THRESH_BINARY);

    vector<vector<Point>> contours;
    vector<vector<Point>> rects;

    findContours(fg_mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    if (arg_parser.get_bool_argument_value("debug")) {
      for (uint i = 0; i < contours.size(); i++) {
        Rect bnd_rect = boundingRect(contours[i]);
        rects.push_back(vector<Point>());
        rects[i].push_back(Point(bnd_rect.x, bnd_rect.y));
        rects[i].push_back(Point(bnd_rect.x + bnd_rect.width, bnd_rect.y));
        rects[i].push_back(
            Point(bnd_rect.x + bnd_rect.width, bnd_rect.y + bnd_rect.height));
        rects[i].push_back(Point(bnd_rect.x, bnd_rect.y + bnd_rect.height));
        drawContours(fg_mask, rects, i, fill_color, 4);
      }
    }
    double area = 0;
    for (auto& contour : contours) {
      area += contourArea(contour);
    }
    density.push_back(make_pair(area / frame_area, 0));

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("MOG2", fg_mask);
      if (waitKey(1) == 'n') {
        break;
      }
    }
  }
  cap.release();
  destroyAllWindows();
  outputCSV(frame_rate);

  return 0;
}

bool handle_arguments(int argc, char* argv[]) {
  arg_parser.set_argument("input", "i", "./input_files/trafficvideo.mp4");
  arg_parser.set_argument("output", "o", "./output_files/density.csv");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");

  return arg_parser.parse_arguments(argc, argv);
}
void show_usage(string name) {
  cerr << "Usage: " << name << "\n"
       << "Options:\n"
       << "\t-h, --help\t\tShow this help message\n"
       << "\t-i, --input\t\tSpecify the input file path. Default is "
          "./input_files/empty.jpg\n"
       << "\t-o, --output\t\tSpecify the output csv file. Default is "
          "./output_files/density.csv\n"
       << "\t-a, --autoselect-points \tSelect second set of points "
          "automatically\n"
       << "\t-d, --debug \t\tDisplay debug output\n"
       << "\t-f, --no-animation \t\tDo not display animation\n"
       << std::endl;
}

bool initialize_video() {
  string file_name = arg_parser.get_argument_value("input");
  cout << "[+] Loading File: " << file_name << endl;
  cap.open(file_name);

  return cap.isOpened();
}

void outputCSV(double frame_rate) {
  cout << "[+] Writing output to CSV file" << endl;
  string file_name = arg_parser.get_argument_value("output");
  fstream out_file;
  out_file.open(file_name, ios::out);
  if (!out_file) {
    cerr << "[-] Error writing to the output file: " << file_name << endl;
  } else {
    out_file << "time,queue_density,dynamic_density" << endl;
    for (uint i = 0; i < density.size(); i++) {
      out_file << double(i + 1) / frame_rate << "," << density[i].first << ","
               << density[i].second << endl;
    }

    out_file.close();
    cout << "[+] Density data succesfully written to: " << file_name << endl;
  }
}
void initialize_images() {
  cap.read(input_file);
  cvtColor(input_file, input_file_bnw, COLOR_BGR2GRAY);
  cvtColor(input_file, input_display, COLOR_BGR2BGRA);
}

void save_images(Mat& transformed_img, Mat& cropped_img) {
  auto output_dir = arg_parser.get_argument_value("output");

  auto transformed_img_name = get_image_name(transformed_name, output_dir);
  auto cropped_img_name = get_image_name(cropped_name, output_dir);

  try {
    validate_directory(output_dir);

    imwrite(transformed_img_name, transformed_img);
    cout << "[+] Transformed image saved to: " << transformed_img_name << endl;

    imwrite(cropped_img_name, cropped_img);
    cout << "[+] Cropped image saved to: " << cropped_img_name << endl;
  } catch (const std::exception& e) {
    cerr << "[-] Unable to save images" << endl;
    std::cerr << e.what() << '\n';
  }
}
