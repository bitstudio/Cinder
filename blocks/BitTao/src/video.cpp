#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include <boost/filesystem.hpp>

#include "cinder/app/App.h"
#include "cinder/ImageIo.h"

#include "media/video.h"
#include "configuration/Config.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace ci;

#define MOV_FILE_EXT "mov"
#define MP4_FILE_EXT "mp4"
#define AVI_FILE_EXT "avi"
#define MP3_FILE_EXT "mp3"
#define WAV_FILE_EXT "wav"

#define DEFAULT_FPS  24

namespace Bit {

	Video::Video()
	{
		hasGraphic_ = false;
		elapsedTime_ = 0;
		videoState_ = STOP_STATE;
		frameCount_ = 0;
		fps_ = DEFAULT_FPS;
	}

	void Video::readConfig(json& tree, Bit::Config* config)
	{
		config_.path = Bit::Config::getAssetPath() + tree["path"].get<std::string>();
		config_.loadOption = tree["loadOption"].get<std::string>();
		config_.loop = tree["playOption"]["loop"].get<bool>();
		config_.pingpong = tree["playOption"]["pingpong"].get<bool>();
	}
	
	void Video::setup()
	{
		// init rate
		rate_ = 1.0f;

		// set load option
		loadOption_ = getVideoLoadOptionString(config_.loadOption);

		// setup mode
		loop_ = config_.loop;
		pingpong_ = config_.pingpong;

		// initialize as true and mark as false later when loading fails
		hasGraphic_ = true;

		try {

			// setup graphics path
			string videoPath = config_.path;
			boost::filesystem::path path(config_.path);
			name_ = path.filename().string();

			string fileExt = videoPath.substr(videoPath.find_last_of(".") + 1);

			switch (loadOption_) {
			case Bit::SURFACE_LOADER:
				if (isVideoExtension(fileExt)) {
#ifndef USE_GSTREAMER 
					qtime::MovieSurface8uRef video = qtime::MovieSurface::create(ci::Url(videoPath));
					video->play();
					video->stop();
					int frameNumber = video->getNumFrames();
					surfaces_ = boost::shared_array<Surface8uRef>( new Surface8uRef[ frameNumber ] );
					for( int i = 0; i < frameNumber; ++i ) {
						video->seekToFrame( i );
						surfaces_[i] = video->getSurface();
					}
					duration_ = video->getDuration();
					frameCount_ = video->getNumFrames();
					fps_ = video->getFramerate();
					size_ = video->getSize();
					video.reset();
#else
					gstVideo_ = ci::gstreamer::Video(videoPath);

					gstVideo_.play();
					gstVideo_.setLoop(false, pingpong_);
					gstVideo_.stop();

					int frameNumber = (int)floor(gstVideo_.getFramerate() * gstVideo_.getDuration());
					float timeStep = gstVideo_.getDuration() / (float)(frameNumber - 1);

					surfaces_ = boost::shared_array<Surface8uRef>(new Surface8uRef[frameNumber]);

					for (int i = 0; i < frameNumber; ++i) {
						float time = timeStep * i;
						gstVideo_.seekToTime(time);	
						std::this_thread::sleep_for(std::chrono::milliseconds(5));
						surfaces_[i] = gstVideo_.getSurface();
					}
					duration_ = gstVideo_.getDuration();
					frameCount_ = frameNumber;
					fps_ = gstVideo_.getFramerate();
					size_ = gstVideo_.getSize();
#endif
				}
				else {
					duration_ = 1.0;
					frameCount_ = 1;
					surfaces_ = boost::shared_array<Surface8uRef>(new Surface8uRef[1]);
					surfaces_[0] = ci::Surface::create(loadImage(videoPath));
					if (surfaces_[0])
						size_ = surfaces_[0]->getSize();
				}
				break;

			case Bit::TEXTURE_LOADER:
				if (isVideoExtension(fileExt)) {
#ifndef USE_GSTREAMER 
					qtime::MovieGlRef video = qtime::MovieGl::create(ci::Url(videoPath));
					video->play();
					video->stop();
					int frameNumber = video->getNumFrames();
					textures_ = boost::shared_array<gl::TextureRef>(new gl::TextureRef[frameNumber]);
					for (int i = 0; i < frameNumber; ++i) {
						video->seekToFrame(i);
						textures_[i] = video->getTexture();
					}
					duration_ = video->getDuration();
					frameCount_ = video->getNumFrames();
					fps_ = video->getFramerate();
					size_ = video->getSize();
					video.reset();
#else
					gstVideo_ = ci::gstreamer::Video(videoPath);

					gstVideo_.play();
					gstVideo_.setLoop(false, pingpong_);
					gstVideo_.stop();

					int frameNumber = (int)floor(gstVideo_.getFramerate() * gstVideo_.getDuration());
					float timeStep = gstVideo_.getDuration() / (float)(frameNumber - 1);

					textures_ = boost::shared_array<gl::TextureRef>(new gl::TextureRef[frameNumber]);

					for (int i = 0; i < frameNumber; ++i) {
						float time = timeStep * i;
						gstVideo_.seekToTime(time);
						std::this_thread::sleep_for(std::chrono::milliseconds(5));
						textures_[i] = gl::Texture2d::create(*gstVideo_.getSurface());
					}
					duration_ = gstVideo_.getDuration();
					frameCount_ = frameNumber;
					fps_ = gstVideo_.getFramerate();
					size_ = gstVideo_.getSize();
#endif
				}
				else {
					duration_ = 1.0;
					frameCount_ = 1;
					textures_ = boost::shared_array<gl::TextureRef>(new gl::TextureRef[1]);
					textures_[0] = gl::Texture2d::create(loadImage(videoPath));
					if (textures_[0])
						size_ = textures_[0]->getSize();
				}
				break;

			case Bit::QTIME_LOADER:
				if (isVideoExtension(fileExt)) {
#ifndef USE_GSTREAMER 
					video_ = qtime::MovieGl::create(ci::Url(videoPath));

					video_->play();
					video_->setLoop(loop_, pingpong_);
					video_->stop();
					duration_ = video_->getDuration();
					frameCount_ = video_->getNumFrames();
					fps_ = video_->getFramerate();
					size_ = video_->getSize();
#else

					hasGraphic_ = false;
#endif
				}
				else {
					duration_ = 1.0;
					frameCount_ = 1;
					textures_ = boost::shared_array<gl::TextureRef>(new gl::TextureRef[1]);
					textures_[0] = gl::Texture2d::create(loadImage(videoPath));
				}

				break;

			case Bit::GSTREAMER_LOADER:
				if (isVideoExtension(fileExt)) {

					// remember last frame
#ifndef USE_GSTREAMER 
					qtime::MovieSurface8uRef video = qtime::MovieSurface::create(ci::Url(videoPath));
					video->play();
					video->stop();
					int frameNumber = video->getNumFrames();
					video->seekToFrame(frameNumber);
					lastSurface_ = video->getSurface();
#endif

					gstVideo_ = ci::gstreamer::Video(videoPath);

					gstVideo_.play();
					gstVideo_.setLoop(loop_, pingpong_);
					gstVideo_.stop();
					gstVideo_.seekToStart();
					duration_ = gstVideo_.getDuration();
					if (gstVideo_.hasVisuals())
						frameCount_ = (int)floor(gstVideo_.getFramerate() * gstVideo_.getDuration());
					fps_ = gstVideo_.getFramerate();
					size_ = gstVideo_.getSize();
				}
				else {
					duration_ = 1.0;
					frameCount_ = 1;
					textures_ = boost::shared_array<gl::TextureRef>(new gl::TextureRef[1]);
					textures_[0] = gl::Texture2d::create(loadImage(videoPath));
					if (textures_[0])
						size_ = textures_[0]->getSize();
				}
				break;
			}
		}
		catch (...) {
			hasGraphic_ = false;
		}
	}

	Video Video::clone()
	{
		Video video;

		video = *this;
		video.cloneVideoData();    

		return video;
	}

	void Video::cloneVideoData() 
	{
		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
		case Bit::QTIME_LOADER :
			break;

		case Bit::GSTREAMER_LOADER :
			gstVideo_ = gstVideo_.clone();
			break;
		}
	}
	
	void Video::setLoop( bool loop )
	{
		loop_ = loop;
	}

	void Video::setPingpong( bool pingpong )
	{
		pingpong_ = pingpong;
	}

	bool Video::isLoop()
	{
		return loop_;
	}

	bool Video::isPingpong()
	{
		return pingpong_;
	}

	float Video::getRate()
	{
		return rate_;
	}

	void Video::setRate(float rate)
	{
		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
		case Bit::TEXTURE_LOADER:
			rate_ = rate;
			break;
		case Bit::QTIME_LOADER:
			if (video_)
			{
#ifndef USE_GSTREAMER 
				rate_ = rate;
				video_->setRate(rate_);
#endif
			}
			break;
		case Bit::GSTREAMER_LOADER:
			// can't call video_.setRate( rate ) here for QTIME_LOADER
			// because setRate has to be set everytime after video_.play() is called
			rate_ = rate;
			gstVideo_.setRate(rate_);
			break;
		}
	}

	float Video::getDuration()
	{
		return duration_;
	}

	string Video::getName()
	{
		return name_;
	}

	float Video::getFps()
	{
		return fps_;
	}

	int Video::getNumberOfFrames()
	{
		return (int)(fps_ * duration_);
	}

	int Video::getWidth()
	{
		return size_.x;
	}

	int Video::getHeight()
	{
		return size_.y;
	}

	ci::ivec2 Video::getSize()
	{
		return size_;
	}

	void Video::play()
	{
		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
		case Bit::TEXTURE_LOADER:
			switch (videoState_) {
			case PLAY_STATE:
				// do nothing with startTime_ while the video is already playing
				if (isDone())
					startTime_ = app::getElapsedSeconds();
				break;

			case PAUSE_STATE:
			case STOP_STATE:
				startTime_ = app::getElapsedSeconds() - elapsedTime_;
				break;
			}
			break;

		case Bit::QTIME_LOADER:
#ifndef USE_GSTREAMER 
			if(video_)
			{
				video_->play();
				video_->setLoop(loop_, pingpong_);
				// rate has to be set every time the clip is played
				video_->setRate(rate_);
			}
#endif
			break;

		case Bit::GSTREAMER_LOADER:
			gstVideo_.play();
			gstVideo_.setLoop(loop_, pingpong_);
			gstVideo_.setRate(rate_);
			break;
		}

		videoState_ = PLAY_STATE;
	}

	void Video::pause()
	{
		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
		case Bit::TEXTURE_LOADER:
			switch (videoState_) {
			case PLAY_STATE:
				elapsedTime_ = app::getElapsedSeconds() - startTime_;
				break;

			case PAUSE_STATE:
			case STOP_STATE:
				// do nothing if already in a pause state;
				break;
			}
			break;

		case Bit::QTIME_LOADER:
#ifndef USE_GSTREAMER 
			if(video_)
				video_->stop();
#endif
			break;

		case Bit::GSTREAMER_LOADER:
			gstVideo_.stop();
			break;
		}

		videoState_ = PAUSE_STATE;
	}
		
	void Video::stop()
	{
		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
			switch (videoState_) {
			case PLAY_STATE:
			case PAUSE_STATE:
			case STOP_STATE:
				elapsedTime_ = 0;
				break;
			}
			break;

		case Bit::QTIME_LOADER :
#ifndef USE_GSTREAMER 
			if (video_) 
			{
				video_->stop();
				video_->seekToStart();
			}
#endif
			break;

		case Bit::GSTREAMER_LOADER :
			gstVideo_.stop();
			gstVideo_.seekToStart();
			break;
		}

		videoState_ = STOP_STATE;
	}

	void Video::seekToStart()
	{
		seekToTime(0);
	}

	void Video::seekToTime(float time)
	{
		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
		case Bit::TEXTURE_LOADER:
			switch (videoState_) {
			case PLAY_STATE:
				elapsedTime_ = 0;
				startTime_ = app::getElapsedSeconds() - time;
				break;

			case PAUSE_STATE:
			case STOP_STATE:
				elapsedTime_ = time;
				break;
			}
			break;

		case Bit::QTIME_LOADER:
#ifndef USE_GSTREAMER 
			if (video_) {
				// multiply rate to time because seekToTime function in cinder
				// doesn't use rate_ to calculate the result
				// but above we use rate_ to get the result
				time *= rate_;
				bool playing = isPlaying();
				video_->play();
				video_->seekToTime(time);
				if (!playing)
					video_->stop();
			}
#endif
			break;

		case Bit::GSTREAMER_LOADER:
			time *= rate_;
			bool playing = isPlaying();
			gstVideo_.play();
			gstVideo_.seekToTime(time);
			if (!playing)
				gstVideo_.stop();
			break;
		}
	}

	void Video::seekToNormalizedTime(float normalizedTime)
	{
		float time = normalizedTime * getDuration();

		seekToTime(time);
	}

	bool Video::hasVisuals()
	{
		return hasGraphic_;
	}

	bool Video::isDone()
	{		
		bool done = false;
		float currentTime = (float) ( app::getElapsedSeconds() - startTime_ );

		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
			
			if( currentTime >= duration_ )
				done = true;
			else
				done = false;

			if( pingpong_ ) {
				float twiceDuration = 2.0f * duration_;
				if( currentTime >= twiceDuration )
					done = true;
				else
					done = false;
			}

			if( loop_ )
				done = false;

			break;

		case Bit::QTIME_LOADER :
#ifndef USE_GSTREAMER 
			if(video_)
				done = video_->isDone();
#endif
			break;

		case Bit::GSTREAMER_LOADER :
			done = gstVideo_.isDone();
			break;
		}

		return done;
	}



	bool Video::isPlaying()
	{		
		bool playing = false;
		float currentTime = (float)(app::getElapsedSeconds() - startTime_);

		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
			if (currentTime <= duration_)
				playing = true;
			else
				playing = false;

			if (pingpong_) {
				float twiceDuration = 2.0f * duration_;
				if (currentTime <= twiceDuration)
					playing = true;
				else
					playing = false;
			}

			if (loop_)
				playing = true;
			break;

		case Bit::QTIME_LOADER :
#ifndef USE_GSTREAMER 
			if(video_)
				playing = video_->isPlaying();
#endif
			break;

		case Bit::GSTREAMER_LOADER :
			playing = gstVideo_.isPlaying();
			break;
		}

		return playing;
	}

	ci::Surface8uRef Video::getSurface()
	{
		Surface8uRef graphics;

		int frame = getCurrentFrame();

		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
			if (surfaces_)
				graphics = surfaces_[frame];
		case Bit::TEXTURE_LOADER:
			if (textures_)
				graphics = Surface::create(textures_[frame]->createSource());
			break;

		case Bit::QTIME_LOADER:
#ifndef USE_GSTREAMER 
			if (video_)
				graphics = Surface::create(video_->getTexture()->createSource());
			else if (textures_)
				graphics = Surface::create(textures_[0]->createSource());
#endif
			break;

		case Bit::GSTREAMER_LOADER:
			if (textures_)
				graphics = Surface::create(textures_[0]->createSource());
			else {
				Surface8uRef surface;

				if (isDone())
					surface = lastSurface_;
				else
					surface = gstVideo_.getSurface();

				if (surface)
					graphics = surface;
			}
			break;
		}

		return graphics;
	}

	ci::Surface8uRef Video::getSurfaceAtTime(float time)
	{
		Surface8uRef graphics;

		int frame = getFrameAtTime(time);

		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
			if (surfaces_)
				graphics = surfaces_[frame];
		case Bit::TEXTURE_LOADER:
			if (textures_)
				graphics = Surface::create(textures_[frame]->createSource());
			break;

		case Bit::QTIME_LOADER:
			if (video_) {
#ifndef USE_GSTREAMER 
				// multiply rate to time because seekToTime function in cinder
				// doesn't use rate_ to calculate the result
				// but above we use rate_ to get the result
				time *= rate_;
				video_->play();
				video_->seekToTime(time);
				video_->stop();
				graphics = Surface::create(video_->getTexture()->createSource());
#endif
			}
			else if (textures_)
				graphics = Surface::create(textures_[0]->createSource());
			break;

		case Bit::GSTREAMER_LOADER:
			if (textures_)
				graphics = Surface::create(textures_[0]->createSource());
			else {
				time *= rate_;
				gstVideo_.play();
				gstVideo_.seekToTime(time);
				// TODO : need to find a way to check if surface of the new frame that get seek to is ready
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				gstVideo_.stop();
				Surface8uRef surface = gstVideo_.getSurface();
				if (surface)
					graphics = surface;
			}
			break;
		}

		return graphics;
	}

	ci::Surface8uRef Video::getSurfaceAtNormalizedTime(float normalizedTime)
	{
		float time = normalizedTime * getDuration();

		return getSurfaceAtTime(time);
	}

	gl::TextureRef Video::getTexture()
	{
		gl::TextureRef graphics;

		if (!hasGraphic_)
			return graphics;

		int frame = getCurrentFrame();

		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
			if (surfaces_)
				graphics = gl::Texture2d::create(*surfaces_[frame]);
		case Bit::TEXTURE_LOADER:
			if (textures_)
				graphics = textures_[frame];
			break;

		case Bit::QTIME_LOADER:
			if (video_)
			{
#ifndef USE_GSTREAMER 
				graphics = video_->getTexture();
#endif
			}
			else if (textures_)
				graphics = textures_[0];
			break;

		case Bit::GSTREAMER_LOADER:
			if (textures_)
				graphics = textures_[0];
			else {
				Surface8uRef surface;
				if (isDone())
					surface = lastSurface_;
				else
					surface = gstVideo_.getSurface();

				if (surface)
				{
					graphics = gl::Texture2d::create(*surface);
				}
			}
			break;
		}

		return graphics;
	}

	gl::TextureRef Video::getTextureAtTime(float time)
	{
		gl::TextureRef graphics;

		int frame = getFrameAtTime(time);

		switch (loadOption_) {
		case Bit::SURFACE_LOADER:
			if (surfaces_)
				graphics = gl::Texture2d::create(*surfaces_[frame]);
		case Bit::TEXTURE_LOADER:
			if (textures_)
				graphics = textures_[frame];
			break;

		case Bit::QTIME_LOADER:
			if (video_) {
#ifndef USE_GSTREAMER 
				// multiply rate to time because seekToTime function in cinder
				// doesn't use rate_ to calculate the result
				// but above we use rate_ to get the result
				time *= rate_;
				video_->play();
				video_->seekToTime(time);
				video_->stop();
				graphics = video_->getTexture();
#endif
			}
			else if (textures_)
				graphics = textures_[0];
			break;

		case Bit::GSTREAMER_LOADER:
			if (textures_)
				graphics = textures_[0];
			else {
				time *= rate_;
				gstVideo_.play();
				gstVideo_.seekToTime(time);
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				gstVideo_.stop();
				Surface8uRef surface = gstVideo_.getSurface();
				graphics = gl::Texture2d::create(*surface);
			}
			break;
		}

		return graphics;
	}

	gl::TextureRef Video::getTextureAtNormalizedTime(float normalizedTime)
	{
		float time = normalizedTime * getDuration();

		return getTextureAtTime(time);
	}
	
	double Video::getStartTime()
	{
		return startTime_;
	}
	
	int Video::getCurrentFrame()
	{
		int frame = 0;

		float elapsedTime = 0;
		
		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
			switch (videoState_) {
			case PLAY_STATE :
				elapsedTime = (float)(app::getElapsedSeconds() - startTime_);
				break;
			case PAUSE_STATE :
			case STOP_STATE :
				elapsedTime = (float)elapsedTime_;
				break;
			}
			break;

		case Bit::QTIME_LOADER :
#ifndef USE_GSTREAMER 
			if( video_ ) 
			    elapsedTime = video_->getCurrentTime();
#endif
			break;

		case Bit::GSTREAMER_LOADER :
			elapsedTime = gstVideo_.getCurrentTime();
			break;
		}

		frame = getFrameAtTime( elapsedTime );

		return frame;
	}

	int Video::getFrameAtTime( float elapsedTime )
	{
		int frame = 0;

		int elapsedFrame;

		float duration = 0;
		if( rate_ != 0 )
			duration = duration_ / rate_;

		switch( loadOption_ ) {
		case Bit::SURFACE_LOADER :
		case Bit::TEXTURE_LOADER :
			elapsedFrame = (int) floor( ( elapsedTime / duration ) * frameCount_ );

			frame = elapsedFrame;

			if( loop_ ) 
				frame = elapsedFrame % frameCount_;

			if( pingpong_ ) {
				int videoLoop = (int) floor( elapsedTime / duration );

				frame = elapsedFrame % frameCount_;
				if( videoLoop % 2 ) 
					frame = frameCount_ - frame - 1;

				if (!loop_ && elapsedTime > 2 * duration)
					frame = 0;
			}

			if( frame < 0 )
				frame = 0;
			else if( frame > frameCount_ - 1 )
				frame = frameCount_ - 1;

			break;

		case Bit::QTIME_LOADER :
			if( video_ ) {
				frame = (int) floor( ( elapsedTime / duration ) * frameCount_ );
			}
			break;

		case Bit::GSTREAMER_LOADER :
			if( duration != 0 )
				frame = (int) floor( ( elapsedTime / duration ) * frameCount_ );
			break;
		}

		return frame;
	}
	
	
	VideoLoadOption Video::getVideoLoadOptionString( std::string option )
	{
		VideoLoadOption loadOption = Bit::SURFACE_LOADER;

		if( !option.compare( LOAD_SURFACE_OPTION ) ) {
			loadOption = Bit::SURFACE_LOADER;
		}
		else if( !option.compare( LOAD_TEXTURE_OPTION ) ) {
			loadOption = Bit::TEXTURE_LOADER;
		}
		else if( !option.compare( LOAD_QTIME_OPTION ) ) {
			loadOption = Bit::QTIME_LOADER;
		}
		else if( !option.compare( LOAD_GSTREAMER_OPTION ) ) {
			loadOption = Bit::GSTREAMER_LOADER;
		}

		return loadOption;
	}

	bool Video::isVideoExtension( std::string extension )
	{
		if (!extension.compare(MOV_FILE_EXT) || !extension.compare(MP4_FILE_EXT) || !extension.compare(AVI_FILE_EXT) || !extension.compare(MP3_FILE_EXT) || !extension.compare(WAV_FILE_EXT) )
			return true;
		else
			return false;
	}
	
}
