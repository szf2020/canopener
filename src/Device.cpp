#include "canopener.h"
#include <stdexcept>

using namespace canopener;

Device::Device(Bus& bus)
		:bus(bus) {
	nodeId=0;
	heartbeatInterval=1000;
	heartbeatDeadline=bus.millis()+heartbeatInterval;
}

Entry& Device::insert(uint16_t index, uint8_t subindex) {
	Entry* e=new Entry(index,subindex);
	entries.push_back(e);
	return *e;
}

Entry& Device::insert(uint16_t index) {
	return insert(index,0);
}

Entry& Device::at(uint16_t index, uint8_t subindex) {
	Entry *e=find(index,subindex);
	if (!e)
		throw std::out_of_range("Entry not found");

	return *e;
}

Entry& Device::at(uint16_t index) {
	return at(index,0);
}

Entry* Device::find(uint16_t index, uint8_t subindex) {
	for (Entry* e: entries)
		if (e->index==index && subindex==e->subindex)
			return e;

	return NULL;
}

void Device::loop() {
	bus.loop();

	while (bus.available()) {
		cof_t frame;
		bus.read(&frame);
		handleSdoExpeditedRead(*this,&frame);
		handleSdoExpeditedWrite(*this,&frame);
	}

	if (bus.millis()>=heartbeatDeadline) {
        cof_t heartbeat;
        cof_set(&heartbeat,COF_FUNC,COF_FUNC_HEARTBEAT); 
        cof_set(&heartbeat,COF_NODE_ID,getNodeId()); 
        cof_set(&heartbeat,COF_HEARTBEAT_STATE,COF_HB_OPERATIONAL);
        getBus().write(&heartbeat);

        //char s[256];
        //cof_to_slcan(&heartbeat,s);
		//Serial.printf("heartbeat...\n");

		heartbeatDeadline=bus.millis()+heartbeatInterval;
	}
}

/*void Device::send(Message m) {
	bus.write(m.f);
}*/