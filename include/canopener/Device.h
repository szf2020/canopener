#pragma once
#include <vector>
#include "Entry.h"
#include "Bus.h"

namespace canopener {
	class Device {
	public:
		Device(Bus& b);
		Entry& insert(uint16_t index, uint8_t subindex);
		Entry& insert(uint16_t index);
		Entry& at(uint16_t index, uint8_t subindex);
		Entry& at(uint16_t index);
		Entry *find(uint16_t index, uint8_t subindex);
		void loop();
		//void send(Message m);
		int getNodeId() { return nodeId; }
		void setNodeId(int nodeId_) { nodeId=nodeId_; };
		Bus& getBus() { return bus; };

	private:
		Bus& bus;
		std::vector<Entry*> entries;
		int nodeId;
		uint32_t heartbeatDeadline;
		uint32_t heartbeatInterval;
	};
}
