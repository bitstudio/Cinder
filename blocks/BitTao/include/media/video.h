#pragma once
#ifndef _BIT_VIDEO_
#define _BIT_VIDEO_

#include <string>

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTimeGlImplAvf.h"
#include "cinder/Surface.h"
#include "cinder/Json.h"
#include "CinderGstVideo.h"
#include "configuration/Config.h"

#include "boost/shared_array.hpp"

#define USE_GSTREAMER

namespace Bit {
	#define LOAD_SURFACE_OPTION "surface"
	#define LOAD_TEXTURE_OPTION "texture"
	#define LOAD_QTIME_OPTION "quicktime"
	#define LOAD_GSTREAMER_OPTION "gstreamer"

	enum VideoLoadOption {
		SURFACE_LOADER,
		TEXTURE_LOADER,
		QTIME_LOADER,
		GSTREAMER_LOADER
	};

	class Video : public Bit::Configurable {
	public:
		typedef struct Config {
			std::string path;
			std::string loadOption;
			bool   loop;
			bool   pingpong;
		};

		Video();

		void readConfig(json& tree, Bit::Config* config);
		void setup();
		
		Video clone();
		
		void setLoop( bool loop = true );
		void setPingpong( bool pingpong = true );

		bool isLoop();
		bool isPingpong();

		float getRate();
		void setRate(float rate = 1);

		float  getDuration();
		std::string getName();
		float  getFps();
		int    getNumberOfFrames();

		int  getWidth();
		int  getHeight();
		ci::ivec2  getSize();

		void play();
		void pause();
		void stop();
		
		void seekToStart();
		void seekToTime( float time );
		void seekToNormalizedTime( float normalizedTime );

		bool hasVisuals();
		bool isDone();
		bool isPlaying();

		ci::Surface8uRef getSurface();
		ci::Surface8uRef getSurfaceAtTime( float time );
		ci::Surface8uRef getSurfaceAtNormalizedTime( float normalizedTime );

		ci::gl::TextureRef getTexture();
		ci::gl::TextureRef getTextureAtTime( float time );
		ci::gl::TextureRef getTextureAtNormalizedTime( float normalizedTime );

		static VideoLoadOption getVideoLoadOptionString( std::string option = LOAD_SURFACE_OPTION );
		
	protected:
		enum VideoState { PLAY_STATE, PAUSE_STATE, STOP_STATE };

		double getStartTime();
		void  cloneVideoData();
		static bool isVideoExtension( std::string extension );
		
		int getCurrentFrame();
		int getFrameAtTime( float elapsedTime );

		VideoLoadOption loadOption_;

		// graphics
		bool hasGraphic_;
		ci::qtime::MovieGlRef  video_;
		ci::gstreamer::Video gstVideo_;
		boost::shared_array<ci::gl::TextureRef> textures_;
		boost::shared_array<ci::Surface8uRef>  surfaces_;
		
		VideoState videoState_;

		Config config_;

		// video information
		float duration_;
		int   frameCount_;
		double startTime_;
		double elapsedTime_;
		std::string name_;
		float  fps_;
		ci::ivec2  size_;
		
		bool  loop_;
		bool  pingpong_;

		float rate_;

		ci::Surface8uRef lastSurface_;
	};
};

#endif