#include "process.h"
#include <iostream>
#include "cinder/app/App.h"
#include "CinderOpenCV.h"


Process::Process()
{
	cv::Mat m = cv::Mat(300, 300, CV_8UC1);
	
	cv::imshow("test", m);
	cv::waitKey(1);
}