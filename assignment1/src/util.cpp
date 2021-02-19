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
    if (!mkdir(dirpath, 0755)) {
      throw runtime_error("Unable to create a directory: " + output_dir);
    }
  } else if (!(info.st_mode & S_IFDIR)) {
    throw runtime_error("Not a directory: " + output_dir);
  }
}
