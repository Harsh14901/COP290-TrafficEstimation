#include "config.hpp"

void runtime_params::set_values(int skip_frames,bool sparse_optical_flow,bool calc_dynamic_density,int res_scale_factor,int split_frame,int split_video){
    this->skip_frames = skip_frames;
    this->sparse_optical_flow = sparse_optical_flow;
    this->calc_dynamic_density = calc_dynamic_density;
    this->resolution = this->resolution/res_scale_factor;
    this->split_frame = split_frame;
    this->split_video = split_video;
}

void runtime_params::read_config_file(string filename){
    ifstream File(filename);
    
    string val;
    getline (File, val);
    this->skip_frames = stoi(val);
    getline (File, val);
    this->sparse_optical_flow = stoi(val);
    getline (File, val);
    this->calc_dynamic_density = stoi(val);
    getline (File, val);
    this->resolution = this->resolution/stoi(val);
    getline (File, val);
    this->split_frame = stoi(val);
    getline (File, val);
    this->split_video = stoi(val);

}


  