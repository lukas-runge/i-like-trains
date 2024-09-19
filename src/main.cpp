/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"

// #define WAIT_TIME_1 58
// #define WAIT_TIME_0 116

#define WAIT_TIME_1 58
#define WAIT_TIME_0 116

#define LED_PIN 25
#define MINUS 14
#define PLUS 15

#define ADDRESS 13

#define FORWARD 0b01111111         // drive forward
#define BACKWARD 0b01011111        // drive backward
#define HALT 0b01100000            // halt
#define EMERGENCY_STOP 0b01100001  // emergency stop

#define LIGHTS_ON 0b10010000   // turn on light
#define LIGHTS_OFF 0b10000000  // turn off light

#define IDLE_ADDRESS 0xFF
#define IDLE_COMMAND 0x00

void invert() {
    gpio_put(MINUS, gpio_get(MINUS) ^ 1);
    gpio_put(PLUS, gpio_get(PLUS) ^ 1);
}

void send0() {
    invert();
    sleep_us(WAIT_TIME_0);
    invert();
    sleep_us(WAIT_TIME_0);
}

void send1() {
    invert();
    sleep_us(WAIT_TIME_1);
    invert();
    sleep_us(WAIT_TIME_1);
}

void sendByteMSB(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
            send1();
        } else {
            send0();
        }
    }
}

void output_bit(uint8_t bit_val) {
    if (bit_val == 0) {
        send0();
    } else {
        send1();
    }
}

void sendCommand(uint8_t address, uint8_t command) {
    // Send Packet preamble. send 14 Ones
    for (auto i = 0; i < 14; ++i) {
        output_bit(1);
    }

    output_bit(0);  // termination bit

    // send Address byte
    sendByteMSB(address);

    output_bit(0);  // termination bit

    // send Instruction byte
    sendByteMSB(command);

    output_bit(0);  // termination bit

    // send Error Detection byte
    sendByteMSB(address ^ command);
    output_bit(1);
}

int main() {
    stdio_usb_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    gpio_init(MINUS);
    gpio_set_dir(MINUS, GPIO_OUT);
    gpio_put(MINUS, 0);

    gpio_init(PLUS);
    gpio_set_dir(PLUS, GPIO_OUT);
    gpio_put(PLUS, 1);

    auto time = to_ms_since_boot(get_absolute_time());

    while (true) {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);

        auto newTime = to_ms_since_boot(get_absolute_time());

        if (newTime - time > 1000) {
            break;
        }
    }

    time = to_ms_since_boot(get_absolute_time());

    sendCommand(ADDRESS, FORWARD);

    while (true) {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);

        auto newTime = to_ms_since_boot(get_absolute_time());

        if (newTime - time > 5000) {
            break;
        }
    }

    time = to_ms_since_boot(get_absolute_time());

    sendCommand(ADDRESS, HALT);

    while (true) {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);

        auto newTime = to_ms_since_boot(get_absolute_time());

        if (newTime - time > 5000) {
            break;
        }
    }

    while (true)
    {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);
    }
}