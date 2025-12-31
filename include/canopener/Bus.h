#pragma once
#include "cof.h"

namespace canopener {
    class Bus {
    public:
        virtual bool available()=0;
        virtual bool read(cof_t *frame)=0;
        virtual void write(cof_t *frame)=0;
        virtual void loop() {};
        //virtual uint32_t millis() {return 0; };//=0;
    };
}