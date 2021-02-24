# Traffic density estimation using OpenCV
[![C++ version][c++-image]][c++-wiki]
[![C++ style guide][gts-image]][gts-url]


This is a `C++` tool to estimate traffic density using OpenCV from camera snapshots of road traffic.

## Usage

Clone the repo and inside the `assignment1` directory run the `make` command to build an executable for the project. To run the executable use `make run`.
- A window will popup with a default image loaded. The user can now select 4 points on the image which will be displayed using an interactive UI. 
- Use **LMB** to select a point and **RMB** to delete the previous point from selection. After 4 points are selected a rectangle will be displayed indicating the selection. 
- Note that it is **not** necessary to select the points in counter clockwise order, you can select any 4 random points the program will automatically select a convex quadrilateral out of the 4 points.
- To switch to the next window press the `n` button on the keyboard. The image will be transformed through an amazing *animation*. 
- Once the transformation is complete you can press the `n` key again to display the cropped image. 
- Pressing `n` again will close the window and save both the files in default directory.

## Features

We have provided extensive customizability through various options on the CLI. A typical help window looks like:

```bash
$ bin/main -h
Usage: bin/main
Options:
	-h, --help		Show this help message
	-i, --input		Specify the input file path. Default is ./input_files/empty.jpg
	-o, --output		Specify the output directory path. Default is ./output_files
	-a, --autoselect-points 	Select second set of points automatically
	-d, --debug 		Display debug output
	-f, --no-animation 		Do not display animation


```

1. `-i , --input`: This flag can be used to specify the input image. By default the image `./input_files/empty.jpg` is loaded
2. `-o, --output`: This flag can be used to specify the directory for saving images. By default they are saved in `./output_files`
3. `-d, --debug`: This flag aids in debugging by printing debug output to the console.
4. `-a, --autoselect-points`: This flag automatically selects the second set of points for transforming the image by creating a rectangle around the selection.
5. `-f, --no-animation`: This flag disables the animation that shows the transformation of the original image.

To run the application with any of the command line flags specify it in the `args` parameter of `make run`
For example 
```bash
$ make run args="-d -a -i /tmp/input.jpg"
```
The above command will run the application with input file as `/tmp/input.jpg` and display debug output and select the second set of points automatically.

## Developers

1. [Pranjal Aggarwal | 2019CS50443 | IIT DELHI](http://github.com/pranjal2041/)
2. [Harsh Agrawal | 2019CS10431 | IIT DELHI](http://github.com/Harsh14901/)

If you face any issue, feel free to raise an issue in the [Issues](https://github.com/Pranjal2041/COP290/issues) section of the repo. The developers would be happy to help :).

[gts-image]: https://img.shields.io/badge/code%20style-google-blueviolet.svg
[gts-url]: https://google.github.io/styleguide/cppguide.html
[c++-image]: https://img.shields.io/badge/c%2B%2B-17-blue
[c++-wiki]: https://en.wikipedia.org/wiki/C%2B%2B17
