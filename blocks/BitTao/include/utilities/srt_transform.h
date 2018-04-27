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

private:
	ci::dvec2 translation_;
	double orientation_;
	double scale_;
};