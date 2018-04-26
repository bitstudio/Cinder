#pragma once

#include "_2RealGStreamerWrapper.h"
//#include "gstreamer_wrapper.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/Url.h"
#include "cinder/DataSource.h"

using namespace _2RealGStreamerWrapper;

namespace cinder {
	namespace gstreamer {

		class Video {

		public:

			Video();
			Video( const std::string path );
			Video( const fs::path &path );
			~Video();

			SurfaceRef getSurface();
			bool    checkPlayable();
			
			int32_t getWidth() const;
			int32_t getHeight() const;
			ivec2   getSize() const;

			float   getAspectRatio() const;
			Area    getBounds() const;
			float   getPixelAspectRatio() const;

			float   getDuration() const;
			float   getFramerate() const;
			int32_t getNumFrames() const;

			bool    hasVisuals() const;
			bool    hasAudio() const;

			bool    checkNewFrame();
			float   getCurrentTime() const;
			
			void    seekToTime( float seconds );
			void    seekToStart();
			void    seekToEnd();

			void    setLoop( bool loop=true, bool palindrome=false );
			void    stepForward();
			void    stepBackward();

			void    setRate( float rate );
			void    setVolume( float volume );
			float   getVolume() const;

			bool    isPlaying() const;
			bool    isDone() const;
			void    play();
			void    stop();

			Video   clone();

		private:
			void    loadFile( std::string path );

			std::shared_ptr<GStreamerWrapper>    gstVideo_;
			unsigned char*        gstData_;

			std::string           filename_;
		};

		class GstreamerExc : public std::exception {
		public:
			 virtual const char* what() const throw() {
				 return "cinder::gstreamer exception";
			 }
		};

		class GstreamerExcPathInvalidExc : public GstreamerExc {
		public:
			GstreamerExcPathInvalidExc( std::string path ) throw()
			{
				message_ = "gstreamer can't open file : \"" + path + "\"";
			}

			const char* what() const throw() {
				return message_.c_str();
			}

		private:
			std::string message_;
		};

	};
};



