# Analysis Utility Error vs Runtime

This folder contains various scripts and generated plots, used for analysing various metrics.

### Scripts
1. *plotter.py*: Plots the queue and dynamic densities on a graph. It can also plot the error between two sets of densities
2. *analysis.py*: It plots the error vs runtime tradeoff for various methods used.
3. *multidimensional_plotter.py*: Plots the tradeoff analysis data for a combination of parameters on a 3-D plot


### Data
1. `plot_{method_name}.png`: Plots the runtime vs error data for the method `method_name` with each point annotated with the corresponding parameter value
2. `plot_{method_name}_eVt.png`: Plots error vs runtime for the method `method_name`
3. `plot_{method_name}_pVt.png`: Plots parameter vs runtime for the method `method_name`
4. `plot_{method_name}_pVe.png`: Plots parameter vs error for the method `method_name`
5. `plot_all.png`: Plots the error vs runtime for all methods in a single graph (excluding sparse optical flow)
6. `plot_dense_OF.png`: Plots the density data when dense optical flow is used
7. `plot_sparse_OF.png`: Plots the density data when sparse optical flow is used
8. `plot_sparse_dense.png`: Plots the error between sparse and dense optical density data.
9. `plot_lowres_error.png`: Plots the error in densities between high resolution and low resolution video streams.
10. `plot_split_frame_{n}_horizontal.png`: Plots the density data when split frame was split into `n` horizontal splits
11. `density_complete.csv`: baseline density data including both queue and dynamic densities

