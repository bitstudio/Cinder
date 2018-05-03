#pragma once
#include "configuration/Config.h"

using namespace Bit;

class Process : public Configurable {
public:
	Process();
	void readConfig(json& tree, Config* config);

private:
	int test_int_;

};