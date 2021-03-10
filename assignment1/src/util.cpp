#include <util.hpp>

using namespace std;
using namespace cv;

string get_image_name(string window_name, string output_dir) {
  // auto output_dir = arg_parser.get_argument_value("output");
  if (output_dir.substr(output_dir.size() - 1, output_dir.size()) != "/") {
    output_dir += "/";
  }
  return output_dir + window_name + ".jpg";
}

void validate_directory(string output_dir) {
  struct stat info;
  char *dirpath = &output_dir[0];
  if (stat(dirpath, &info) != 0) {
    if (mkdir(dirpath, 0755)) {
      throw runtime_error("Unable to create a directory: " + output_dir);
    }
  } else if (!(info.st_mode & S_IFDIR)) {
    throw runtime_error("Not a directory: " + output_dir);
  }
}

void outputCSV(vector<pair<double, double>> &density, double frame_rate) {
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

void save_images(Mat &transformed_img, Mat &cropped_img) {
  auto output_dir = arg_parser.get_argument_value("output");

  auto transformed_img_name = get_image_name(transformed_name, output_dir);
  auto cropped_img_name = get_image_name(cropped_name, output_dir);

  try {
    validate_directory(output_dir);

    imwrite(transformed_img_name, transformed_img);
    cout << "[+] Transformed image saved to: " << transformed_img_name << endl;

    imwrite(cropped_img_name, cropped_img);
    cout << "[+] Cropped image saved to: " << cropped_img_name << endl;
  } catch (const std::exception &e) {
    cerr << "[-] Unable to save images" << endl;
    std::cerr << e.what() << '\n';
  }
}