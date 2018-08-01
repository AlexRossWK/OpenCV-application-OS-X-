#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"

#include <chrono>

#include <iostream>
//#include <io.h>
#include <fcntl.h>




bool Inits();
//change_coiff_eye_distance = пусто, то изменений нет. + на 0.01, -0.01
//bool Detect(cv::Mat &captured_image, int frame_count, float fx, float fy, float cx, float cy, char change_coiff_eye_distance, std::string mac, std::string uid);
bool Detect(cv::Mat &captured_image, int frame_count, char change_coiff_eye_distance, std::string mac, std::string uid, std::string get_status_text);

bool Reset();
bool Clear();
void InitMat(cv::Mat result);

