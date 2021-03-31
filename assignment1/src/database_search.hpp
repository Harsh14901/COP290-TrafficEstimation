#include <opencv2/opencv.hpp>

#include "config.hpp"

using namespace cv;
using namespace std;

vector<pair<runtime_params, pair<double, double>>> get_data();
pair<runtime_params, pair<double, double>> getOptimalConfigWithErrorConstraint(
    double max_error);
pair<runtime_params, pair<double, double>> getOptimalConfigWithTimeConstraint(
    double max_time);
