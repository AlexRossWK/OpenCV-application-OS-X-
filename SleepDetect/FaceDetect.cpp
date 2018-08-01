#include "FaceDetect.h"

#include <memory>
#include <iostream>
#include <stdio.h>
LandmarkDetector::FaceModelParameters det_parameters;
// The modules that are being used for tracking
LandmarkDetector::CLNF clnf_model;

std::chrono::high_resolution_clock::time_point time_start_program; //время начала работы программы
//каждые 10 секунд снимается лог по данным

cv::Rect_<double> bounding_box;

bool Inits()
{
     det_parameters.init();
    
    
    // det_parameters.model_location = std::string("/usr/local/model/main_clnf_general.txt");
     //det_parameters.face_detector_location = std::string("/usr/local/classifiers/haarcascade_frontalface_alt.xml");
    det_parameters.model_location = "model/main_clnf_general.txt";
    det_parameters.face_detector_location = "classifiers/haarcascade_frontalface_alt.xml";
    
    
    
   std::cout<< "model_location = " << det_parameters.model_location << std::endl;
   std::cout<< "face_detector_location = " << det_parameters.face_detector_location << std::endl;
    
    clnf_model.model_location_clnf = "model/main_clnf_general.txt";
    clnf_model.face_detector_location_clnf = "classifiers/haarcascade_frontalface_alt.xml";
   // clnf_model.model_location_clnf = std::string("/usr/local/model/main_clnf_general.txt");
   // clnf_model.face_detector_location_clnf = std::string("/usr/local/classifiers/haarcascade_frontalface_alt.xml");
    
    clnf_model.inits();

	time_start_program = std::chrono::high_resolution_clock::now();
	

    return true;
}

// Visualising the results
void visualise_tracking(cv::Mat& captured_image, cv::Mat& graf_image, cv::Mat_<float>& depth_image, const LandmarkDetector::CLNF& face_model, const LandmarkDetector::FaceModelParameters& det_parameters, 
	int frame_count, char change_coiff_eye_distance, std::string mac, std::string uid, std::string get_status_text)
{

	// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
	double detection_certainty = face_model.detection_certainty;
	bool detection_success = face_model.detection_success;

	double visualisation_boundary = 0.2;

	//time_start_program = std::chrono::high_resolution_clock::now();

	std::chrono::high_resolution_clock::time_point time_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = time_end - time_start_program;

	bool refresh_loging_time = fp_ms.count() > 10 * 1000; //если более 10 секунд обновляем логи усталости.
	if (refresh_loging_time)
	{
		time_start_program = time_end;
		//std::cout << refresh_loging_time << "------------------------------------------"std::endl;
	}
	/*else
		std::cout << refresh_loging_time << std::endl;*/



	// Only draw if the reliability is reasonable, the value is slightly ad-hoc
	if (detection_certainty < visualisation_boundary)
	{
		LandmarkDetector::Draw(captured_image, graf_image, face_model, bounding_box, refresh_loging_time, change_coiff_eye_distance, mac, uid, get_status_text);
	}
}

int row = 12;
cv::Mat marGraf;
cv::Ptr<cv::CLAHE> clahe;
cv::CascadeClassifier eye_cascade;

void InitMat(cv::Mat result)
{

	

	clahe = cv::createCLAHE();
	clahe->setClipLimit(1);

	eye_cascade.load("haarcascade2.xml");
	
}

// Some globals for tracking timing information for visualisation
//double fps_tracker = -1.0;
int64 t0 = 0;

//text1
//моргания в минуту
//сейчас
//в среднем здесь
//в среднем в БД
std::string col1_row1 = "blinking in a minute:";
std::string col1_row2 = "now";
std::string col1_row3 = "on average here";
std::string col1_row4 = "on average in the database";


//int index = 25;


void Clarity(int step, cv::Mat& m_imgEdit)
{
	//try
	//{
	if (step < 0)
	{
		cv::blur(m_imgEdit, m_imgEdit, cv::Size(-step * 2 + 1, -step * 2 + 1));
	}
	else
	{
		cv::Mat dst = m_imgEdit.clone();
		float matr[9]{
            static_cast<float>(-0.0375 - 0.05*step), static_cast<float>(-0.0375 - 0.05*step), static_cast<float>(-0.0375 - 0.05*step),
            static_cast<float>(-0.0375 - 0.05*step), static_cast<float>(1.3 + 0.4*step), static_cast<float>(-0.0375 - 0.05*step),
            static_cast<float>(-0.0375 - 0.05*step), static_cast<float>(-0.0375 - 0.05*step), static_cast<float>(-0.0375 - 0.05*step)
		};
		cv::Mat kernel_matrix = cv::Mat(3, 3, CV_32FC1, &matr);
		cv::filter2D(m_imgEdit, dst, 32, kernel_matrix);
		auto sheredMat = std::make_shared<cv::Mat>(dst);
		cv::Mat* m_edit = sheredMat.get();
		m_imgEdit = *m_edit;
	}
	/*}
	catch (std::Exception ex)
	{
		throw;
	}*/
}


int detectEye(cv::Mat face)
{
	//https://github.com/bsdnoobz/opencv-code/blob/master/eye-tracking.cpp

	cv::Mat fMat = face.clone();
	Clarity(5, fMat);

	std::vector<cv::Rect> eyes;
    eye_cascade.detectMultiScale(fMat, eyes, 1.1, 3, cv::CASCADE_DO_CANNY_PRUNING | cv::CASCADE_SCALE_IMAGE, cv::Size(10, 10), cv::Size(fMat.cols / 3, fMat.rows / 3));

	if (eyes.size() > 1)
	{
		for (int i = 0; i < eyes.size(); i++)
		{
			cv::rectangle(face, eyes[i], cv::Scalar(255, 255, 255), 10);
		}
	}

	return eyes.size();
}

bool track_success = false;
bool Detect(cv::Mat &captured_image, int frame_count, char change_coiff_eye_distance, std::string mac, std::string uid, std::string get_status_text)
{
	// Reading the images
	cv::Mat_<float> depth_image;
	cv::Mat_<uchar> grayscale_image;

	if (!captured_image.data)
		return 0;

	if (captured_image.channels() == 3)
	{
		cv::cvtColor(captured_image, grayscale_image, 6);
	}
	else
	{
		grayscale_image = captured_image.clone();
	}



	//equalizeHist(grayscale_image, grayscale_image);
	//Clarity(2, grayscale_image);
	//clahe->apply(grayscale_image, grayscale_image);

	row++;



	//// If the face detector has not been initialised read it in
	if (clnf_model.face_detector_HAAR.empty())
	{
		clnf_model.face_detector_HAAR.load(det_parameters.face_detector_location);
		clnf_model.face_detector_location = det_parameters.face_detector_location;
	}

	cv::Point preference_det(-1, -1);
	/*if (clnf_model.preference_det.x != -1 && clnf_model.preference_det.y != -1)
	{
		preference_det.x = clnf_model.preference_det.x * grayscale_image.cols;
		preference_det.y = clnf_model.preference_det.y * grayscale_image.rows;
		clnf_model.preference_det = cv::Point(-1, -1);
	}*/

	//bool face_detection_success;
	//if (index % 35 == 0)
	//{
	//	face_detection_success = LandmarkDetector::DetectSingleFace(bounding_box, grayscale_image, clnf_model.face_detector_HAAR, preference_det);
	//}

	if (!track_success)
	{
		LandmarkDetector::DetectSingleFace(bounding_box, grayscale_image, clnf_model.face_detector_HAAR, preference_det);
	}

//    ++index;

	if (bounding_box.area() < 50)
		return 0;

	if (0 > bounding_box.x || 0 > bounding_box.width || bounding_box.x + bounding_box.width > grayscale_image.cols
		|| 0 >= bounding_box.y || 0 >= bounding_box.height || bounding_box.y + bounding_box.height >= grayscale_image.rows)
	{
		return false;
	}

	grayscale_image = grayscale_image(bounding_box).clone();

	//cv::resize(grayscale_image, grayscale_image, /*cv::Size(680, 480)*/cv::Size(340, 410), cv::INTER_LANCZOS4);
	//cv::resize(grayscale_image, grayscale_image, /*cv::Size(680, 480)*/cv::Size(350, 350), cv::INTER_LANCZOS4);
	cv::resize(grayscale_image, grayscale_image, /*cv::Size(680, 480)*/cv::Size(640, 480), cv::INTER_LANCZOS4);


	clnf_model.face_template = grayscale_image.clone();



	// The actual facial landmark detection / tracking
	//bool detection_success = 


	cv::Rect rect_face = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, depth_image, clnf_model, det_parameters, track_success);
	//double rect_text = rect_face.height;
	//cv::putText(captured_image, std::to_string(rect_text), cv::Point(rect_face.tl().x - 10, rect_face.tl().y - 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0));
	cv::Mat test_rect = grayscale_image.clone();
	//cv::rectangle(captured_image, rect_face, cv::Scalar(0, 0, 255));


	// Visualising the results
	// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
	double detection_certainty = clnf_model.detection_certainty;

	// Gaze tracking, absolute gaze direction
	cv::Point3f gazeDirection0(0, 0, -1);
	cv::Point3f gazeDirection1(0, 0, -1);




	marGraf = cv::Mat(480, 640, grayscale_image.type(), cv::Scalar(255, 255, 255));

	//marGraf = cv::Scalar(255, 255, 255);
	visualise_tracking(grayscale_image, marGraf, depth_image, clnf_model, det_parameters, frame_count, change_coiff_eye_distance, mac, uid, get_status_text);



	//склейка
	//cv::Mat large((480 + 480), 640, grayscale_image.type());
	cv::Mat large(480, 640 + 640, grayscale_image.type());


	//cv::putText(captured_image, "FPS:" + std::to_string((int)fps_tracker), cv::Point(captured_image.cols - 100, captured_image.rows - 50), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 255));

	//text1
	//моргания в минуту
	//сейчас
	//в среднем здесь
	//в среднем в БД
	/*std::string col1_row1 = "blinking in a minute:";
	std::string col1_row2 = "now";
	std::string col1_row3 = "on average here";
	std::string col1_row4 = "on average in the database";

	cv::putText(marGraf, "-", cv::Point(20, 480 / 3), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 255, 0));
	cv::line(marGraf, cv::Point(0, 480 / 2 - 30), cv::Point(640, 480 / 2 - 30), CV_RGB(0, 0, 0), 1.4);


	cv::putText(marGraf, col1_row1, cv::Point(20, 480 / 2), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0));

	cv::putText(marGraf, col1_row2, cv::Point(20, 480 / 2 + 15), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0));
	cv::putText(marGraf, col1_row3, cv::Point(20, 480 / 2 + 30), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0));
	cv::putText(marGraf, col1_row4, cv::Point(20, 480 / 2 + 45), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0));*/




	//detectEye(test_rect);

	//captured_image = test_rect;
	captured_image = grayscale_image.clone();

	//cv::imshow("grayscale_image", grayscale_image);
    //две картинки (склейка)
  	cv::hconcat(captured_image, marGraf, large);
    captured_image = large.clone();

	return true;
}

bool Reset()
{
    clnf_model.Reset();
    
    return true;
}

bool Clear()
{
    clnf_model.Reset();
    
    return true;
}



