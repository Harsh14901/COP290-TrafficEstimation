//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

void mouseCallback( int event, int x, int y, int, void* );
Mat transformImage(Mat src, vector<Point2f> start_points);
Mat transformImage(Mat src, vector<Point2f> start_points, vector<Point2f> end_points);
vector<Point2f> getFinalPoints(Mat src, vector<Point2f> start_points);

vector<Point2f> start_points;

int main(int argc, char *argv[])
{

	// using namespace cv;
	// cv::

	Mat inp_file = imread("input_files/empty.jpg");

	Mat inp_file_bnw;

	cvtColor(inp_file, inp_file_bnw, COLOR_BGR2GRAY);

	namedWindow("Original", WINDOW_FREERATIO);
	setMouseCallback("Original", mouseCallback,0);
	imshow("Original", inp_file_bnw);
	waitKey(0);
	destroyWindow("Original");


	Mat dest = transformImage(inp_file_bnw,start_points);
	
	namedWindow("Transformed",WINDOW_FREERATIO);
	imshow("Transformed",dest);
	waitKey(0);
	destroyWindow("Transformed");

	Rect crop_rect;
	vector<Point2f> end_pts;
	end_pts = getFinalPoints(inp_file_bnw,start_points);
	crop_rect.x = end_pts.at(0).x;
	crop_rect.y = end_pts.at(0).y;

	crop_rect.width = end_pts.at(3).x - end_pts.at(0).x;
	crop_rect.height = end_pts.at(1).y - end_pts.at(0).y;

	printf("Here are co-ords %d %d %d %d \n",crop_rect.x,crop_rect.y,crop_rect.height,crop_rect.width);
	Mat cropped = dest(crop_rect);


	namedWindow("Cropped",WINDOW_FREERATIO);
	imshow("Cropped",cropped);
	waitKey(0);
	destroyWindow("Cropped");



	return 0;
}

void mouseCallback( int event, int x, int y, int, void* ){
	if(event == EVENT_FLAG_LBUTTON){
		printf("hurrah we have got a point %d , %d \n",x,y);
		start_points.push_back(Point2f(x,y));
	}

}

// Currently Destination Points are hardcoded
vector<Point2f> getFinalPoints(Mat src, vector<Point2f> start_points)
{
	vector<Point2f> end_points;
	end_points.push_back(Point2f(472, 52));
	end_points.push_back(Point2f(472, 830));
	end_points.push_back(Point2f(800, 830));
	end_points.push_back(Point2f(800, 52));

	return end_points;
}

Mat transformImage(Mat src, vector<Point2f> start_points){
	return transformImage(src,start_points,getFinalPoints(src,start_points));
}

Mat transformImage(Mat src, vector<Point2f> start_points, vector<Point2f> end_points)
{
	Mat homo = findHomography(start_points,end_points);

	Mat final_img;
	warpPerspective(src,final_img,homo,src.size());

	return final_img;
}