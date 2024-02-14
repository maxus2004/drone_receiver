#include "software_serial.h"

#define tx_h() PORTC |= (1<<5)
#define tx_l() PORTC &= ~(1<<5)
#define tx_delay() __asm__ __volatile__ ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n")

void sserial_begin() {
    pinMode(A4, INPUT);
    pinMode(A5, OUTPUT);
    tx_h();
}

void sserial_write(uint8_t data) {
    uint8_t bits[10];
    bits[0] = 0;
    for (int i = 0;i < 8;i++) {
        bits[i + 1] = (data >> i) % 2;
    }
    bits[9] = 1;
    cli();
    for (uint8_t i = 0;i < 10;i++) {
        if (bits[i]) {
            tx_h();
        } else {
            tx_l();
        }
        tx_delay();
    }
    sei();
}

void sserial_write(void* data, uint8_t length) {
    uint8_t* bytes = (uint8_t*)data;
    for (uint8_t i = 0;i < length;i++) {
        sserial_write(bytes[i]);
    }
}