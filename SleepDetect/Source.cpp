////это тест для тенниса
////рисуются точки и линяи, нужно полоджить точку на линию
//
//#include "opencv2/opencv.hpp"
//
//using namespace cv;
//
//int main() {
//	Mat image = Mat::zeros(640, 480, CV_8UC3);
//	
//
//	RNG rng(0xFFFFFFFF);
//
//	int width = 640, height = 480;
//	int x1 = width / 2, x2 = width * 3 / 2, y1 = -height / 2, y2 = height * 3 / 2;
//	
//
//	Point pt1, pt2;
//	pt1.x = rng.uniform(x1, x2);
//	pt1.y = rng.uniform(y1, y2);
//	pt2.x = rng.uniform(x1, x2);
//	pt2.y = rng.uniform(y1, y2);
//
//	line(image, pt1, pt2, Scalar(0, 0, 255), rng.uniform(1, 10));
//
//	imshow("test", image);
//
//	waitKey(0);
//	return 0;
//}
