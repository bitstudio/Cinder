#ifndef _BIT_PERSPECTIVE_PROJECTION_H_
#define _BIT_PERSPECTIVE_PROJECTION_H_


#include "opencv2/opencv.hpp"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"
#include <fstream>

namespace Bit {

	class Projectible
	{
	public:

		Projectible(std::string id, cv::Size s);

		void calibrate()
		{
			perspectiveTransform_ = getPerspectiveTransform(bounds_, beacons_);

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

		cv::Mat update(cv::Mat src)
		{
			if (show_) 
			{
				cv::Mat canvas;
				src.copyTo(canvas);
				auto color = cv::Scalar(255, 255, 0);
				cv::circle(canvas, bounds_[index_], 4, color);
				cv::line(canvas, bounds_[0], bounds_[1], color);
				cv::line(canvas, bounds_[1], bounds_[2], color);
				cv::line(canvas, bounds_[2], bounds_[3], color);
				cv::line(canvas, bounds_[3], bounds_[0], color);

				cv::imshow(id_, canvas);
				cv::waitKey(1);
			}


			cv::Mat dst;
			cv::warpPerspective(src, dst, perspectiveTransform_, size_);
			return dst;
		}

		void setCorners(cv::Point point, int index = -1)
		{
			if (index >= 0) index_ = index;
			bounds_[index_] = point;
			index_ = (index_ + 1) % 4;
			calibrate();
			writeCache();
		}

		void toggleShowConsole();

		void readCache();
		void writeCache();

	private:
		std::string id_;

		cv::Size size_;
		std::vector<cv::Point2f> beacons_;

		int index_;
		std::vector<cv::Point2f> bounds_;

		bool show_;

		cv::Mat perspectiveTransform_;
		ci::dmat4 gl_perspectiveTransform_;
	};


}

#endif
