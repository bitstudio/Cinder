#include "process.h"
#include <iostream>
#include "cinder/app/App.h"
#include "CinderOpenCV.h"
#include "media/effect.h"


void draw_fit_area_preserve_asp(ci::gl::TextureRef tex)
{
	if (tex)
	{
		ci::gl::pushMatrices();
		float scale = 1.0 / tex->getHeight();
		ci::dvec2 t = ci::dvec2(-0.5*(tex->getWidth()*scale), -0.5);
		ci::gl::translate(t);
		ci::gl::scale(scale, scale);
		ci::gl::draw(tex);
		ci::gl::popMatrices();
	}
}

void Effector::setup()
{
}


void Effector::addEffect(Effect* effect)
{
	this->addEffect(std::shared_ptr<Effect>(effect));
}

void Effector::addEffect(std::shared_ptr<Effect> effect)
{
	std::lock_guard<std::mutex> lock(mutex_);

	//creation
	this->playing_.push_back(std::shared_ptr<Effect>(effect));
	this->playing_.back()->play();
}

void Effector::draw(ci::Area area)
{
	std::lock_guard<std::mutex> lock(mutex_);

	//termination
	for (auto iter = this->playing_.begin(); iter != this->playing_.end();)
	{
		if ((*iter)->isDone())
		{
			(*iter)->stop();
			iter = this->playing_.erase(iter);
		}
		else ++iter;
	}

	ci::gl::pushMatrices();
	ci::gl::translate(area.getCenter());
	ci::gl::scale(area.getHeight(), area.getHeight());
	for (auto iter = this->playing_.begin(); iter != this->playing_.end(); ++iter)
	{
		(*iter)->draw();
	}
	ci::gl::popMatrices();
}
