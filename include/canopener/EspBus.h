#pragma once
#include "Bus.h"
#include <stdio.h>

#ifdef ESP32

namespace canopener {
    class EspBus: public Bus {
    public:
    	EspBus(int txPin_, int rxPin_);
        bool available();
        bool read(cof_t *frame);
        void write(cof_t *frame);
        void loop();
        //uint32_t millis() { return millis(); }

    private:
        int txPin, rxPin;
        uint32_t lastBusTime;
        bool initialized;
        void resetBus();
        void populatePeeked();
        cof_t peeked;
        bool havePeeked;
        int sendErrorCount;
    };
}

#endif