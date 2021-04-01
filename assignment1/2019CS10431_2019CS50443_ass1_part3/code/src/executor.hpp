#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <pthread.h>

#include <config.hpp>
#include <gui.hpp>
#include <img_processor.hpp>
#include <img_transform.hpp>

#include "util/tqdm.h"

void run(runtime_params& params, density_t& density);

#endif