#pragma once
#include "Device.h"
#include "cof.h"

namespace canopener {
	//void handleSdoExpeditedRead(Device &dev, Message &msg);
	void handleSdoExpeditedWrite(Device &dev, cof_t *frame);
}