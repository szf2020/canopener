#pragma once
#include "Bus.h"
#include <stdio.h>

#ifdef ARDUINO
#include <Arduino.h>

namespace canopener {
    class SerialBus: public Bus {
    public:
    	SerialBus(Stream& stream_);
        bool available();
        bool read(cof_t *frame);
        void write(cof_t *frame);
        void loop();
        uint32_t millis() { return ::millis(); }

    private:
        Stream &stream;
        std::queue<cof_t> rxBuf;
    };
}

#endif