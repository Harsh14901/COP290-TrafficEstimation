// Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>
#include <util.hpp>

#include "arg_parser.hpp"
using namespace cv;
using namespace std;

vector<Point> start_points;
Mat input_file, input_file_bnw, input_display;
string input_file_name, output_file_name;
bool auto_point;

const auto dot_color = Scalar(255, 0, 0), line_color = Scalar(255, 0, 0),
           fill_color = Scalar(100, 100, 0);

const auto original_name = "original", transformed_name = "transformed",
           cropped_name = "cropped";

void initialize_images();
void show_usage(string name);
bool handle_arguments(int argc, char* argv[]);
void save_images(Mat& transformed_img, Mat& cropped_img);

int main(int argc, char* argv[]) {
  if (!handle_arguments(argc, argv)) {
    show_usage(argv[0]);
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

//   Mat intermediate_img, transformed_img, cropped_img;
//   transform_image(input_file_bnw, intermediate_img,
//                   selection_window.start_points);
//   remove_black_borders(intermediate_img, transformed_img);

//   Window(transformed_name, transformed_img).show();



	Mat intermediate_img, transformed_img, cropped_img;
	vector<Point> processed_start_points = get_points_in_order(selection_window.start_points);
	transform_image(input_file_bnw, intermediate_img, processed_start_points);
	remove_black_borders(intermediate_img, transformed_img);
  	Window(transformed_name, transformed_img).show();


	crop_end_pts(transformed_img, cropped_img, processed_start_points);
	
	Window(cropped_name, cropped_img).show();

  save_images(transformed_img, cropped_img);

  return 0;
}

bool handle_arguments(int argc, char *argv[])
{
	arg_parser.set_argument("input", "i", "input_files/empty.jpg");
	arg_parser.set_argument("output", "o", "output_files/empty_out.jpg");
	arg_parser.set_standalone_argument("auto_points", "a");

	bool flag = arg_parser.parse_arguments(argc, argv);
	if (flag)
	{
		auto_point = arg_parser.get_bool_argument_value("auto_points");
	}
	return flag;
}
void show_usage(string name) {
  std::cerr
      << "Usage: " << name << "\n"
      << "Options:\n"
      << "\t-h, --help\t\tShow this help message\n"
      << "\t-i, --input\t\tSpecify the input file path. Default is "
         "./input_files/empty.jpg\n"
      << "\t-o, --output\t\tSpecify the output directory path. Default is "
         "./output_files\n"
      << "\t-a, --auto_points \tSelect second set of points automatically\n"
      << std::endl;
}


void initialize_images() {
  string file_name = arg_parser.get_argument_value("input");
  cout << "[+] Loading File: " << file_name << endl;

  input_file = imread(file_name, IMREAD_COLOR);
  cvtColor(input_file, input_file_bnw, COLOR_BGR2GRAY);
  cvtColor(input_file, input_display, COLOR_BGR2BGRA);
}

// void mouse_callback(int event, int x, int y, int, void *)
// {
// 	if (event == EVENT_LBUTTONDOWN)
// 	{
// 		printf("[+] Point selected: %d , %d \n", x, y);
// 		auto pt = Point(x, y);
// 		start_points.push_back(pt);

// 		circle(input_display, pt, 8, dot_color, -1);
		
		

// 		if (start_points.size() > 1)
// 		{
// 			const int n = start_points.size();
// 			line(input_display, start_points[n - 1], start_points[n - 2], line_color,3);
// 		}

// 		if (start_points.size() == 4)
// 		{
// 			line(input_display, start_points[3], start_points[0], line_color,3);
// 			display_polygon(input_display, start_points, fill_color, fill_alpha,
// 							original_window);
// 		}
// 		else
// 		{
// 			imshow(original_window, input_display);
// 		}
// 	}
// }


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
