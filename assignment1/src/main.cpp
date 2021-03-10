// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <util.hpp>
#include <opencv2/opencv.hpp>

#include "tqdm.h"
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
void train_bg_subtractor(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub,Mat frame,Mat &frame_gray,
                        vector<Point> &start_points, Mat &fg_mask);
Mat opticalFlow(Mat &prvs,Mat &next);
void train_bg_subtractor2(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub,Mat &frame_gray,
                        vector<Point> &start_points, Mat &fg_mask);


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
  vector<Point> start_points;
  Mat intermediate_img, transformed_img, cropped_img;

  if(!arg_parser.get_bool_argument_value("skip_initial")){
    auto selection_window = SelectionWindow(original_name, input_display,
                                            {dot_color, line_color, fill_color});
    selection_window.show();

    if (selection_window.start_points.size() != 4) {
      cout << "[-] User did not select 4 points, Exiting." << endl;
      return -1;
    }



    auto animated_window = AnimatedWindow(
        transformed_name, input_file_bnw, selection_window.start_points,
        (arg_parser.get_bool_argument_value("no-animation")) ? 1 : 120);
    animated_window.show();
    animated_window.get_display(transformed_img);
    start_points = selection_window.start_points;
  }else{

    start_points.push_back(Point(972,264));
    start_points.push_back(Point(540,1068));
    start_points.push_back(Point(1530,1074));
    start_points.push_back(Point(1308,264));

    transform_image(input_file_bnw, intermediate_img, start_points,120);

    remove_black_borders(intermediate_img, transformed_img);
  }


  auto frame_rate = cap.get(CAP_PROP_FPS);
  auto frame_count = cap.get(CAP_PROP_FRAME_COUNT);

  cout << "[+] Frame rate is: " << frame_rate << endl;
  cout << "[+] Frame count is: " << frame_count << endl;

  Mat frame, fg_mask, frame_gray;
  Mat processed_img;

  auto bg_sub = createBackgroundSubtractorMOG2(
      int(frame_count * training_percent / 100.0),8.0);

  auto kernel = getStructuringElement(MORPH_ELLIPSE, kernel_shape);
  auto large_kernel = getStructuringElement(MORPH_ELLIPSE, Size(5,5));

  cout << "[+] Training BG subtractor ..." << endl;
  train_bg_subtractor(bg_sub,frame,frame_gray,start_points,fg_mask);
  // train_bg_subtractor2(bg_sub,frame_gray,start_points,fg_mask);


  int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
  VideoWriter outputVideo;                                        // Open the output

  Size S = Size(fg_mask.cols,    // Acquire input size
                  fg_mask.rows);
  outputVideo.open("TestVideo.mp4", ex, cap.get(CAP_PROP_FPS),S,true);
  vector<Mat> spl;


  Mat prev,prev_opt;
  // Main for loop
    // auto T = tq::trange(100, 5000);

  tqdm bar;
  bar.set_theme_line();
    // for (int i : T){
  for (int i=0; i<frame_count;i++) {
    bar.progress(i,frame_count);
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished" << endl;
      break;
    }

    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);



    Mat temp;

    transform_image(frame_gray, temp, start_points);
    remove_black_borders(temp, frame_gray);
    crop_end_pts(frame_gray, temp, start_points);
    frame_gray = temp;

    if(i==0){
      prev = frame_gray;
    }

    Mat new_opt = opticalFlow(prev,frame_gray);
    prev = frame_gray;
    Mat dynamic_img; 
    if(i==0){
      prev_opt = new_opt;
    }
    bitwise_and(new_opt,prev_opt,dynamic_img);
    prev_opt = new_opt;

    Mat bnw_dynamic;
    threshold(dynamic_img*255,bnw_dynamic,15,255,THRESH_BINARY);
    Mat processed_bnw_dynamic;
    // morphologyEx(bnw_dynamic, processed_bnw_dynamic, MORPH_ERODE, large_kernel, Point(-1, -1),
    //         1);
    // morphologyEx(bnw_dynamic,processed_bnw_dynamic, MORPH_DILATE, kernel, Point(-1, -1),
    //           2);

    bnw_dynamic.copyTo(processed_bnw_dynamic);




    bg_sub->apply(frame_gray, fg_mask, learning_rate);

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Preprocessed", fg_mask);
    }



    // TODO: Change the number of iterations of the three ops below.
    morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 4);
    morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);
    morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 3);
    morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);

    
    morphologyEx(fg_mask, processed_img, MORPH_DILATE, kernel, Point(-1, -1),
                 4);

    threshold(processed_img, fg_mask, 240, 255, THRESH_BINARY);

    Mat rect_img;
    fg_mask.copyTo(rect_img);

    
    vector<vector<Point>> contours;
    vector<vector<Point>> rects;




    // Calculate the suitable area
    Mat final_static,final_static2;
    processed_bnw_dynamic.convertTo(final_static2,fg_mask.depth());
    bitwise_or(fg_mask,final_static2,final_static);

    

    double area  = cv::sum(final_static).val[0]/(255.0 * final_static.cols * final_static.rows );
    double dynamic_area  = cv::sum(processed_bnw_dynamic).val[0]/(255.0 * processed_bnw_dynamic.cols * processed_bnw_dynamic.rows );
    
    


    // Push density data to array
    density.push_back(make_pair(area, dynamic_area));

    if (arg_parser.get_bool_argument_value("debug")) {
      imshow("Dynamic Density",processed_bnw_dynamic);
      imshow("MOG2", fg_mask);
      if (waitKey(1) == 'n') {
        break;
      }
    }
    outputVideo.write(fg_mask);


  }
  bar.finish();
  outputVideo.release();
  cap.release();
  cv::destroyAllWindows();
  outputCSV(frame_rate);

  return 0;
}




void train_bg_subtractor2(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub,Mat &frame_gray,
                        vector<Point> &start_points, Mat &fg_mask){
  Mat frame = imread("./input_files/empty.jpg");
  cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
  Mat temp;
  transform_image(frame_gray, temp, start_points);
  remove_black_borders(temp, frame_gray);
  crop_end_pts(frame_gray, temp, start_points);
  frame_gray = temp;

  for (int i = 0; i < bg_sub->getHistory(); i++) {
    bg_sub->apply(frame_gray, fg_mask, learning_rate);
  }
  
}

void train_bg_subtractor(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub,Mat frame,Mat &frame_gray,
                        vector<Point> &start_points, Mat &fg_mask){

  for (int i = 0; i < bg_sub->getHistory(); i++) {
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished before training" << endl;
      return ;
    }
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    Mat temp;

    transform_image(frame_gray, temp, start_points);
    remove_black_borders(temp, frame_gray);
    crop_end_pts(frame_gray, temp, start_points);
    frame_gray = temp;

    bg_sub->apply(frame_gray, fg_mask, learning_rate);
  }
  
}


bool handle_arguments(int argc, char* argv[]) {
  arg_parser.set_argument("input", "i", "./input_files/trafficvideo.mp4");
  arg_parser.set_argument("output", "o", "./output_files/density.csv");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");
  arg_parser.set_standalone_argument("skip_initial","s");

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
       << "\t-s, --skip_initial \t\tSkip initial input of points\n"

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


Mat opticalFlow(Mat &prvs,Mat &next){

    Mat flow(prvs.size(), CV_32FC2);
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);


    // visualization
    Mat flow_parts[2];

    split(flow, flow_parts);

    Mat magnitude, angle, magn_norm;

    cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
    normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
    angle *= ((1.f / 360.f) * (180.f / 255.f));


    //build hsv image
    Mat _hsv[3], hsv, hsv8, bgr;
    _hsv[0] = angle;
    _hsv[1] = Mat::ones(angle.size(), CV_32F);
    _hsv[2] = magn_norm;
    // merge(_hsv, 3, hsv);
    // hsv.convertTo(hsv8, CV_8U, 255.0);
    // cvtColor(hsv8, bgr, COLOR_HSV2BGR);
    
    return _hsv[2];
  }