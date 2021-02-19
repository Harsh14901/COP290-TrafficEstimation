// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <img_transform.hpp>
#include <util.hpp>
#include <img_processor.hpp>
#include <ArgParser.hpp>

using namespace cv;
using namespace std;

ArgParser arg_parser;
vector<Point> start_points;
Mat input_file, input_file_bnw, input_display;
string input_file_name,output_file_name;
bool auto_point;

const auto dot_color = Scalar(255, 0, 0), line_color = Scalar(255, 0, 0),
		   fill_color = Scalar(100, 100, 0);

const auto fill_alpha = 0.4;
const auto original_window = "original", transformed_window = "transformed",
		   cropped_window = "cropped";


void initialize_images();
void mouse_callback(int event, int x, int y, int, void *);
void show_usage(string name);
bool handle_arguments(int argc, char *argv[]);

int main(int argc, char *argv[])
{

	if(!handle_arguments(argc,argv)){
		show_usage(argv[0]);
		return -1;
	}

	initialize_images();

	display_window(original_window, input_display, mouse_callback);
	if (start_points.size() != 4)
	{
		cout << "[-] User did not select 4 points, Exiting." << endl;
		return -1;
	}

	Mat intermediate_img,transformed_image, cropped_img;
	transform_image(input_file_bnw, intermediate_img, start_points,auto_point);
	remove_black_borders(intermediate_img,transformed_image);
	
	display_window(transformed_window, transformed_image);

	crop_end_pts(transformed_image, cropped_img,start_points,auto_point);
	display_window(cropped_window, cropped_img);

	return 0;
}

void show_usage(string name)
{
    std::cerr << "Usage: " << name << "\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-i,--input\t\tSpecify the input file path (required)\n"
			  << "\t-o,--output\t\tSpecify the output file path. Default is based on input file\n"
			  << "\t-a,--auto_points \tSelect second set of points automatically\n"
              << std::endl;
}

bool handle_arguments(int argc, char *argv[]){
	arg_parser.set_argument("input","i","input_files/empty.jpg");
	arg_parser.set_argument("output","o","output_files/empty_out.jpg");
	arg_parser.set_standalone_argument("auto_points","a");

	bool flag = arg_parser.parse_arguments(argc,argv);
	if(flag){
		auto_point = arg_parser.get_bool_argument_value("auto_points");
	}
	return flag;
}

void initialize_images()
{
	string file_name = arg_parser.get_argument_value("input");
	cout << "Loading File: " << file_name << endl;

	input_file = imread(file_name, IMREAD_COLOR);
	cvtColor(input_file, input_file_bnw, COLOR_BGR2GRAY);
	cvtColor(input_file, input_display, COLOR_BGR2BGRA);
}

void mouse_callback(int event, int x, int y, int, void *)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		printf("[+] Point selected: %d , %d \n", x, y);
		auto pt = Point(x, y);
		start_points.push_back(pt);

		circle(input_display, pt, 5, dot_color, -1);

		if (start_points.size() > 1)
		{
			const int n = start_points.size();
			line(input_display, start_points[n - 1], start_points[n - 2], line_color);
		}

		if (start_points.size() == 4)
		{
			line(input_display, start_points[3], start_points[0], line_color);
			display_polygon(input_display, start_points, fill_color, fill_alpha,
							original_window);
		}
		else
		{
			imshow(original_window, input_display);
		}
	}
}
