#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "process/process.h"
#include "http.h"
#include "CinderOpenCV.h"
#include "media/video.h"
#include "configuration/Config.h"
#include "media/effect.h"

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

	Process process_obj_;

	Effector layer_;
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

	config_.readConfigurableConfig(vid, "sample");
	vid.setup();
	vid.play();

	auto list = config_.getTreePtr()["array"];
	for (auto iter = list.begin(); iter != list.end(); ++iter)
	{
		switch (iter->type())
		{
			case json::value_t::boolean:
				ci::app::console() << iter->get<bool>() << std::endl;
				break;
			case json::value_t::string:
				ci::app::console() << iter->get<std::string>() << std::endl;
				break;
			case json::value_t::number_float:
				ci::app::console() << iter->get<float>() << std::endl;
				break;
			case json::value_t::number_integer:
				ci::app::console() << iter->get<int>() << std::endl;
				break;
		}
	}
	config_.readConfigurableConfig(process_obj_, "process");

	auto group = std::make_shared<AggregatedEffect>(SRT_transform(ci::vec2(), 0, 0.5));

	group->push_back(std::make_shared<Winkle>(SRT_transform(ci::vec2(), 0, 0.5), vid));
	group->push_back(std::make_shared<Winkle>(SRT_transform(ci::vec2(0, -0.5), 0, 0.5), vid));

	layer_.addEffect(group);
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

	auto tex = vid.getTexture();
	if (tex)
	{
		//cv::imwrite("test.png", ci::toOcv(*surface));
		gl::draw(ci::gl::Texture2d::create(ci::fromOcv(display_)), ci::app::getWindowBounds());
		gl::draw(tex);


		layer_.draw(ci::app::getWindowBounds());
	}
		
	gl::begin( GL_LINE_STRIP );
	for( const vec2 &point : mPoints ) {
		gl::vertex( point );
	}
	gl::end();
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( BasicApp, RendererGl, prepareSettings )
