#include "canopener.h"

#ifdef ESP32
#include <driver/twai.h>

using namespace canopener;

EspBus::EspBus(int txPin_, int rxPin_) {
	txPin=txPin_;
	rxPin=rxPin_;
	initialized=false;
	havePeeked=false;
}

void EspBus::populatePeeked() {
    //Serial.printf("rcv, havePeeked=%d\n",havePeeked);

	if (havePeeked)
		return;

    esp_err_t result;
    twai_message_t message;
    result=twai_receive(&message,0); //pdMS_TO_TICKS(0));
    if (result==ESP_ERR_TIMEOUT)
    	return;

    //Serial.printf("rcv!!!\n");

    if (result!=ESP_OK) {
        Serial.printf("Recv error: %d\n",result);
        return;
    }

    lastBusTime=millis();
    peeked.id=message.identifier;
    peeked.len=message.data_length_code;
    for (int i=0; i<8; i++)
        peeked.data[i]=message.data[i];

    havePeeked=true;
}

bool EspBus::available() {
	populatePeeked();
	return havePeeked;
}

bool EspBus::read(cof_t *frame) {
	populatePeeked();

	if (!havePeeked)
		return false;

	*frame=peeked;
	havePeeked=false;
	return true;
}

void EspBus::write(cof_t *frame) {
    twai_message_t message;

    message.identifier = frame->id;  // CAN ID
    message.extd = 0;            // Standard frame (11-bit ID)
    message.rtr = 0;             // Data frame (not remote)
    message.data_length_code=frame->len;
    for (int i=0; i<8; i++)
        message.data[i]=frame->data[i];

    esp_err_t result;
    result=twai_transmit(&message,0); // nonblocking, else: pdMS_TO_TICKS(1000));
    if (result==ESP_OK) {
        lastBusTime=millis();
        //Serial.printf("message sent...\n");
        sendErrorCount=0;
    }

    else {
	    sendErrorCount++;
        Serial.printf("Send error: %d\n",result);
    }
}

void EspBus::loop() {
	if (!initialized) {
		resetBus();
		initialized=true;
	}

    twai_status_info_t status;
    twai_get_status_info(&status);

    //bool errorPressure = status.tx_error_counter > 5;
    bool stalled = millis() - lastBusTime > 500;
    if (sendErrorCount>1 && stalled) // WHAAAT??? THIS SHOuLD NOT BE 0, it should be 5 or so... test and fix...
        resetBus();
}

void EspBus::resetBus() {
    Serial.println("********* Resetting ESP CAN bus...");
    lastBusTime=millis();
    sendErrorCount=0;

    twai_stop();
    delay(20);
    //vTaskDelay(pdMS_TO_TICKS(20));

    twai_driver_uninstall();
    delay(20);
    //vTaskDelay(pdMS_TO_TICKS(20));

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin, (gpio_num_t)rxPin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();  // 500kbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("TWAI driver installed");
    } else {
        Serial.println("Failed to install TWAI driver");
        return;
    }

    if (twai_start() == ESP_OK) {
        Serial.println("TWAI driver started");
    } else {
        Serial.println("Failed to start TWAI driver");
        return;
    }
}

#endif