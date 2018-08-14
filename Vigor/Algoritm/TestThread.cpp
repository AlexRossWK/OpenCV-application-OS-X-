
#include "TestThread.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "FaceDetect.h"

#include <iostream>
#include <vector>
#include <time.h>
#include "string.h"


//mac
//#include <winsock2.h>
//#include <iphlpapi.h>
//#include <stdio.h>
//#include <stdlib.h>

#include <thread>
#include <mutex>

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
//#include <windows.h>
#include <iostream>

#include <curl/curl.h>

//#include <iostream>
//#include <io.h>
//#include <fcntl.h>



// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


int frame_count = 1;


void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent = 0)
{

	CV_Assert(clipHistPercent >= 0);
	CV_Assert((src.type() == CV_8UC1) || (src.type() == CV_8UC3) || (src.type() == CV_8UC4));

	int histSize = 256;
	float alpha, beta;
	double minGray = 0, maxGray = 0;

	cv::Mat gray;
	if (src.type() == CV_8UC1) gray = src;
	else if (src.type() == CV_8UC3) cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else if (src.type() == CV_8UC4) cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
	if (clipHistPercent == 0)
	{
		cv::minMaxLoc(gray, &minGray, &maxGray);
	}
	else
	{
		cv::Mat hist;

		float range[] = { 0, 256 };
		const float* histRange = { range };
		bool uniform = true;
		bool accumulate = false;
		calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

		std::vector<float> accumulator(histSize);
		accumulator[0] = hist.at<float>(0);
		for (int i = 1; i < histSize; i++)
		{
			accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
		}

		float max = accumulator.back();
		clipHistPercent *= (max / 100.0); //make percent as absolute
		clipHistPercent /= 2.0; // left and right wings
								// locate left cut
		minGray = 0;
		while (accumulator[minGray] < clipHistPercent)
			minGray++;

		maxGray = histSize - 1;
		while (accumulator[maxGray] >= (max - clipHistPercent))
			maxGray--;
	}

	float inputRange = maxGray - minGray;

	alpha = (histSize - 1) / inputRange;
	beta = -minGray * alpha;


	src.convertTo(dst, -1, alpha, beta);

	if (dst.type() == CV_8UC4)
	{
		int from_to[] = { 3, 3 };
		cv::mixChannels(&src, 4, &dst, 1, from_to, 1);
	}
	return;
}

//void getdMacAddresses(std::vector<std::string> &vMacAddresses)
//{
//	vMacAddresses.clear();
//	IP_ADAPTER_INFO AdapterInfo[32];       // Allocate information for up to 32 NICs
//	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
//	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
//		AdapterInfo,                 // [out] buffer to receive data
//		&dwBufLen);                  // [in] size of receive data buffer
//
//									 //No network card? Other error?
//	if (dwStatus != ERROR_SUCCESS)
//		return;
//
//	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
//	char szBuffer[512];
//	while (pAdapterInfo)
//	{
//		if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
//		{
//			//"%.2x-%.2x-%.2x-%.2x-%.2x-%.2x"
//			sprintf_s(szBuffer, sizeof(szBuffer), "%.2x%.2x%.2x%.2x%.2x%.2x"
//				, pAdapterInfo->Address[0]
//				, pAdapterInfo->Address[1]
//				, pAdapterInfo->Address[2]
//				, pAdapterInfo->Address[3]
//				, pAdapterInfo->Address[4]
//				, pAdapterInfo->Address[5]
//			);
//			vMacAddresses.push_back(szBuffer);
//		}
//		pAdapterInfo = pAdapterInfo->Next;
//
//	}
//}


int index_file_name_save = 0;
std::vector<std::vector<cv::Mat>> recordFrames;

std::string UID_ID_TEXT_MESSAGE;
/*std::string GetUID()
{
	UUID uuid;
	UuidCreate(&uuid);
	char *str_uid;
	UuidToStringA(&uuid, (RPC_CSTR*)&str_uid);
	std::cout << str_uid << std::endl;

	std::string string_text = std::string(str_uid);

	return string_text;

}*/

//long GetFileSizeArhive(std::string filename)
//{
//    struct stat stat_buf;
//    int rc = stat(filename.c_str(), &stat_buf);
//    return rc == 0 ? stat_buf.st_size : -1;
//}

//void SendJSONVideoFile(std::string name_file, long size_file, const char* uid)
//{
//	CURLcode ret;
//	CURL *hnd;
//	curl_mime *mime1;
//	curl_mimepart *part1;
//	struct curl_slist *slist1;
//
//	mime1 = NULL;
//	slist1 = NULL;
//	slist1 = curl_slist_append(slist1, "X-Device-ID: 309c231d555e");
//
//	hnd = curl_easy_init();
//	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, size_file);
//	curl_easy_setopt(hnd, CURLOPT_URL, "http://eye-server.woodenshark.com/api/v1/save_video");
//	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
//	mime1 = curl_mime_init(hnd);
//	part1 = curl_mime_addpart(mime1);
//	curl_mime_data(part1, /*"8a665889-b7a2-d9cc-3d40-810d37265fe1"*/uid, CURL_ZERO_TERMINATED);
//	curl_mime_name(part1, "log_record");
//	part1 = curl_mime_addpart(mime1);
//	curl_mime_filedata(part1, /*"arhive/out_camera0.avi"*/name_file.c_str());
//	curl_mime_name(part1, "video_file");
//	curl_easy_setopt(hnd, CURLOPT_MIMEPOST, mime1);
//	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
//	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
//	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
//	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
//	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
//
//	CURLcode res;
//	ret = curl_easy_perform(hnd);
//	if (CURLE_OK == res) {
//		char *ct;
//		res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &ct);
//		if ((CURLE_OK == res) && ct)
//			printf("\nWe received Content-Type: %s\n", ct);
//	}
//
//	curl_easy_cleanup(hnd);
//	hnd = NULL;
//	curl_mime_free(mime1);
//	mime1 = NULL;
//	curl_slist_free_all(slist1);
//	slist1 = NULL;
//}
//
//void RecordFile()
//{
//	while (1)
//	{
//
//		if (recordFrames.size() == 0)
//		{
//			Sleep(1);
//			// recordFramesObj.unlock();
//			continue;
//		}
//
//
//		std::vector<cv::Mat> records(recordFrames[0]);
//		recordFrames.erase(recordFrames.begin() + 0);
//		std::string name_file = "arhive/out_camera" + std::to_string(index_file_name_save) + ".avi";
//		cv::VideoWriter video(name_file, CV_FOURCC('M', 'P', '4', '2'), 15, cv::Size(640, 480), true);
//
//
//		for (int index = 0; index < records.size(); index++)
//		{
//			cv::Mat dt = records[index].clone();
//			video.write(dt);
//		}
//
//		video.release();
//		records.clear();
//
//		Sleep(10);
//
//		//if (LandmarkDetector::UID_preview == UID_ID_TEXT_MESSAGE)
//		//{
//		SendJSONVideoFile(name_file, GetFileSizeArhive(name_file), UID_ID_TEXT_MESSAGE.c_str());
//		//удаляем файл
//		std::remove(name_file.c_str());
//
//		++index_file_name_save;
//
//		std::ofstream myfile;
//		myfile.open("uid.txt", std::ios_base::app);
//		myfile << "send: " + UID_ID_TEXT_MESSAGE + "\n";
//
//		//UID_ID_TEXT_MESSAGE = GetUID();
//
//		myfile << "new: " + UID_ID_TEXT_MESSAGE + "\n";
//		myfile.close();
//
//
//		//}
//		Sleep(100);
//	}
//}

std::vector<cv::Mat> frames;


bool stop = false;

void SleepDetectMetod(std::string id_device, std::string path_start)
{
	std::string mac = id_device;
	
	//if (false)
	//{//get_mac

	//	std::vector<std::string> vect;
	//	getdMacAddresses(vect);

	//	if (vect.size() > 0)
	//		mac = vect[0];
	//	//std::cout << vect[0] << std::endl;
	//	//std::cout << getdMacAddresses() << std::endl;
	//}

	cv::VideoCapture video_capture(0);
	//cv::VideoCapture video_capture("video/1.mp4");

    Inits(path_start);

	//cv::namedWindow("tracking_result", cv::WINDOW_AUTOSIZE);
	
	//cv::namedWindow("grayscale_image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("grayscale_image", grayscale_image);

	if (!video_capture.isOpened())
		return;


	cv::Mat image;
	video_capture >> image;
	cv::resize(image, image, cv::Size(640, 480));

	InitMat(image);


	char k;
	bool skip = false;

	//int index_write_video = 0;
	cv::VideoWriter video;
	//video.open("arhive/out_camera" + std::to_string(index_write_video)+ ".avi", CV_FOURCC('M', 'P', '4', '2'), 15, cv::Size(640, 480), true);

	//cv::VideoWriter video("arhive/out.avi", CV_FOURCC('M', 'P', '4', 'V'), 23, cv::Size(1280, 720), true);
	//cv::VideoWriter video("arhive/out.avi", CV_FOURCC('V', 'M', 'P', '4'), 11, cv::Size(2688, 1512), true);


	std::chrono::high_resolution_clock::time_point time_start = std::chrono::high_resolution_clock::now();

	//std::thread thread_record = std::thread(RecordFile);
	//thread_record.detach();

	UID_ID_TEXT_MESSAGE = id_device;

	while (true)
	{

		//анализируем каждый кадр
		if (stop)
		{
			std::cout << "exit_buttom" << std::endl;
			//нажали SleepDetect::Stop
			break;
		}

		video_capture >> image;

		//cv::imshow("image", image);

        
        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////

		BrightnessAndContrastAuto(image, image, 0.5);
        
        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////

		//std::cout<<image.rows <<":"<<image.cols<<std::endl;
		cv::resize(image, image, cv::Size(640, 480));

        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
		if (false)
		{
			//video.write(image);

			frames.push_back(image);
			std::chrono::high_resolution_clock::time_point time_end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> time_send_report = time_end - time_start;

            ////
            if (stop)
            {
                std::cout << "exit_buttom" << std::endl;
                //нажали SleepDetect::Stop
                break;
            }
            ////
            
			if (time_send_report.count() > 30000)
			{
				//video.release();

                ////
                if (stop)
                {
                    std::cout << "exit_buttom" << std::endl;
                    //нажали SleepDetect::Stop
                    break;
                }
                ////
                
				recordFrames.push_back(frames);
				frames.clear();

				//video.open("arhive/out_camera" + std::to_string(index_write_video) + ".avi", CV_FOURCC('M', 'P', '4', '2'), 15, cv::Size(1280, 720), true);
				time_start = std::chrono::high_resolution_clock::now();
			}
		}
        
        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
		cv::Mat targetImage(image.cols, image.rows, CV_8UC3);
		//cv::cvtColor(image, targetImage, cv::COLOR_BGRA2BGR);
        
        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
		targetImage = image.clone();

        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////

		if (targetImage.empty())
		{
			std::cout << "targetImage empty" << std::endl;
			break;
		}
		else
		{

            ////
            if (stop)
            {
                std::cout << "exit_buttom" << std::endl;
                //нажали SleepDetect::Stop
                break;
            }
            ////
            
			if (k == 27)
			{
				skip = !skip;
			}

			if (!skip)
			{
				Detect(targetImage, frame_count, k, mac, UID_ID_TEXT_MESSAGE);
			}
			else
			{
				cv::putText(targetImage, "PAUSE", cv::Point(targetImage.cols - 120, targetImage.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1);

			}

			frame_count = frame_count + 1;
		}
		//cv::cvtColor(targetImage, image, cv::COLOR_BGRA2RGB);
        
        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
		image = targetImage.clone();

		/*	cv::putText(image, "Stamina: N/A", cv::Point(25, image.rows - 100), CV_FONT_HERSHEY_SIMPLEX, 1, CV_RGB(192, 192, 192), 2);
		cv::putText(image, "ID: " + mac, cv::Point(25, image.rows - 70), CV_FONT_HERSHEY_SIMPLEX, 1,  CV_RGB(192, 192, 192), 2);

		if (!skip)
		{
		cv::putText(image, "HotKey ESC: OFF ", cv::Point(25, image.rows - 40), CV_FONT_HERSHEY_SIMPLEX, 1, CV_RGB(192, 192, 192) , 2);
		}
		else
		cv::putText(image, "HotKey ESC: ON ", cv::Point(25, image.rows - 40), CV_FONT_HERSHEY_SIMPLEX, 1, CV_RGB(192, 192, 192), 2);*/

        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
        cv::putText(image, "Stamina: N/A", cv::Point(25, image.rows - 50), cv::FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 2);
        cv::putText(image, "ID: " + mac, cv::Point(25, image.rows - 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 2);

        ////
        if (stop)
        {
            std::cout << "exit_buttom" << std::endl;
            //нажали SleepDetect::Stop
            break;
        }
        ////
        
		if (!skip)
		{
			cv::putText(image, "HotKey ESC: OFF ", cv::Point(25, image.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 2);
		}
		else
			cv::putText(image, "HotKey ESC: ON ", cv::Point(25, image.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 2);


		// Write out the framerate on the image before displaying it
		/*char fpsC[255];
		std::sprintf(fpsC, "%d", (int)fps_tracker);
		std::string fpsSt("FPS:");
		fpsSt += fpsC;
		cv::putText(image, "FPS:" + std::to_string((int)fps_tracker), cv::Point(25, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0));*/



		//cv::imshow("tracking_result", image);
		//k = cv::waitKey(1);
	}

	std::cout << "stop thread" << std::endl;



}

void SleepDetect::Start(std::string id_device, std::string path_start)
{
	
	//в отдельном потоке запускаем анализ состояния водителя.
	//std::thread thread_camera_detect = std::thread(SleepDetectMetod);
	//thread_camera_detect.detach();
    stop = false;
	SleepDetectMetod(id_device, path_start);

}

void StopStatus()
{
	while (1)
	{
//        std::this_thread::sleep_for(std::chrono::milliseconds(0));

		//выход
		stop = true;

		break;
	}
}
void SleepDetect::Stop()
{
	
	std::thread thread_camera_detect = std::thread(StopStatus);
	thread_camera_detect.detach();

}
