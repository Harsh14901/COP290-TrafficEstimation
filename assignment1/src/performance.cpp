#include <performance.hpp>

using namespace std;

void perform_analysis(run_t f, int method) {
  switch (method) {
    case 1:
      method1(f);
      break;
    case 2:
      method2(f);
      break;
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
string get_test_param(runtime_params& test_params) {
  return to_string(test_params.skip_frames) + ";" +
         to_string(test_params.sparse_optical_flow) + ";" +
         to_string(test_params.calc_dynamic_density) + ";" +
         to_string(test_params.resolution.width) + "X" +
         to_string(test_params.resolution.height) + ";" +
         to_string(test_params.split_frame) + ";" +
         to_string(test_params.split_video);
}
string get_test_param(runtime_params& baseline_params,
                      runtime_params& test_params) {
  if (baseline_params.skip_frames != test_params.skip_frames) {
    return to_string(test_params.skip_frames);
  } else if (baseline_params.sparse_optical_flow !=
             test_params.sparse_optical_flow) {
    return to_string(test_params.sparse_optical_flow);
  } else if (baseline_params.calc_dynamic_density !=
             test_params.calc_dynamic_density) {
    return to_string(test_params.calc_dynamic_density);
  } else if (baseline_params.resolution != test_params.resolution) {
    return to_string(test_params.resolution.width) + "X" +
           to_string(test_params.resolution.height);
  } else if (baseline_params.split_frame != test_params.split_frame) {
    return to_string(test_params.split_frame);
  } else if (baseline_params.split_video != test_params.split_video) {
    return to_string(test_params.split_video);
  }
  return "";
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
  return sqrt(utility);
}

double time_execution(run_t f, runtime_params& params, density_t& density) {
  auto start = std::chrono::steady_clock::now();
  f(params, density);
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

void analyze(run_t f, runtime_params& baseline_params,
             vector<runtime_params>& test_params, string out_file,
             bool verbose) {
  result_t errorVtime = result_t(), paramVerror = result_t(),
           paramVtime = result_t();

  auto baseline_density = density_t();
  auto baseline_time = time_execution(f, baseline_params, baseline_density);
  printf("[#] BASELINE TIME: %f\n", baseline_time);
  errorVtime.push_back(make_pair(to_string(0), to_string(baseline_time)));
  paramVerror.push_back(make_pair("", "0"));
  paramVtime.push_back(make_pair("", to_string(baseline_time)));

  int i = 0;

  for (auto& test_param : test_params) {
    i++;
    cout << "[#] Params for Test " << i << " are: " << endl;
    test_param.print_params();

    auto test_density = density_t();
    auto test_time = time_execution(f, test_param, test_density);
    auto utility = compute_utility(baseline_density, test_density);

    printf("[#] TEST TIME: %f\n", test_time);

    errorVtime.push_back(make_pair(to_string(utility), to_string(test_time)));
    auto param = (verbose) ? get_test_param(test_param)
                           : get_test_param(baseline_params, test_param);
    paramVerror.push_back(make_pair(param, to_string(utility)));
    paramVtime.push_back(make_pair(param, to_string(test_time)));
  }

  outputCSV(out_file + "_eVt.csv", errorVtime, "utility_error,time");
  outputCSV(out_file + "_pVe.csv", paramVerror, "param,error");
  outputCSV(out_file + "_pVt.csv", paramVtime, "param,time");
}

void method1(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  for (int i = 1; i < 8; i++) {
    auto test_param = runtime_params{};
    test_param.skip_frames = i;
    test_params.push_back(test_param);
  }
  analyze(f, baseline_params, test_params, "./output_files/skip_frames");
}

void method3(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  auto add_res = [&](int width, int height) {
    test_params.push_back(baseline_params);
    test_params.back().resolution = Size(width, height);
  };

  add_res(1536, 864);
  add_res(1600, 900);
  add_res(1440, 900);
  add_res(1366, 768);
  add_res(1280, 1024);
  add_res(1280, 720);
  add_res(1024, 768);
  add_res(640, 360);

  analyze(f, baseline_params, test_params, "./output_files/resolution");
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

  analyze(f, baseline_params, test_params, "./output_files/split_video");
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

  analyze(f, baseline_params, test_params, "./output_files/split_frame");
}

void method2(run_t f) {
  auto baseline_params = runtime_params{};
  vector<runtime_params> test_params;

  baseline_params.calc_dynamic_density = true;

  test_params.push_back(baseline_params);
  test_params.back().sparse_optical_flow = true;

  analyze(f, baseline_params, test_params, "./output_files/sparse_optical");
}

void complete_analysis(run_t f) {
  runtime_params baseline_params;
  vector<runtime_params> test_params;

  int split_vids[] = {1, 4, 8};
  int split_f[] = {1, 2, 4};
  bool sparse[] = {false, true};
  int resolutions[] = {1, 3, 5};
  int skip_frame[] = {0, 1, 3};

  for (int sv : split_vids) {
    for (int sf : split_f) {
      for (bool spa : sparse) {
        for (int res : resolutions) {
          for (int skf : skip_frame) {
            if (sv != 1 && sf != 1) {
              continue;
            }
            runtime_params rp;
            rp.set_values(skf, spa, true, res, sf, sv);
            test_params.push_back(rp);
          }
        }
      }
    }
  }

  analyze(f, test_params[0], test_params, "./output_files/complete_analysis",
          true);
}