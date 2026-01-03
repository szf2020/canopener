#include <Arduino.h>
#include "canopener.h"
using namespace canopener;

SerialBus serialBus(Serial);
EspBus espBus(5,4); // tx, rx def= 5,4
BridgeBus bus(serialBus,espBus);
Device dev(bus);

void setup() {
    Serial.begin(115200);
    dev.setNodeId(DEVID);
    dev.insert(0x2000,0).setType(Entry::BOOL).set<bool>(false);

    pinMode(8,OUTPUT);
}

void loop() {
    dev.loop();
    digitalWrite(8,dev.at(0x2000,0).get<bool>());
}
