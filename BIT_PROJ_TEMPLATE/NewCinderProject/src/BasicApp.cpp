#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "process/process.h"
#include "http.h"
#include "CinderOpenCV.h"
#include "media/video.h"
#include "configuration/Config.h"

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class BasicApp : public App {
  public:
	// Cinder will call 'mouseDrag' when the user moves the mouse while holding one of its buttons.
	// See also: mouseMove, mouseDown, mouseUp and mouseWheel.
	void mouseDrag( MouseEvent event ) override;

	// Cinder will call 'keyDown' when the user presses a key on the keyboard.
	// See also: keyUp.
	void keyDown( KeyEvent event ) override;

	void setup() override;

	void update() override;

	// Cinder will call 'draw' each time the contents of the window need to be redrawn.
	void draw() override;

  private:
	// This will maintain a list of points which we will draw line segments between
	std::vector<vec2> mPoints;
	Bit::Video vid;
	cv::VideoCapture* capture_;
	cv::Mat display_;
};

Bit::Config config_;

void prepareSettings( BasicApp::Settings* settings )
{
	settings->setMultiTouchEnabled(false);
	config_.readConfig();

	auto display = config_.getDisplayConfig();

	settings->setWindowPos(display.windowPos);
	settings->setWindowSize(display.windowSize);
	settings->setAlwaysOnTop(display.alwaysOnTop);
	settings->setBorderless(display.borderless);
}

void BasicApp::setup()
{
	Bit::ConfigEndScopeSetup setup(&config_);
	ci::app::console() << "HEY!" << std::endl;

	capture_ = new cv::VideoCapture(0);

	vid.readConfig(config_.getTreePtr()["sample"], &config_);
	vid.setup();
	vid.play();
}

void BasicApp::mouseDrag( MouseEvent event )
{
	// Store the current mouse position in the list.
	mPoints.push_back( event.getPos() );
}

void BasicApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		// Toggle full screen when the user presses the 'f' key.
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getCode() == KeyEvent::KEY_SPACE ) {
		// Clear the list of points when the user presses the space bar.
		mPoints.clear();
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		// Exit full screen, or quit the application, when the user presses the ESC key.
		if( isFullScreen() )
			setFullScreen( false );
		else
			quit();
	}
}

void BasicApp::update()
{
	Process nothing_obj_;
	capture_->read(display_);
	if (!display_.empty())
		cv::imshow("test2", display_);
	cv::waitKey(10);
}

void BasicApp::draw()
{
	// Clear the contents of the window. This call will clear
	// both the color and depth buffers. 
	gl::clear( Color::gray( 0.1f ) );

	// Set the current draw color to orange by setting values for
	// red, green and blue directly. Values range from 0 to 1.
	// See also: gl::ScopedColor
	gl::color( 1.0f, 1.0f, 1.0f );

	auto surface = vid.getSurface();
	if (surface)
	{
		//cv::imwrite("test.png", ci::toOcv(*surface));
		gl::draw(ci::gl::Texture2d::create(ci::fromOcv(display_)), ci::app::getWindowBounds());
		gl::draw(ci::gl::Texture2d::create(*surface));

		auto tex = vid.getTexture();
		if (tex) gl::draw(tex, ci::Area(0, 0, 100, 100));
	}
		
	gl::begin( GL_LINE_STRIP );
	for( const vec2 &point : mPoints ) {
		gl::vertex( point );
	}
	gl::end();
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( BasicApp, RendererGl, prepareSettings )
