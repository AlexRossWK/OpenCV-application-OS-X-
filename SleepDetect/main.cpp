
#include "TestThread.h"

std::string status = "";
void GetStatusSleepDetect(SleepDetect &sleep_d)
{
	while (1)
	{
		//просто цикл, ждем событий
		std::cout << "================================ getstatus ================================" << std::endl;
		Sleep(1000);

		std::cout << sleep_d.GetStatus() << std::endl;

	}
}
int main(int argc, char * argv[])
{
	SleepDetect sleep_d;

	//стартуем в отдельном потоке
	//sleep_d.Start();
	sleep_d.Stop();

	std::thread thread_camera_detect = std::thread(GetStatusSleepDetect, sleep_d);
	thread_camera_detect.detach();

	sleep_d.Start();

	
}