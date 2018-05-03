#include "process.h"
#include <iostream>
#include "cinder/app/App.h"
#include "CinderOpenCV.h"


Process::Process()
{
	
}

void Process::readConfig(json& tree, Config* config)
{
	test_int_ = tree["int"].get<int>();
	config->bind_interface("int type", new Bit::IntVariable(&tree["int"], &test_int_, 0, 10));
	
	config->bind_interface("bool type", new Bit::BoolVariable(&tree["bool"], nullptr));

	config->bind_interface("string type", new Bit::StringVariable(&tree["string"], nullptr));

	config->bind_interface("float type", new Bit::NumericVariable<float>(&tree["float"], nullptr, 0, 1.0, 0.1));

	config->bind_event("trigger", [](std::string key, void* data) {
		ci::app::console() << key << std::endl;
	}, nullptr);
}