#pragma once
#include "media/video.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "utilities/srt_transform.h"

void draw_fit_area_preserve_asp(ci::gl::TextureRef tex);

class Effect {
public:
	Effect()
	{

	}

	Effect(SRT_transform transform): transform_(transform)
	{

	}

	virtual void play() = 0;
	virtual void stop() = 0;
	virtual bool isDone() = 0;
	virtual void draw() = 0;

	SRT_transform Transform() const
	{
		return transform_;
	}

	void setTransform(const SRT_transform& transform)
	{
		transform_ = transform;
	}

	void operator=(const SRT_transform& transform)
	{
		transform_ = transform;
	}

	virtual void terminate(){}

protected:
	SRT_transform transform_;
};

class UniqueVisualEffect : public Effect {
public:
	UniqueVisualEffect()
	{
		loop_ = false;
	}

	UniqueVisualEffect(SRT_transform transform, Bit::Video* video)
	{
		video_ = video;
		transform_ = transform;
		loop_ = false;
	}

	void play()
	{
		video_->stop();
		video_->play();
	}

	void stop()
	{
		video_->stop();
	}

	bool isDone()
	{
		return !loop_ && video_->isDone();
	}

	void setLoop(bool flag)
	{
		loop_ = flag;
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);
		if (loop_ && video_->isDone())
		{
			video_->stop();
			video_->play();
		}
		draw_fit_area_preserve_asp(video_->getTexture());
	}

private:
	Bit::Video* video_;
	bool loop_;
};

class Winkle : public Effect {
public:
	Winkle()
	{
	}

	Winkle(SRT_transform transform, Bit::Video &video)
	{
		video_ = video;
		transform_ = transform;
		if (!video_.hasVisuals()) video_.setup();
	}

	void play()
	{
		video_.play();
	}

	void stop()
	{
		video_.stop();
	}

	bool isDone()
	{
		return video_.isDone();
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);
		draw_fit_area_preserve_asp(video_.getTexture());
	}

private:
	Bit::Video video_;
};

class FadeEffect : public Effect {
public:
	FadeEffect()
	{
		terminating_ = false;
		phase_ = -1;
	}

	FadeEffect(SRT_transform transform, Bit::Video &videoStart, Bit::Video &videoLoop, Bit::Video &videoEnd)
	{
		videoStart_ = videoStart;
		videoLoop_ = videoLoop;
		videoEnd_ = videoEnd;
		if (!videoStart_.hasVisuals()) videoStart_.setup();
		if (!videoLoop_.hasVisuals()) videoLoop_.setup();
		if (!videoEnd_.hasVisuals()) videoEnd_.setup();
		transform_ = transform;

		terminating_ = false;
		phase_ = 0;
	}

	void reset()
	{
		terminating_ = false;
		phase_ = 0;
		videoStart_.stop();
		videoLoop_.stop();
		videoEnd_.stop();
	}

	void play()
	{
		switch (phase_)
		{
		case 0:
			videoStart_.play();
			break;
		case 1:
			videoLoop_.play();
			break;
		case 2:
			videoEnd_.play();
			break;
		}
	}

	void stop()
	{
		switch (phase_) 
		{
		case 0:
			videoStart_.stop();
			break;
		case 1:
			videoLoop_.stop();
			break;
		case 2:
			videoEnd_.stop();
			break;
		}
	}

	bool isDone()
	{
		return phase_ == 2 && videoEnd_.isDone();
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);

		if (phase_ == 0) 
		{
			if (!videoStart_.isDone())
			{
				draw_fit_area_preserve_asp(videoStart_.getTexture());
				return;
			}
			else 
			{
				phase_ = 1;
				videoStart_.stop();
				videoLoop_.play();
			}
		}

		if (phase_ == 1)
		{
			if (terminating_ && videoLoop_.isDone())
			{
				phase_ = 2;
				videoLoop_.stop();
				videoEnd_.play();
			}
			else 
			{
				if (videoLoop_.isDone()) {
					videoLoop_.stop();
					videoLoop_.play();
				}
				draw_fit_area_preserve_asp(videoLoop_.getTexture());
				return;
			}
		}

		draw_fit_area_preserve_asp(videoEnd_.getTexture());
		return;
	}

	void terminate()
	{
		terminating_ = true;
	}

private:
	Bit::Video videoStart_;
	Bit::Video videoLoop_;
	Bit::Video videoEnd_;

	bool terminating_;
	int phase_;
};

class ChainEffect : public Effect {

public:
	ChainEffect()
	{
		playing_ = false;
	}

	ChainEffect(SRT_transform transform)
	{
		transform_ = transform;
		playing_ = false;
	}

	void push_back(Effect* effect)
	{
		this->push_back(std::shared_ptr<Effect>(effect));
	}

	void push_back(std::shared_ptr<Effect> effect)
	{
		chain_.push_back(effect);
		if(chain_.size() == 1 && playing_) chain_.front()->play();
	}
	
	void push(Effect* effect, int index) {
		this->push(std::shared_ptr<Effect>(effect), index);
	}

	void push(std::shared_ptr<Effect> effect, int index) {
		if (index < 0) index = index + chain_.size();
		auto iter = chain_.begin();
		for (int i = 0; i < index; ++i)
			++iter;
		chain_.insert(iter, effect);
		if (chain_.size() == 1 && playing_) chain_.front()->play();
	}

	void play()
	{
		if(chain_.size() > 0 ) chain_.front()->play();
		playing_ = true;
	}

	void stop()
	{
		if (chain_.size() > 0) chain_.front()->stop();
		playing_ = false;
	}

	bool isDone()
	{
		return chain_.size() == 0;
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);

		if (chain_.size() > 0)
		{
			if (chain_.front()->isDone())
			{
				this->chain_.erase(this->chain_.begin());
				play();
				if (chain_.size() > 0) chain_.front()->draw();
			}
			else 
			{
				chain_.front()->draw();
			}
		}
	}

	int size()
	{
		return chain_.size();
	}

	void clear()
	{
		if (chain_.size() > 0) chain_.front()->stop();
		chain_.clear();
	}

	void terminate()
	{
		if (chain_.size() > 0) chain_.front()->terminate();
	}

private:
	std::list<std::shared_ptr<Effect>> chain_;
	bool playing_;

};

class AggregatedEffect : public Effect {

public:
	AggregatedEffect()
	{
		playing_ = false;
	}

	AggregatedEffect(SRT_transform transform)
	{
		transform_ = transform;
		playing_ = false;
	}

	void push_back(Effect* effect)
	{
		this->push_back(std::shared_ptr<Effect>(effect));
	}

	void push_back(std::shared_ptr<Effect> effect)
	{
		collection_.push_back(effect);
		if(playing_) effect->play();
	}

	void play()
	{
		playing_ = true;
		for (auto iter = collection_.begin(); iter != collection_.end(); ++iter)
			(*iter)->play();
	}

	void stop()
	{
		playing_ = false;
		for (auto iter = collection_.begin(); iter != collection_.end(); ++iter)
			(*iter)->stop();
	}

	bool isDone()
	{
		return collection_.size() == 0;
	}

	bool empty()
	{
		return collection_.size() == 0;
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);
		for (auto iter = collection_.begin(); iter != collection_.end();)
		{
			if ((*iter)->isDone())
			{
				iter = collection_.erase(iter);
			}
			else 
			{
				(*iter)->draw();
				++iter;
			}
		}
	}

	void terminate()
	{
		for (auto iter = collection_.begin(); iter != collection_.end(); ++iter)
			(*iter)->terminate();
	}

private:
	std::list<std::shared_ptr<Effect>> collection_;
	bool playing_;
};

class SmoothFadeEffect : public Effect {

public:
	SmoothFadeEffect()
	{
		current_ = nullptr;
		new_ = nullptr;
		done_ = false;
		playing_ = false;
	}

	SmoothFadeEffect(SRT_transform transform)
	{
		transform_ = transform;
		current_ = nullptr;
		new_ = nullptr;
		done_ = false;
		playing_ = false;
	}

	void push_back(Effect* effect)
	{
		this->push_back(std::shared_ptr<Effect>(effect));
	}

	void push_back(std::shared_ptr<Effect> effect)
	{
		if (current_ == nullptr) 
		{
			current_ = effect;
			if (playing_) current_->play();
		}
		else 
		{
			new_ = effect;
			current_->terminate();
		}

	}

	void play()
	{
		playing_ = true;
		if(current_ != nullptr) current_->play();
	}

	void stop()
	{
		playing_ = false;
		if (current_ != nullptr) current_->stop();
	}

	bool isDone()
	{
		return done_ && new_ == nullptr && current_->isDone();
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);
		if (current_ != nullptr)
		{
			if (current_->isDone())
			{
				current_ = new_;
				new_ = nullptr;
				if (current_ != nullptr) 
				{
					current_->play();
					current_->draw();
				}
			}
			else 
			{
				current_->draw();
			}
		}
	}

	void terminate()
	{
		if (current_ != nullptr) current_->terminate();
	}

	void mark_for_remove()
	{
		done_ = true;
	}

	bool empty() const
	{
		return current_ == nullptr;
	}

private:
	std::shared_ptr<Effect> current_;
	std::shared_ptr<Effect> new_;

	bool done_;
	bool playing_;
};

class SoundOnly : public Effect {
public:
	SoundOnly()
	{
	}

	SoundOnly(Bit::Video &video)
	{
		video_ = video;
		if (!video_.hasVisuals()) video_.setup();
	}

	void play()
	{
		video_.play();
	}

	void stop()
	{
		video_.stop();
	}

	bool isDone()
	{
		return video_.isDone();
	}

	void draw()
	{
	}

private:
	Bit::Video video_;
};

class SharedEffect : public Effect {
public:
	SharedEffect()
	{
	}

	SharedEffect(SRT_transform transform, Effect* tmpl): SharedEffect(transform, std::shared_ptr<Effect>(tmpl))
	{
	}

	SharedEffect(SRT_transform transform, std::shared_ptr<Effect> tmpl)
	{
		effect_ = tmpl;
		transform_ = transform;
	}

	void play()
	{
	}

	void stop()
	{
	}

	bool isDone()
	{
		return effect_->isDone();
	}

	void draw()
	{
		SRT_transform::ScopeTransform s(transform_);
		effect_->draw();
	}

	void terminate()
	{
	}

private:
	std::shared_ptr<Effect> effect_;
};

class NotShowEffect : public Effect {
public:
	NotShowEffect()
	{
	}

	NotShowEffect(Effect* tmpl):NotShowEffect(std::shared_ptr<Effect>(tmpl))
	{
	}

	NotShowEffect(std::shared_ptr<Effect> tmpl)
	{
		effect_ = tmpl;
	}

	void play()
	{
		effect_->play();
	}

	void stop()
	{
		effect_->stop();
	}

	bool isDone()
	{
		return effect_->isDone();
	}

	void draw()
	{
	}

	void terminate()
	{
		effect_->terminate();
	}

private:
	std::shared_ptr<Effect> effect_;
};

class Effector{

public:
	Effector() {};
	~Effector() {};

	void setup();
	void addEffect(Effect* effect);
	void addEffect(std::shared_ptr<Effect> effect);
	void draw(ci::Area area);
	bool empty() const
	{
		return playing_.size() == 0;
	}

protected:

	std::list<std::shared_ptr<Effect>> playing_;
};
