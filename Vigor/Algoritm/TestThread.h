#pragma once

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
#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <mutex>

#include <iostream>
#include <vector>
#include <time.h>
#include "string.h"


class SleepDetect
{
private:
	//bool stop = false;
	
public:
	void Start(std::string id_device);
	
	void Stop();
};
