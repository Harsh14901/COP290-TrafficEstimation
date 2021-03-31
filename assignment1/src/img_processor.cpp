#include <img_processor.hpp>

void remove_black_borders(const Mat& src, Mat& dst) {
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

void train_static_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub,
                        const Mat& bg_img, Mat& fg_mask,
                        const Size& resolution) {
  Mat frame = bg_img.clone();
  preprocess_frame(frame, resolution);

  for (int i = 0; i < bg_sub->getHistory(); i++) {
    bg_sub->apply(frame, fg_mask, learning_rate);
  }
}

void train_bgsub(cv::Ptr<cv::BackgroundSubtractorMOG2>& bg_sub,
                 VideoCapture& cap, Mat& fg_mask, const Size& resolution) {
  Mat frame;
  for (int i = 0; i < bg_sub->getHistory(); i++) {
    cap.read(frame);
    if (frame.empty()) {
      cerr << "[+] Video stream finished before training" << endl;
      throw "Insufficient Video Stream";
    }
    preprocess_frame(frame, resolution);

    bg_sub->apply(frame, fg_mask, learning_rate);
  }
}

void opticalFlow(const Mat& prvs, const Mat& next, Mat& dst) {
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

void preprocess_frame(Mat& frame, const Size& resolution) {
  Mat temp;
  resize(frame, temp, resolution, 0, 0, INTER_CUBIC);
  frame = temp;
  cvtColor(frame, temp, COLOR_BGR2GRAY);

  transform_image(temp, frame, 1, resolution);
  remove_black_borders(frame, temp);
  crop_end_pts(temp, frame, resolution);
}

// Takes grayscale images as input
void sparseOpticalFlow(Mat& prvs, Mat& next, Mat& fg_mask,Mat& dst, int frame_skip){
  

  dst = Mat(next.rows, next.cols, CV_8UC1, Scalar(0));

  int frame_area = prvs.rows*prvs.cols;

  vector<Point2f> p0, p1,p0r;
  goodFeaturesToTrack(prvs, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

  if(p0.size()==0){
    return; 
  }

  // calculate optical flow
  vector<uchar> status;
  vector<float> err;
  TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);

  calcOpticalFlowPyrLK(prvs, next, p0, p1, status, err, Size(15,15), 2, criteria);
  calcOpticalFlowPyrLK(next, prvs, p1, p0r, status, err, Size(15,15), 2, criteria); //Calculates in reverse order


  vector<Point2f> good_points;

  for(uint i = 0; i < p0.size(); i++)
  {
      if(status[i]!=1) continue;

      float good_point_val = max(abs(p0r.at(i).y-p1.at(i).y),abs(p0r.at(i).x-p1.at(i).x));
      if(good_point_val>1){

        if(p1.at(i).y-p0.at(i).y>-4*frame_skip) continue;
        good_points.push_back(p1[i]);
      }

  } 



  vector<vector<Point>> contours;
  findContours(fg_mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

  vector<vector<Point>> goodContours; 
  for(vector<Point> cont : contours){
    int tot = 1;
    for(int i=0;i<good_points.size();i++){
      int val = pointPolygonTest(cont,good_points.at(i),false);
      if(val==-1) continue;
      tot+=1;
    }
    if(tot>=4){
      if(contourArea(cont)>0.5*frame_area) continue;
      goodContours.push_back(cont);
    }
  }

  fillPoly(dst,goodContours,Scalar(255));

}


void reduce_noise(Mat& fg_mask, const Mat& kernel) {
  Mat processed_img;
  // TODO: Change the number of iterations of the three ops below.
  morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 4);
  morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);
  morphologyEx(fg_mask, processed_img, MORPH_CLOSE, kernel, Point(-1, -1), 3);
  morphologyEx(processed_img, fg_mask, MORPH_OPEN, kernel, Point(-1, -1), 2);

  morphologyEx(fg_mask, processed_img, MORPH_DILATE, kernel, Point(-1, -1), 4);

  threshold(processed_img, fg_mask, 240, 255, THRESH_BINARY);
}

pair<double, double> compute_density(const Mat& fg_mask, Mat& dynamic_img) {
  Mat bnw_dynamic;
  threshold(dynamic_img * 255, bnw_dynamic, 60, 255, THRESH_BINARY);
  Mat processed_bnw_dynamic;
  
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


pair<double,double> compute_density(const Mat& fg_mask){
  return make_pair(sum(fg_mask).val[0] /
                (255.0 * fg_mask.cols * fg_mask.rows), 0);
}