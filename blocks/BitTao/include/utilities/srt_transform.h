#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"


class SRT_transform
{
public:

	class ScopeTransform
	{
	public:
		ScopeTransform(SRT_transform& aspt)
		{
			ci::gl::pushMatrices();
			ci::gl::translate(aspt.translation_);
			ci::gl::rotate(aspt.orientation_);
			ci::gl::scale(aspt.scale_, aspt.scale_);
		}

		~ScopeTransform()
		{
			ci::gl::popMatrices();
		}

	};

	SRT_transform()
	{
		translation_ = ci::dvec2(0.0, 0.0);
		orientation_ = 0.0;
		scale_ = 1.0;
	}

	SRT_transform(ci::dvec2 translation, double orientation, double scale): translation_(translation), orientation_(orientation), scale_(scale)
	{
	}

	SRT_transform& operator+(const SRT_transform& transform)
	{
		translation_ = translation_ + transform.translation_;
		orientation_ = orientation_ + transform.orientation_;
		scale_ = scale_ + transform.scale_;
		return *this;
	}

	SRT_transform& operator+(const ci::dvec2 translate)
	{
		translation_ = translation_ + translate;
		return *this;
	}

	SRT_transform& rotate(const double rotate)
	{
		orientation_ = orientation_ + rotate;
		return *this;
	}

	SRT_transform& scale(const double scale)
	{
		scale_ = scale_*scale;
		return *this;
	}


private:
	ci::dvec2 translation_;
	double orientation_;
	double scale_;
};