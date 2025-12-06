#pragma once
#include "Bus.h"
#include "cof.h"
#include <queue>
#include <string>

namespace canopener {
	class MockBus: public Bus {
	public:
		bool available() { return rxBuf.size(); };
		void write(cof_t *frame) { txBuf.push(*frame); };
		bool read(cof_t *frame) { 
			cof_t c=rxBuf.front();
			cof_cpy(frame,&c);
			rxBuf.pop();
			return true;
		}

		void rxBufPushSlcan(std::string s) {
			cof_t frame;
			cof_from_slcan(&frame,s.c_str());
			rxBuf.push(frame);
		}

		std::string txBufPopSlcan() {
			cof_t frame;
			cof_t c=txBuf.front();
			cof_cpy(&frame,&c);
			txBuf.pop();
			char s[16];
			cof_to_slcan(&frame,s);
			return std::string(s);
		}

		std::queue<cof_t> rxBuf;
		std::queue<cof_t> txBuf;
	};
}
