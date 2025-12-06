#include "canopener.h"
#include <stdexcept>

using namespace canopener;

Device::Device(Bus& bus)
		:bus(bus) {
	nodeId=0;
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
	while (bus.available()) {
		cof_t frame;
		bus.read(&frame);

		//Message m=Message(bus.read());
		//handleSdoExpeditedRead(*this,m);
		handleSdoExpeditedWrite(*this,&frame);
	}
}

/*void Device::send(Message m) {
	bus.write(m.f);
}*/