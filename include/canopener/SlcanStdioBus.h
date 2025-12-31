#pragma once
#include "Bus.h"
#include <stdio.h>

namespace canopener {
    class SlcanStdioBus: public Bus {
    public:
    	SlcanStdioBus(int readfd, int writefd);
        bool available();
        bool read(cof_t *frame);
        void write(cof_t *frame);
        void loop();
        //uint32_t millis() { return 0; }

    private:
    	FILE *readf,*writef;
        std::queue<cof_t> rxBuf;
    };
}