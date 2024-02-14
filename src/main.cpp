#include <Arduino.h>
#include <SX126x.h>

#include "crsf.h"

crsf_channels_s rc_channels = {992, 992, 992, 992, 992, 992, 992, 992,
                               992, 992, 992, 992, 992, 992, 992, 992};

#define RF400_NRST 9
#define RF400_BUSY 8
#define RF400_NSS 10
#define RF400_TXEN 14
#define RF400_RXEN 15

#define RF900_NRST 6
#define RF900_BUSY 7
#define RF900_NSS 3
#define RF900_TXEN 5
#define RF900_RXEN 4

#define RADIO_OFF 0
#define RADIO_FSK 1
#define RADIO_LORA 2
#define RADIO_FHSS 3

SX126x rf400;
SX126x rf900;

uint8_t rf400_mode = RADIO_LORA;
uint8_t rf900_mode = RADIO_LORA;

typedef struct __attribute__((__packed__)) {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
    uint8_t b1 : 1;
    uint8_t b2 : 1;
    uint8_t b3 : 1;
} Controls;

typedef struct __attribute__((__packed__)) {
    uint8_t radio1_mode;
    uint32_t radio1_frequency;
    uint8_t radio2_mode;
    uint32_t radio2_frequency;
} RadioSettings;

Controls controls;
RadioSettings radioSettings;

void radio_init() {
    pinMode(RF400_NSS, OUTPUT);
    pinMode(RF900_NSS, OUTPUT);
    digitalWrite(RF400_NSS, HIGH);
    digitalWrite(RF900_NSS, HIGH);

    if (!rf400.begin(RF400_NSS, RF400_NRST, RF400_BUSY, -1, RF400_TXEN,
                     RF400_RXEN)) {
        Serial.println("Something wrong, can't start 433Mhz radio");
        while (1) {}
    }
    Serial.println("initialized 433Mhz radio");
    rf400.setFrequency(433000000);
    rf400.setTxPower(10, SX126X_TX_POWER_SX1268);
    rf400.setLoRaModulation(7, 125000, 5, false);
    rf400.setSyncWord(0x3444);
    rf400.setLoRaPacket(LORA_HEADER_EXPLICIT, 12, 15, true, false);
    Serial.println("configured 433Mhz radio");

    if (!rf900.begin(RF900_NSS, RF900_NRST, RF900_BUSY, -1, RF900_TXEN,
                     RF900_RXEN)) {
        Serial.println("can't start 868Mhz radio");
        while (1) {}
    }
    Serial.println("initialized 868Mhz radio");
    rf900.setFrequency(868000000);
    rf900.setTxPower(10, SX126X_TX_POWER_SX1268);
    rf900.setLoRaModulation(7, 125000, 5, false);
    rf900.setSyncWord(0x3444);
    rf900.setLoRaPacket(LORA_HEADER_EXPLICIT, 12, 15, true, false);
    Serial.println("configured 868Mhz radio");
}

void setup() {
    Serial.begin(115200);

    radio_init();
    crsf_begin();
}

void loop() {
    rf400.request();
    rf400.wait();

    const uint8_t length = rf400.available();
    uint8_t data[length];
    for (int i = 0; i < length; i++) {
        data[i] = rf400.read();
    }

    uint32_t packet_id = ((uint32_t*)data)[0];
    uint8_t packet_type = data[4];

    Serial.print("Received packet id: ");
    Serial.println(packet_id);

    if (packet_type == 0) {
        memcpy(&controls, data + 5, sizeof(Controls));

        rc_channels.roll = 192 + controls.x2 * 6.28 - 17;
        rc_channels.pitch = 192 + controls.y2 * 6.28 - 6;
        rc_channels.yaw = 192 + controls.x1 * 6.28 + 2;
        rc_channels.thrust = 192 + controls.y1 * 6.28;
        rc_channels.aux1 = controls.b1 ? 1792 : 192;
        rc_channels.aux2 = controls.b2 ? 1792 : 192;
        rc_channels.aux3 = controls.b3 ? 1792 : 192;
        crsf_sendChannels(&rc_channels);

        Serial.print("X1: ");
        Serial.println(controls.x1);
        Serial.print("Y1: ");
        Serial.println(controls.y1);
        Serial.print("X2: ");
        Serial.println(controls.x2);
        Serial.print("Y2: ");
        Serial.println(controls.y2);
        Serial.print("B1: ");
        Serial.println(controls.b1);
        Serial.print("B2: ");
        Serial.println(controls.b2);
        Serial.print("B3: ");
        Serial.println(controls.b3);
    } else if (packet_type == 1) {
        memcpy(&radioSettings, data + 5, sizeof(RadioSettings));

        rf400.setFrequency(radioSettings.radio1_frequency);
        switch (radioSettings.radio1_mode) {
            case RADIO_OFF:
                rf400_mode = RADIO_OFF;
                break;
            case RADIO_FSK:
                rf400_mode = RADIO_FSK;
                rf400.setModem(FSK_MODEM);
                break;
            case RADIO_LORA:
                rf400_mode = RADIO_LORA;
                rf400.setModem(LORA_MODEM);
                break;
            case RADIO_FHSS:
                rf400_mode = RADIO_FHSS;
                rf400.setModem(FSK_MODEM);
                break;
        }
        rf900.setFrequency(radioSettings.radio2_frequency);
        switch (radioSettings.radio2_mode) {
            case RADIO_OFF:
                rf900_mode = RADIO_OFF;
                break;
            case RADIO_FSK:
                rf900_mode = RADIO_FSK;
                rf900.setModem(FSK_MODEM);
                break;
            case RADIO_LORA:
                rf900_mode = RADIO_LORA;
                rf900.setModem(LORA_MODEM);
                break;
            case RADIO_FHSS:
                rf900_mode = RADIO_FHSS;
                rf900.setModem(FSK_MODEM);
                break;
        }
    }
}