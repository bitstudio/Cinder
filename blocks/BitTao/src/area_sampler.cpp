#include "algorithms/area_sampler.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"

using namespace std;
using namespace ci;

namespace Bit {
	
	void onMouse(int event, int x, int y, int, void* data)
	{
		auto projectible = (Projectible*) data;

		switch (event)
		{
		case cv::MouseEventTypes::EVENT_LBUTTONDOWN:
			projectible->setCorners(cv::Point(x, y));
			break;
		}
	}

	Projectible::Projectible(std::string id = "0", cv::Size size = cv::Size(1.0, 1.0))
	{
		id_ = id;

		size_ = size;
		beacons_.push_back(cv::Point2f(0, 0));
		beacons_.push_back(cv::Point2f(size_.width, 0));
		beacons_.push_back(cv::Point2f(size_.width, size_.height));
		beacons_.push_back(cv::Point2f(0, size_.height));

		bounds_.resize(4);
		readCache();
		calibrate();

		show_ = false;
		index_ = 0;
	}

	void Projectible::readCache()
	{
		std::stringstream ss;
		ss << id_ << ".ptj";

		std::ifstream myfile;
		myfile.open(ss.str());
		if (myfile.is_open())
		{
			myfile >> bounds_[0].x >> bounds_[0].y;
			myfile >> bounds_[1].x >> bounds_[1].y;
			myfile >> bounds_[2].x >> bounds_[2].y;
			myfile >> bounds_[3].x >> bounds_[3].y;
			myfile.close();
		}
		else {
			bounds_[0] = cv::Point2f(0, 0);
			bounds_[1] = cv::Point2f(100, 0);
			bounds_[2] = cv::Point2f(100, 100);
			bounds_[3] = cv::Point2f(0, 100);
		}
	}

	void Projectible::writeCache()
	{
		std::stringstream ss;
		ss << id_ << ".ptj";

		std::ofstream myfile;
		myfile.open(ss.str());
		myfile << bounds_[0].x << " " << bounds_[0].y << std::endl;
		myfile << bounds_[1].x << " " << bounds_[1].y << std::endl;
		myfile << bounds_[2].x << " " << bounds_[2].y << std::endl;
		myfile << bounds_[3].x << " " << bounds_[3].y << std::endl;
		myfile.close();
	}

	void Projectible::toggleShowConsole()
	{
		show_ = !show_;
		if (show_)
		{
			index_ = 0;
			cv::namedWindow(id_);
			cv::setMouseCallback(id_, onMouse, this);
		}
		else
		{
			cv::destroyWindow(id_);
		}
	}
	
}
