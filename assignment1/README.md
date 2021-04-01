# Traffic density estimation using OpenCV
[![C++ version][c++-image]][c++-wiki]
[![C++ style guide][gts-image]][gts-url]


This is a `C++` tool to estimate traffic density using OpenCV from camera snapshots of road traffic.

## Usage

To download the input video file just run `make video` or specify the custom path as explained in the [Features](#features) section.
### UI
Clone the [repo](https://github.com/Pranjal2041/COP290) and inside the `assignment1` directory run the `make` command to build an executable for the project. To run the executable use `make run`.
- A window will popup with a default image loaded. The user can now select 4 points on the image which will be displayed using an interactive UI. 
- Use **LMB** to select a point and **RMB** to delete the previous point from selection. After 4 points are selected a rectangle will be displayed indicating the selection. 
- Note that it is **not** necessary to select the points in counter clockwise order, you can select any 4 random points the program will automatically select a convex quadrilateral out of the 4 points.
- To switch to the next window press the `n` button on the keyboard. The image will be transformed through an amazing *animation*. 
- Once the transformation is complete you can press the `n` key again to start processing of the video.

### Output
- The console output will show the amount of frames processed and the corresponding frame rate using an animated progress bar.
- Once the processing is complete the output data is saved to `./output_files/density.csv`. 
- The plot for the data can be seen by running the command `make plot`. This will popup a window with the image of the graph.
- The graph is automatically saved to `./output_files/plot.png`, however the user has the option to save the file elsewhere by clicking on the save icon in the graph window.

**NOTE:** These default actions were put in place to favour ease of use. However we have provided extensive customizability through various command line option as specified in the section below.

## <a id="features">Features</a>

We have provided extensive customizability through various options on the CLI. A typical help window looks like:

```bash
$ bin/main -h
Usage: bin/main
Options:
	-h, --help		Show this help message
	-i, --input		Specify the input file path. Default is ./input_files/trafficvideo.mp4
	-o, --output		Specify the output csv file. Default is ./output_files/density.csv
	-a, --autoselect-points 	Select second set of points automatically
	-d, --debug 		Display debug output
	-f, --no-animation 		Do not display animation
	-s, --skip-initial 		Skip initial selection of points
	-q, --quick 		Output a quick result by skipping frames
	-t, --train-bg 		Auto train the background of video
	-m, --method            Runtime analysis method {1,2,3,4,5}
	-y, --complete_analysis                 Run a comprehensive analysis of the various runtime parameters


```

1. `-i , --input`: This flag can be used to specify the input video file. By default the video `./input_files/trafficvideo.mp4` is loaded
2. `-o, --output`: This flag can be used to specify the directory for CSV file. By default it is saved as  `./output_files/density.csv`
3. `-d, --debug`: This flag aids in debugging by printing debug output to the console and displaying the stages of processing as 3 videos.
4. `-a, --autoselect-points`: This flag automatically selects the second set of points for transforming the image by creating a rectangle around the selection.
5. `-f, --no-animation`: This flag disables the animation that shows the transformation of the original image.
6. `-s, --skip-initial`: This flag will skip the initial process of selecting the points to transform the perspective. This causes the program to take up default points which are currently hardcoded.
7. `-q, --quick`: This flag processes every third frame instead of the original FPS. This decreases the time taken to generate the output by a fraction of 3. **Note:** On a *i7* intel processor the program took 10 minutes to generate the output for a 6 minute video at full FPS. When this option was enabled the process completed in 3.5 minutes with only minor differences in the plot produced.
8. `-t, --train-bg`: This option auto trains the background of the input video using some starting frames. By default the file `./input_files/empty.jpg` is used for background subtraction.
9. `-m --method`: This option runs program on various parameters and plots the utility error vs runtime graph. The 5 options indicates the 5 different parameters that will be individually tweaked in each case.
10. `-y --complete_analysis`: Runs a complete analysis by running model on 54 different set of parameters, and creates a database containing error, runtime for each set.

To run the application with any of the command line flags specify it in the `args` parameter of `make run`
For example 
```bash
$ make run args="-d -s -q -i /tmp/trafficvideo.mp4"
```
The above command will run the application with input file as `/tmp/trafficvideo.mp4` and display debug output and the 3 video processing stages in different windows. The `-s` option ensures that user is not prompted for selection of the input points and the program takes on the default values. The `-q` option ensures that the processing is made quicker 3 times.

## Developers

1. [Pranjal Aggarwal | 2019CS50443 | IIT DELHI](http://github.com/pranjal2041/)
2. [Harsh Agrawal | 2019CS10431 | IIT DELHI](http://github.com/Harsh14901/)

If you face any issue, feel free to raise an issue in the [Issues](https://github.com/Pranjal2041/COP290/issues) section of the repo. The developers would be happy to help :).

[gts-image]: https://img.shields.io/badge/code%20style-google-blueviolet.svg
[gts-url]: https://google.github.io/styleguide/cppguide.html
[c++-image]: https://img.shields.io/badge/c%2B%2B-17-blue
[c++-wiki]: https://en.wikipedia.org/wiki/C%2B%2B17
