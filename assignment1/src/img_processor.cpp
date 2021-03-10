#include <img_processor.hpp>

void remove_black_borders(const Mat &src, Mat &dst) {
  Mat bnw, gray;
  if (src.channels() == 3) {
    cvtColor(src, gray, COLOR_BGR2RGB);
  } else {
    gray = src;
  }
  threshold(gray, bnw, 0, 255, THRESH_BINARY);
  vector<vector<Point>> contours;
  findContours(bnw, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
  Rect crop_rect = boundingRect(contours[0]);
  dst = src(crop_rect);
}

void train_static_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub, const Mat& bg_img, Mat &fg_mask) {
  Mat frame = bg_img;
  preprocess_frame(frame);

  for (int i = 0; i < bg_sub->getHistory(); i++) {
    bg_sub->apply(frame, fg_mask, learning_rate);
  }
}

void train_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2> &bg_sub,
                  VideoCapture& cap, Mat &fg_mask) {
  Mat frame;
  for (int i = 0; i < bg_sub->getHistory(); i++) {
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished before training" << endl;
      throw "Insufficient Video Stream";
    }
    preprocess_frame(frame);
    
    bg_sub->apply(frame, fg_mask, learning_rate);
  }
}

void opticalFlow(const Mat &prvs, const Mat &next, Mat& dst) {
  Mat flow(prvs.size(), CV_32FC2);
  calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

  // visualization
  Mat flow_parts[2];

  split(flow, flow_parts);

  Mat magnitude, angle, magn_norm;

  cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
  normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
  angle *= ((1.f / 360.f) * (180.f / 255.f));

  // build hsv image
  Mat _hsv[3], hsv, hsv8, bgr;
  _hsv[0] = angle;
  _hsv[1] = Mat::ones(angle.size(), CV_32F);
  _hsv[2] = magn_norm;

  merge(_hsv, 3, hsv);

  inRange(hsv, Scalar(0.47, 0, 0.2), Scalar(0.57, 255, 1), dst);
  hsv.convertTo(hsv8, CV_8U, 255.0);
  // cvtColor(hsv8, bgr, COLOR_HSV2BGR);
  // return _hsv[2];
}


void preprocess_frame(Mat& frame){
  Mat temp;
  cvtColor(frame, temp, COLOR_BGR2GRAY);

  transform_image(temp, frame);
  remove_black_borders(frame, temp);
  crop_end_pts(temp, frame);
}

void reduce_noise(Mat& fg_mask, const Mat& kernel){
  Mat processed_img;
  // TODO: Change the number of iterations of the three ops below.
    morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 4);
    morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);
    morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 3);
    morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);

    morphologyEx(fg_mask, processed_img, MORPH_DILATE, kernel, Point(-1, -1),
                 4);

    threshold(processed_img, fg_mask, 240, 255, THRESH_BINARY);
}


pair<double, double> compute_density(const Mat& fg_mask, Mat& dynamic_img){
  Mat bnw_dynamic;
    threshold(dynamic_img * 255, bnw_dynamic, 60, 255, THRESH_BINARY);
    Mat processed_bnw_dynamic;
    // morphologyEx(bnw_dynamic, processed_bnw_dynamic, MORPH_ERODE,
    // large_kernel, Point(-1, -1),
    //         1);
    // morphologyEx(bnw_dynamic,processed_bnw_dynamic, MORPH_DILATE, kernel,
    // Point(-1, -1),
    //           2);

    bnw_dynamic.copyTo(processed_bnw_dynamic);

    // Calculate the suitable area
    Mat final_static, final_static2;
    processed_bnw_dynamic.convertTo(final_static2, fg_mask.depth());
    bitwise_or(fg_mask, final_static2, final_static);

    double area = cv::sum(final_static).val[0] /
                  (255.0 * final_static.cols * final_static.rows);
    double dynamic_area =
        cv::sum(processed_bnw_dynamic).val[0] /
        (255.0 * processed_bnw_dynamic.cols * processed_bnw_dynamic.rows);
    dynamic_img = processed_bnw_dynamic;
    return make_pair(area, dynamic_area);
}
