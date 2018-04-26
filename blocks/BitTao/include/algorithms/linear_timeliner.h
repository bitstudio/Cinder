#pragma once

#include "cinder/Vector.h"
#include "cinder/app/App.h"

class LinearTimeliner
{
public:

	LinearTimeliner()
	{
		step_ = 0;
	}

	void set(float v)
	{
		value_ = v;
	}

	void operator=(float v)
	{
		value_ = v;
	}

	void toWithin(float t, float sec)
	{
		step_ = (t - value_)/sec;
		target_ = t;
	}

	void toWithin(float t, float sec, float b)
	{
		step_ = (t - b) / sec;
		target_ = t;
	}

	float get_value()
	{
		if (step_ > 0) 
		{
			return std::min(value_, target_);
		}
		else
		{
			return std::max(value_, target_);
		}
	}

	void update()
	{
		if (step_ > 0) 
		{
			value_ = std::min(value_ + step_*(ci::app::getElapsedSeconds() - stamp_), target_);
		}
		else
		{
			value_ = std::max(value_ + step_*(ci::app::getElapsedSeconds() - stamp_), target_);
		}
		stamp_ = ci::app::getElapsedSeconds();
	}

private:
	double target_;
	float step_;
	double value_;
	float stamp_;
};
