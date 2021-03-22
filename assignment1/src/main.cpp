// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <config.hpp>
#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <opencv2/opencv.hpp>
#include <performance.hpp>
#include <util/util.hpp>

#include "util/arg_parser.hpp"
#include "util/tqdm.h"
using namespace cv;
using namespace std;

density_t density;
tqdm bar;
int frame_rate, frame_count;
cv::Ptr<cv::BackgroundSubtractorMOG2> bg_sub;
Mat kernel, large_kernel, frame, first_frame, fg_mask, last_frame, prev_opt,
    bg_img;

vector<Point> start_points = vector<Point>();

void show_usage(string name);
bool handle_arguments(int argc, char* argv[]);
void initialize_elements(VideoCapture& cap);

void run(runtime_params& params, density_t& density);

int main(int argc, char* argv[]) {
  if (!handle_arguments(argc, argv)) {
    show_usage(argv[0]);
    return -1;
  }

  auto method = stoi(arg_parser.get_argument_value("method"));
  if (method != 0) {
    perform_analysis(run, method);
  } else {
    auto params = runtime_params{};
    auto density = density_t();
    run(params, density);

    outputCSV(density, frame_rate);
  }

  return 0;
}

bool handle_arguments(int argc, char* argv[]) {
  arg_parser.set_argument("input", "i", "./input_files/trafficvideo.mp4");
  arg_parser.set_argument("output", "o", "./output_files/density.csv");
  arg_parser.set_argument("method", "m", "0");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");
  arg_parser.set_standalone_argument("skip-initial", "s");
  arg_parser.set_standalone_argument("train", "t");

  return arg_parser.parse_arguments(argc, argv);
}
void show_usage(string name) {
  cerr << "Usage: " << name << "\n"
       << "Options:\n"
       << "\t-h, --help\t\tShow this help message\n"
       << "\t-i, --input\t\tSpecify the input file path. Default is "
          "./input_files/trafficvideo.mp4\n"
       << "\t-o, --output\t\tSpecify the output csv file. Default is "
          "./output_files/density.csv\n"
       << "\t-a, --autoselect-points \tSelect second set of points "
          "automatically\n"
       << "\t-d, --debug \t\tDisplay debug output\n"
       << "\t-f, --no-animation \t\tDo not display animation\n"
       << "\t-s, --skip-initial \t\tSkip initial selection of points\n"
       << "\t-t, --train-bg \t\tAuto train the background of video\n"
       << "\t-m, --method \t\tRuntime analysis method {1,2,3,4,5}\n"

       << endl;
}

void initialize_elements(VideoCapture& cap) {
  string file_name = arg_parser.get_argument_value("input");
  cout << "[+] Loading File: " << file_name << endl;
  cap.open(file_name);

  if (!cap.isOpened()) {
    cerr << "[-] Unable to open the video" << endl;
    throw "Could not load video";
  }

  cap.read(first_frame);
  select_start_points(first_frame);

  frame_rate = cap.get(CAP_PROP_FPS);
  frame_count = cap.get(CAP_PROP_FRAME_COUNT);

  if (arg_parser.get_bool_argument_value("debug")) {
    cout << "[+] Frame rate is: " << frame_rate << endl;
    cout << "[+] Frame count is: " << frame_count << endl;
  }

  bg_img = imread("./input_files/empty.jpg", IMREAD_UNCHANGED);

  if (bg_img.empty() && !arg_parser.get_bool_argument_value("train")) {
    cerr << "[-] Unable to load the background file ./input_files/empty.jpg"
         << endl;
    throw "Background Image not found";
  }

  bg_sub = createBackgroundSubtractorMOG2(
      int(frame_count * training_percent / 100.0), 8.0);

  kernel = getStructuringElement(MORPH_ELLIPSE, kernel_shape);
  large_kernel = getStructuringElement(MORPH_ELLIPSE, large_kernel_shape);

  bar.set_theme_line();
}

void run(runtime_params& params, density_t& density) {
  VideoCapture cap;

  initialize_elements(cap);

  cout << "[+] Training BG subtractor ..." << endl;
  if (arg_parser.get_bool_argument_value("train")) {
    train_bgsub(bg_sub, cap, fg_mask);
  } else {
    train_static_bgsub(bg_sub, bg_img, fg_mask);
  }

  cap.set(CAP_PROP_POS_FRAMES, 0);

  for (int i = 0; i < frame_count; i++) {
    bar.progress(i, frame_count);
    cap.read(frame);

    if (frame.empty()) {
      cerr << "[-] Video stream terminated unexpectedly" << endl;
      break;
    }

    if (i % (1 + params.skip_frames) != 0) {
      density.push_back(density.back());
      continue;
    }

    preprocess_frame(frame);

    if (i == 0) {
      last_frame = frame;
    }

    bg_sub->apply(frame, fg_mask, learning_rate);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }

    reduce_noise(fg_mask, kernel);

    Mat new_opt, dynamic_img;
    opticalFlow(last_frame, frame, new_opt);

    last_frame = frame;
    if (i == 0) {
      prev_opt = new_opt;
    }

    bitwise_and(new_opt, prev_opt, dynamic_img);
    prev_opt = new_opt;

    auto density_point = compute_density(fg_mask, dynamic_img);

    // Push density data to array
    density.push_back(density_point);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Dynamic Density", dynamic_img);
      imshow("Noise Reduction", fg_mask);
      if (waitKey(1) == 'n') {
        break;
      }
    }
  }

  bar.finish();
  cap.release();
  cv::destroyAllWindows();
  start_points.clear();
}