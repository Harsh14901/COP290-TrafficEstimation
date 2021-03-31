#include <config.hpp>
#include <executor.hpp>
#include <performance.hpp>
#include <util/arg_parser.hpp>
#include <util/util.hpp>

using namespace cv;
using namespace std;

void show_usage(string name);
bool handle_arguments(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  if (!handle_arguments(argc, argv)) {
    show_usage(argv[0]);
    return -1;
  }

  auto method = stoi(arg_parser.get_argument_value("method"));
  auto complete_anly = arg_parser.get_bool_argument_value("complete_analysis");
  if (complete_anly) {
    complete_analysis(run);
  }
  if (method != 0) {
    perform_analysis(run, method);
  } else {
    runtime_params params;
    params.read_config_file(arg_parser.get_argument_value("config"));

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
  arg_parser.set_argument("config", "c", "./configs/config");
  arg_parser.set_standalone_argument("autoselect-points", "a");
  arg_parser.set_standalone_argument("debug", "d");
  arg_parser.set_standalone_argument("no-animation", "f");
  arg_parser.set_standalone_argument("skip-initial", "s");
  arg_parser.set_standalone_argument("train", "t");
  arg_parser.set_standalone_argument("complete_analysis", "y");

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
       << "\t-y, --complete_analysis \t\tRun a comprehensive analysis of the "
          "various runtime parameters\n"

       << endl;
}
