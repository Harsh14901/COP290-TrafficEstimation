#include "database_searc.hpp"


vector<pair<runtime_params,pair<double,double>>> get_data(){
    vector<pair<runtime_params,pair<double,double>>> data;
    return data;
}


pair<runtime_params,pair<double,double>> getOptimalConfigWithErrorConstraint(double max_error){

    // Load file
    auto data =  get_data();  
    // Assume data has all the data  <Error,Time>


    pair<runtime_params,pair<double,double>> best_metric;

    double max_time = 100000;

    for(auto data_elem: data){
        if(data_elem.second.first > max_error) continue;
        if(data_elem.second.second < max_time){
            best_metric = data_elem;
            max_time = data_elem.second.second;
        }
    }

    cout << "Best Params are:" << endl;
    best_metric.first.print_params();
    return best_metric;
}


pair<runtime_params,pair<double,double>> getOptimalConfigWithTimeConstraint(double max_time){

    // Load file
    auto data =  get_data();  
    // Assume data has all the data  <Error,Time>


    pair<runtime_params,pair<double,double>> best_metric;

    double max_error = 100000;

    for(auto data_elem: data){
        if(data_elem.second.first > max_time) continue;
        if(data_elem.second.first < max_error){
            best_metric = data_elem;
            max_error = data_elem.second.first;
        }
    }

    cout << "Best Params are:" << endl;
    best_metric.first.print_params();
    return best_metric;
}