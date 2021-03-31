#ifndef PERFORMANCE_H
#define PERFORMANCE_H
#include <bits/stdc++.h>

#include <config.hpp>
#include <opencv2/opencv.hpp>
#include <util/util.hpp>
using namespace std;

typedef void (*run_t)(runtime_params&, density_t&);

double compute_utility(density_t& baseline_density, density_t& test_density);
double time_execution(run_t f, runtime_params& params, density_t& density);
void analyze(run_t f, runtime_params& baseline_params,
             vector<runtime_params>& test_params, string out_file,
             bool verbose = false);

void perform_analysis(run_t f, int method);
string get_test_param(runtime_params& baseline_params,
                      runtime_params& test_params);
string get_test_param(runtime_params& test_params);

void method1(run_t f);
void method2(run_t f);
void method3(run_t f);
void method4(run_t f);
void method5(run_t f);
void complete_analysis(run_t f);

#endif
