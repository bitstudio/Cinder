#pragma once

#include "CinderOpenCV.h"
#include "cinder/app/App.h"

class Area_Sampler
{
public:

	Area_Sampler(ci::ivec2 size, std::vector<ci::vec2> &bounds)
	{
		size_ = cv::Size(size.x, size.y);
		beacons_.push_back(cv::Point2f(0, 0));
		beacons_.push_back(cv::Point2f(size_.width, 0));
		beacons_.push_back(cv::Point2f(size_.width, size_.height));
		beacons_.push_back(cv::Point2f(0, size_.height));

		bounds_.resize(4);
		bounds_ = bounds;
		calibrate(bounds_);
	}

	void calibrate(std::vector<ci::vec2> corners, bool inverted = false)
	{
		if (!inverted) 
		{
			std::vector<cv::Point2f> corners_;
			for (int i = 0; i < 4; ++i)
				corners_.push_back(ci::toOcv(corners[i]));
			perspectiveTransform_ = getPerspectiveTransform(corners_, beacons_);
		}
		else 
		{
			ci::vec2 window_size = ci::app::getWindowSize();

			std::vector<cv::Point2f> corners_;
			for (int i = 0; i < 4; ++i)
				corners_.push_back(cv::Point2f(corners[i].x*window_size.x, corners[i].y*window_size.y));

			std::vector<cv::Point2f> temp_;
			for (int i = 0; i < 4; ++i)
				temp_.push_back(cv::Point2f(beacons_[i].x*window_size.x, beacons_[i].y*window_size.y));

			perspectiveTransform_ = getPerspectiveTransform(temp_, corners_);
		}
		bounds_ = corners;

		gl_perspectiveTransform_[0][0] = perspectiveTransform_.ptr<double>(0)[0];
		gl_perspectiveTransform_[1][0] = perspectiveTransform_.ptr<double>(0)[1];
		gl_perspectiveTransform_[3][0] = perspectiveTransform_.ptr<double>(0)[2];

		gl_perspectiveTransform_[0][1] = perspectiveTransform_.ptr<double>(1)[0];
		gl_perspectiveTransform_[1][1] = perspectiveTransform_.ptr<double>(1)[1];
		gl_perspectiveTransform_[3][1] = perspectiveTransform_.ptr<double>(1)[2];

		gl_perspectiveTransform_[0][3] = perspectiveTransform_.ptr<double>(2)[0];
		gl_perspectiveTransform_[1][3] = perspectiveTransform_.ptr<double>(2)[1];
		gl_perspectiveTransform_[3][3] = perspectiveTransform_.ptr<double>(2)[2];
	}

	void fromInverted(Area_Sampler inverted)
	{
		cv::invert(inverted.perspectiveTransform_, perspectiveTransform_);

		gl_perspectiveTransform_[0][0] = perspectiveTransform_.ptr<double>(0)[0];
		gl_perspectiveTransform_[1][0] = perspectiveTransform_.ptr<double>(0)[1];
		gl_perspectiveTransform_[3][0] = perspectiveTransform_.ptr<double>(0)[2];

		gl_perspectiveTransform_[0][1] = perspectiveTransform_.ptr<double>(1)[0];
		gl_perspectiveTransform_[1][1] = perspectiveTransform_.ptr<double>(1)[1];
		gl_perspectiveTransform_[3][1] = perspectiveTransform_.ptr<double>(1)[2];

		gl_perspectiveTransform_[0][3] = perspectiveTransform_.ptr<double>(2)[0];
		gl_perspectiveTransform_[1][3] = perspectiveTransform_.ptr<double>(2)[1];
		gl_perspectiveTransform_[3][3] = perspectiveTransform_.ptr<double>(2)[2];
	}

	cv::Mat map(cv::Mat src)
	{
		cv::Mat dst;
		cv::warpPerspective(src, dst, perspectiveTransform_, size_);
		return dst;
	}

	std::vector<ci::vec2>& getBounds()
	{
		return bounds_;
	}

	ci::dmat4& getTransformationMatrix()
	{
		return gl_perspectiveTransform_;
	}


private:
	cv::Size size_;
	std::vector<cv::Point2f> beacons_;
	cv::Mat perspectiveTransform_;
	ci::dmat4 gl_perspectiveTransform_;

	std::vector<ci::vec2> bounds_;
};