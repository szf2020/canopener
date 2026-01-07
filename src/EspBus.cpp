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

    //lastBusTime=millis();
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
        //Serial.printf("Send error: %d\n",result);
    }
}

void EspBus::loop() {
	if (!initialized) {
		resetBus();
		initialized=true;
	}

    uint32_t alerts;
    esp_err_t res = twai_read_alerts(&alerts, 0); // non-blocking

    if (res == ESP_OK) {
        if (alerts & TWAI_ALERT_TX_FAILED) {
            sendErrorCount++;
            // Optional: Serial.println("TWAI TX failed");
        }

        if (alerts & TWAI_ALERT_ERR_PASS) {
            // Controller entered error-passive
            // TX is already in trouble here
            sendErrorCount++;
        }

        if (alerts & TWAI_ALERT_BUS_OFF) {
            // This is terminal without reset
            resetBus();
        }
    }

    twai_status_info_t status;
    twai_get_status_info(&status);

    //bool errorPressure = status.tx_error_counter > 5;
    bool stalled = millis() - lastBusTime > 500;
    if (sendErrorCount>=3 && stalled)
        resetBus();
}

void EspBus::resetBus() {
    Serial.println("********* Resetting ESP CAN bus...");
    lastBusTime=millis();
    sendErrorCount=0;

    twai_stop();
    //delay(20);
    vTaskDelay(pdMS_TO_TICKS(50));

    twai_driver_uninstall();
    //delay(20);
    vTaskDelay(pdMS_TO_TICKS(50));

    // GPIO unstick
    gpio_reset_pin((gpio_num_t)txPin);
    gpio_set_direction((gpio_num_t)txPin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)txPin, 0);

    gpio_reset_pin((gpio_num_t)rxPin);
    gpio_set_direction((gpio_num_t)rxPin, GPIO_MODE_INPUT);

    vTaskDelay(pdMS_TO_TICKS(20));

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

    uint32_t alerts_to_enable =
        TWAI_ALERT_TX_FAILED |
        TWAI_ALERT_BUS_OFF   |
        TWAI_ALERT_ERR_PASS  |
        TWAI_ALERT_BUS_RECOVERED;

    ESP_ERROR_CHECK(twai_reconfigure_alerts(alerts_to_enable, NULL));
}

#endif