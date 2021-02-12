//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
	//open the video file for reading
	VideoCapture cap("./input_files/nfs.mp4");

	// for(int i=0;i<100000;i++){
	// 	for(int j=0;j<100000;j++){
	// 		int k = 3;
	// 	}
	// }

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video file" << endl;
		cin.get(); //wait for any key press
		return -1;
	}

	//Uncomment the following line if you want to start the video in the middle
	//cap.set(CAP_PROP_POS_MSEC, 300);

	//get the frames rate of the video
	double fps = cap.get(CAP_PROP_FPS);
	double height = cap.get(CAP_PROP_FRAME_HEIGHT);
	double width = cap.get(CAP_PROP_FRAME_WIDTH);

	cout << "Height is " << height << " Width is " << width << endl;

	Point pt;
	pt.x = width/36;
	pt.y = height/12;

	

	cout << "Frames per seconds : " << fps << endl;

	String window_name = "My First Video";

	namedWindow(window_name, WINDOW_NORMAL); //create a window
	int mode = 1;
	TickMeter tm;
	tm.start();
	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); // read a new frame from video

		Mat new_img = Mat::zeros(frame.size(), frame.type());

		double alpha = 1.4;
		double beta = -30;

		tm.stop();
		double fps_calc = tm.getFPS();
		tm.start();


		RNG rng;
		Scalar color = Scalar(0,0,255);
		fps = cap.get(CAP_PROP_FPS);
		putText(frame,to_string(fps_calc),pt,2,2,color,3);

		frame.convertTo(new_img, -1, alpha, beta);

		
		Mat bnw = Mat::zeros(frame.size(), frame.type());

		cvtColor(frame, bnw, COLOR_BGR2GRAY);
		

		//Breaking the while loop at the end of the video
		if (bSuccess == false)
		{
			cout << "Found the end of the video" << endl;
			break;
		}

		//show the frame in the created window
		if(mode == 0){
			imshow(window_name, bnw);
		}else{
			imshow(window_name,new_img);
		}

		//wait for for 10 ms until any key is pressed.
		//If the 'Esc' key is pressed, break the while loop.
		//If the any other key is pressed, continue the loop
		//If any key is not pressed withing 10 ms, continue the loop
		int key = waitKey(10);
		if (key == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
		if(key == 'q'){
			mode = mode==0?1:0;
		}
	}

	return 0;
}