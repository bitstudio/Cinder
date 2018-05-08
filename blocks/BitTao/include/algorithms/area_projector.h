#ifndef _BIT_PERSPECTIVE_PROJECTION_H_
#define _BIT_PERSPECTIVE_PROJECTION_H_


#include "opencv2/opencv.hpp"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"
#include <fstream>

namespace Bit {

	class Projector
	{
	public:

		Projector(std::string id, cv::Size s);

		void calibrate()
		{
			perspectiveTransform_ = getPerspectiveTransform(beacons_, bounds_);
			cv::invert(perspectiveTransform_, invPerspectiveTransform_);

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

		ci::dmat4& getTransformationMatrix()
		{
			return gl_perspectiveTransform_;
		}

		cv::Point2f& operator[](int index)
		{
			index = index % 4;
			return bounds_[index];
		}

		void setCorners(cv::Point point, int index = -1)
		{
			setCorners_only(point, index);
			calibrate();
			writeCache();
		}

		void setCorners_only(cv::Point point, int index = -1)
		{
			if (index >= 0) index_ = index;
			bounds_[index_] = point;
			index_ = (index_ + 1) % 4;
		}

		cv::Mat getInverse()
		{
			return invPerspectiveTransform_;
		}

		cv::Mat getTransform()
		{
			return perspectiveTransform_;
		}

		void readCache();
		void writeCache();

	private:
		std::string id_;

		cv::Size size_;
		std::vector<cv::Point2f> beacons_;

		int index_;
		std::vector<cv::Point2f> bounds_;

		cv::Mat invPerspectiveTransform_;
		cv::Mat perspectiveTransform_;
		ci::dmat4 gl_perspectiveTransform_;
	};


}

#endif
