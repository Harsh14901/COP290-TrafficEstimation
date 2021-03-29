#include <performance.hpp>

using namespace std;

void perform_analysis(run_t f, int method) {
  switch (method) {
    case 1:
      method1(f);
      break;
    // case 2:
    //   method2(f);
    //   break;
    case 3:
      method3(f);
      break;
    case 4:
      method4(f);
      break;
    case 5:
      method5(f);
      break;

    default:
      throw runtime_error("Unsupported method : " + to_string(method));
  }
}

double compute_utility(density_t& baseline_density, density_t& test_density) {
  assert(baseline_density.size() == test_density.size());
  double utility = 0.0;
  int n = baseline_density.size();

  for (int i = 0; i < n; i++) {
    auto baseline_diff =
        abs(baseline_density[i].first - baseline_density[i].second);
    auto test_diff = abs(test_density[i].first - test_density[i].second);
    auto diff = abs(test_diff - baseline_diff);
    utility += diff * diff;
  }
  return utility;
}

double time_execution(run_t f, runtime_params& params, density_t& density) {
  auto start = std::chrono::steady_clock::now();
  f(params, density);
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(end - start).count();
}
void analyze(run_t f, runtime_params& baseline_params,
             vector<runtime_params>& test_params, string out_file) {
  auto results = result_t();

  auto baseline_density = density_t();
  auto baseline_time = time_execution(f, baseline_params, baseline_density);
  printf("[#] BASELINE TIME: %f\n", baseline_time);
  results.push_back(make_pair(0, baseline_time));

  for (auto& test_param : test_params) {
    auto test_density = density_t();
    auto test_time = time_execution(f, test_param, test_density);
    auto utility = compute_utility(baseline_density, test_density);

    printf("[#] TEST TIME: %f\n", test_time);

    results.push_back(make_pair(utility, test_time));
  }

  outputCSV(out_file, results, "utility_error,time");
}

void method1(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  for (int i = 3; i < 7; i += 2) {
    auto test_param = runtime_params{};
    test_param.skip_frames = i;
    test_params.push_back(test_param);
  }
  analyze(f, baseline_params, test_params, "./output_files/skip_frames.csv");
}

void method3(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  auto add_res = [&](int width, int height) {
    test_params.push_back(baseline_params);
    test_params.back().resolution = Size(width, height);
  };

  // add_res(1920, 1080);
  // add_res(1366, 768);
  // add_res(1536, 864);
  // add_res(1440, 900);
  // add_res(640, 360);
  // add_res(1600, 900);
  add_res(1024, 768);
  // add_res(1280, 1024);
  add_res(1280, 720);

  analyze(f, baseline_params, test_params, "./output_files/resolution.csv");
}

void method4(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  auto add_threads = [&](int num) {
    test_params.push_back(baseline_params);
    test_params.back().split_video = num;
  };

  add_threads(2);
  add_threads(4);
  add_threads(6);
  add_threads(8);

  analyze(f, baseline_params, test_params, "./output_files/split_video.csv");
}
void method5(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  auto add_threads = [&](int num) {
    test_params.push_back(baseline_params);
    test_params.back().split_frame = num;
  };

  add_threads(2);
  add_threads(4);
  // add_threads(6);
  // add_threads(8);

  analyze(f, baseline_params, test_params, "./output_files/split_frame.csv");
}