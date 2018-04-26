#include "cinder/app/App.h"

#include "CinderGstVideo.h"

namespace cinder {

	namespace gstreamer {

		Video::Video()
		{
			gstData_ = NULL;
			filename_ = "";
			setLoop( false );
		}

		Video::Video( const std::string path )
		{
			loadFile( path );

			if( gstVideo_ ) {
				gstVideo_->stop();
				gstVideo_->setLoopMode( NO_LOOP );
			}
		}

		Video::Video( const fs::path &path )
		{
			Video( path.string() );
		}

		Video::~Video()
		{
		}

		Video Video::clone()
		{
			Video newVideo;

			if( gstVideo_ ) {
				newVideo = Video( filename_ );
				// set rate
				float rate = gstVideo_->getSpeed();
				newVideo.setRate( rate );
				// set loop mode
				LoopMode mode = gstVideo_->getLoopMode();
				switch( mode ) {
				case NO_LOOP :
					newVideo.setLoop( false, false );
					break;
				case LOOP :
					newVideo.setLoop( true, false );
					break;
				case BIDIRECTIONAL_LOOP :
					newVideo.setLoop( true, true );
					break;
				}
			}

			return newVideo;
		}

		void Video::loadFile( std::string path )
		{
			gstData_ = NULL;
			filename_ = path;

			std::string filePath;
			if( path.find( "http://" ) != std::string::npos || 
				path.find( "https://" ) != std::string::npos ||
				path.find( "file:/" ) != std::string::npos )
				filePath = path;
			else
				filePath = "file:/" + path;

			gstVideo_ = std::shared_ptr<GStreamerWrapper>( new GStreamerWrapper() );

			if( gstVideo_->open( filePath, true, false ) ) {

			}
			else {
				gstVideo_.reset();

				throw GstreamerExcPathInvalidExc( path );
			}
		}

		SurfaceRef Video::getSurface()
		{
			if( gstVideo_ ) {
#ifndef THREADED_MESSAGE_HANDLER
				gstVideo_->update();
#endif
				gstData_ = gstVideo_->getVideo();
			}
			
			if( gstData_ ) {
				return Surface::create( gstData_,
					gstVideo_->getWidth(),
					gstVideo_->getHeight(),
					gstVideo_->getWidth() * 4, 
					SurfaceChannelOrder::RGBA );
			}
			else {
				return Surface::create(0, 0, true);
			}
		}

		bool Video::checkPlayable()
		{
			bool playable = false;

			if( gstVideo_ ) {
				if( gstVideo_->hasVideo() || gstVideo_->hasAudio() )
					playable = true;
			}

			return playable;
		}
			
		int32_t Video::getWidth() const
		{
			int32_t width = 0;
			
			if( gstVideo_ && gstVideo_->hasVideo() )
				width = gstVideo_->getWidth();
			
			return width;
		}

		int32_t Video::getHeight() const
		{
			int32_t height = 0;

			if( gstVideo_ && gstVideo_->hasVideo() )
				height = gstVideo_->getHeight();

			return height;
		}

		ivec2 Video::getSize() const
		{
			ivec2 size = ivec2();

			if( gstVideo_ && gstVideo_->hasVideo() ) {
				size = ivec2( gstVideo_->getWidth() , gstVideo_->getHeight() );
			}

			return size;
		}

		float Video::getAspectRatio() const
		{
			float ratio = 1.0f;

			if( gstVideo_ && gstVideo_->hasVideo() ) {
				ratio = (float) gstVideo_->getWidth() / (float) gstVideo_->getHeight();
			}

			return ratio;
		}

		Area Video::getBounds() const
		{
			Area area( 0, 0, 0, 0 );

			if( gstVideo_ && gstVideo_->hasVideo() ) {
				area.set( 0, 0, gstVideo_->getWidth(), gstVideo_->getHeight() );
			}

			return area;
		}

		float Video::getPixelAspectRatio() const
		{
			float par = 1.0f;

			if( gstVideo_ && gstVideo_->hasVideo() ) {
				// find aspect ratio
				// to implement
			}

			return par;
		}

		float Video::getDuration() const
		{
			float duration = 0.0f;

			if( gstVideo_ ) {
				duration = (float) (gstVideo_->getDurationInMs() * 0.001f);
			}

			return duration;
		}

		float Video::getFramerate() const
		{
			float framerate = 24.0f;

			if( gstVideo_ ) {
				framerate = gstVideo_->getFps();
			}

			return framerate;
		}

		int32_t Video::getNumFrames() const
		{
			int32_t frames = 0;

			if( gstVideo_ ) {
				frames = (int32_t) gstVideo_->getNumberOfFrames();
			}

			return frames;
		}

		bool Video::hasVisuals() const
		{
			bool hasVideo = false;

			if( gstVideo_ ) {
				hasVideo = gstVideo_->hasVideo();
			}

			return hasVideo;
		}

		bool Video::hasAudio() const
		{
			bool hasAudio = false;

			if( gstVideo_ ) {
				hasAudio = gstVideo_->hasAudio();
			}

			return hasAudio;
		}

		bool Video::checkNewFrame()
		{
			bool hasNewFrame = false;

			if( gstVideo_ && gstVideo_->hasVideo() && gstVideo_->isNewVideoFrame() ) {
				hasNewFrame = true;
			}

			return hasNewFrame;
		}

		float Video::getCurrentTime() const
		{
			float currentTime = 0.0f;

			if( gstVideo_ ) {
				currentTime = (float)( gstVideo_->getCurrentTimeInMs() * 0.001f );
			}

			return currentTime;
		}
			
		void Video::seekToTime( float seconds )
		{
			if( gstVideo_ ) {
				double milliSeconds = (double)( seconds * 1000 );
				gstVideo_->setTimePositionInMs( milliSeconds );
			}
		}

		void Video::seekToStart()
		{
			if( gstVideo_ ) {
				gstVideo_->setPosition( 0.0f );
			}
		}

		void Video::seekToEnd()
		{
			if( gstVideo_ ) {
				gstVideo_->setPosition( 1.0f );
			}
		}

		void Video::setLoop( bool loop, bool palindrome )
		{
			if( gstVideo_ ) {
				if( loop ) {
					if( palindrome )
						gstVideo_->setLoopMode( BIDIRECTIONAL_LOOP );
					else
						gstVideo_->setLoopMode( LOOP );
				}
				else {
					gstVideo_->setLoopMode( NO_LOOP );
				}
			}
		}

		void Video::stepForward()
		{
			if( gstVideo_ && gstVideo_->hasVideo() ) {
				gint64 durationNs = gstVideo_->getDurationInNs();
				gint64 timePerFrameNs = (gint64) (1e9 / gstVideo_->getFps());
				gint64 currentTimeNs = gstVideo_->getCurrentTimeInNs();

				LoopMode loopMode = gstVideo_->getLoopMode();

				switch( loopMode ) {
				case NO_LOOP :
					currentTimeNs += timePerFrameNs;
					if( currentTimeNs > durationNs )
						currentTimeNs = durationNs;
					break;

				case LOOP :
					currentTimeNs += timePerFrameNs;
					if( currentTimeNs > durationNs )
						currentTimeNs = 0;
					break;

				case BIDIRECTIONAL_LOOP :
					PlayDirection direction = gstVideo_->getDirection();

					if( direction == FORWARD ) {
						currentTimeNs += timePerFrameNs;
						if( currentTimeNs > durationNs ) {
							gstVideo_->setDirection( BACKWARD );
							currentTimeNs = durationNs - timePerFrameNs;
							if( currentTimeNs < 0 )
								currentTimeNs = 0;
						}
					}
					else if( direction == BACKWARD ) {
						currentTimeNs -= timePerFrameNs;
						if( currentTimeNs < 0 ) {
							gstVideo_->setDirection( FORWARD );
							currentTimeNs = timePerFrameNs;
							if( currentTimeNs > durationNs )
								currentTimeNs = durationNs;
						}
					}
					break;
				}

				gstVideo_->setTimePositionInNs( currentTimeNs );
			}
		}

		void Video::stepBackward()
		{
			if( gstVideo_ && gstVideo_->hasVideo() ) {
				gint64 durationNs = gstVideo_->getDurationInNs();
				gint64 timePerFrameNs = (gint64)(1e9 / gstVideo_->getFps());
				gint64 currentTimeNs = gstVideo_->getCurrentTimeInNs();

				LoopMode loopMode = gstVideo_->getLoopMode();

				switch( loopMode ) {
				case NO_LOOP :
					currentTimeNs -= timePerFrameNs;
					if( currentTimeNs < 0 )
						currentTimeNs = 0;
					break;

				case LOOP :
					currentTimeNs -= timePerFrameNs;
					if( currentTimeNs < 0 )
						currentTimeNs = durationNs;
					break;

				case BIDIRECTIONAL_LOOP :
					PlayDirection direction = gstVideo_->getDirection();

					if( direction == FORWARD ) {
						currentTimeNs -= timePerFrameNs;
						if( currentTimeNs < 0 ) {
							gstVideo_->setDirection( BACKWARD );
							currentTimeNs = timePerFrameNs;
							if( currentTimeNs > durationNs )
								currentTimeNs = durationNs;
						}
					}
					else if( direction == BACKWARD ) {
						currentTimeNs += timePerFrameNs;
						if( currentTimeNs > durationNs ) {
							gstVideo_->setDirection( FORWARD );
							currentTimeNs = durationNs - timePerFrameNs;
							if( currentTimeNs < 0 )
								currentTimeNs = 0;
						}
					}
					break;
				}
				
				gstVideo_->setTimePositionInNs( currentTimeNs );
			}
		}

		void Video::setRate( float rate )
		{
			if( gstVideo_ ) {
				if( rate >= 0 ) {
					gstVideo_->setDirection( FORWARD );
					gstVideo_->setSpeed( rate );
				} else {
					gstVideo_->setDirection( BACKWARD );
					gstVideo_->setSpeed( -rate );
				}
			}
		}

		void Video::setVolume( float volume )
		{
			if( gstVideo_ ) {
				gstVideo_->setVolume( volume );
			}
		}

		float Video::getVolume() const
		{
			float volume = 1.0f;

			if( gstVideo_ ) {
				gstVideo_->getCurrentVolume();
			}

			return volume;
		}

		bool Video::isPlaying() const
		{
			bool playing = false;

			if( gstVideo_ ) {
				PlayState state = gstVideo_->getState();

				if( state == PLAYING )
					playing = true;
			}

			return playing;
		}

		bool Video::isDone() const
		{
			bool done = false;

			if( gstVideo_ ) {
				PlayState state = gstVideo_->getState();

				if( (state == STOPPED) )
					done = true;
			}

			return done;
		}

		void Video::play()
		{
			if( gstVideo_ )
				gstVideo_->play();
		}

		void Video::stop()
		{
			if( gstVideo_ )
				gstVideo_->pause();
		}

	}

}