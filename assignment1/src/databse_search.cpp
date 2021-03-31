#include "database_searc.hpp"
    
runtime_params get_runtime_params_from_string(string rp_str){
    runtime_params rp;

    stringstream s(rp_str);
    string val;


    getline(s, val,';');
    rp.skip_frames = stoi(val);
    getline(s, val,';');
    rp.sparse_optical_flow = stoi(val);
    getline(s, val,';');
    rp.calc_dynamic_density = stoi(val);

    getline(s, val,';');
    rp.resolution.width = stoi(val.substr(0,val.find('X')));
    rp.resolution.height = stoi(val.substr(val.find('X')+1));

    getline(s, val,';');
    rp.split_frame = stoi(val);
    getline(s, val,';');
    rp.split_video = stoi(val);

    return rp;
}

vector<pair<runtime_params,pair<double,double>>> get_data(){
    vector<pair<runtime_params,pair<double,double>>> data;

    string pVeFile = "./output_files/complete_analysis_pVe.csv";
    string pVtFile = "./output_files/complete_analysis_pVt.csv";

    
  
    fstream fin;
    fstream fin_t;
    // Open an existing file
    fin.open(pVeFile, ios::in);
    fin_t.open(pVtFile, ios::in);

    string temp,temp2,line,word;

    // fin >> temp;
    // fin_t >> temp2;
    getline(fin, line);
    getline(fin_t, line);


    while(!fin.eof()){

        getline(fin, line);
        if(line=="" || line=="\n") continue;

        stringstream s(line);

        string param_str;
        getline(s, param_str, ',');

        string error_str;
        getline(s, error_str, ',');

        getline(fin_t, line);
        stringstream s2(line);

        getline(s2, param_str, ',');

        string runtime_str;
        getline(s2, runtime_str, ',');

        stod(error_str);
        
        data.push_back(pair<runtime_params,pair<double,double>>(get_runtime_params_from_string(param_str),pair<double,double>(stod(error_str),stod(runtime_str))));

    }

    return data;
}


pair<runtime_params,pair<double,double>> getOptimalConfigWithErrorConstraint(double max_error){

    // Load file
    auto data =  get_data();  
    // Assume data has all the data  <Error,Time>


    pair<runtime_params,pair<double,double>> best_metric;



    double max_time = 100000;

    for(auto data_elem: data){
        cout << data_elem.second.first << endl;
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