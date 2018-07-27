#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>

#include <iostream>



using namespace std;
using namespace cv;

//this is a sample for foreground detection functions
int main_etye(int argc, const char** argv)
{
	

	bool useCamera = true;
	bool smoothMask = true;
	string method = "knn1";
	
	VideoCapture cap;
	bool update_bg_model = true;

	cap.open(0);
	
	if (!cap.isOpened())
	{
		printf("can not open camera or video file\n");
		return -1;
	}

	namedWindow("image", WINDOW_NORMAL);
	namedWindow("foreground mask", WINDOW_NORMAL);
	namedWindow("foreground image", WINDOW_NORMAL);
	namedWindow("mean background image", WINDOW_NORMAL);

	Ptr<BackgroundSubtractor> bg_model = method == "knn" ?
		createBackgroundSubtractorKNN().dynamicCast<BackgroundSubtractor>() :
		createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();

	Mat img0, img, fgmask, fgimg;

	for (;;)
	{
		cap >> img0;

		if (img0.empty())
			break;


		resize(img0, img, Size(120, 120));

		if (fgimg.empty())
			fgimg.create(img.size(), img.type());

		//update the model
		bg_model->apply(img, fgmask, update_bg_model ? -5 : 1);
		if (smoothMask)
		{
			GaussianBlur(fgmask, fgmask, Size(11, 11), 3.5, 3.5);
			threshold(fgmask, fgmask, 10, 255, THRESH_BINARY);
		}

		fgimg = Scalar::all(0);
		img.copyTo(fgimg, fgmask);

		Mat bgimg;
		bg_model->getBackgroundImage(bgimg);

		imshow("image", img);
		imshow("foreground mask", fgmask);
		imshow("foreground image", fgimg);
		if (!bgimg.empty())
			imshow("mean background image", bgimg);

		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i > hierarchy;

        cv::findContours(fgmask, contours, hierarchy, 2, CHAIN_APPROX_NONE);

		double aray = 0.0;
		for (unsigned int i = 0; i < contours.size(); i++)
		{
			aray += contourArea(contours[i]);
		}
		std::cout << " Area: " << aray << std::endl;



		char k = (char)waitKey(30);
		if (k == 27) break;
		if (k == ' ')
		{
			update_bg_model = !update_bg_model;
			if (update_bg_model)
				printf("Background update is on\n");
			else
				printf("Background update is off\n");
		}
	}

	return 0;
}
