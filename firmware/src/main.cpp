/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pb_decode.h>
#include <pb_encode.h>

#include <string>

#include "message.pb.h"
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

uint32_t get32BitUIntFromStdin() {
    uint32_t unsignedInt;

    unsignedInt = getchar();
    unsignedInt |= getchar() << 8;
    unsignedInt |= getchar() << 16;
    unsignedInt |= getchar() << 24;

    return unsignedInt;
}

int main() {
    stdio_init_all();

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

    // while (true)
    // {
    //     printf("Hello, world!\n");
    //     auto newTime = to_ms_since_boot(get_absolute_time());
    //     if (newTime - time > 10000)
    //     {
    //         break;
    //     }
    // }

    while (true) {
        uint32_t messageLength = get32BitUIntFromStdin();

        uint8_t buffer[messageLength];
        for (uint32_t i = 0; i < messageLength; i++) {
            buffer[i] = getchar();
        }

        pb_istream_t stream = pb_istream_from_buffer(buffer, messageLength);

        serial_Message message = serial_Message_init_zero;

        if (!pb_decode(&stream, serial_Message_fields, &message)) {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
            continue;
        }

        if (message.which_content == serial_Message_handshake_tag) {
            serial_Message response = serial_Message_init_zero;

            uint8_t buffer[SERIAL_MESSAGE_PB_H_MAX_SIZE];

            pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

            response.which_content = serial_Message_handshake_tag;
            response.content.handshake = serial_Handshake_init_default;
            response.content.handshake.type = serial_HandshakeType_RESPONSE;

            if (!pb_encode(&stream, serial_Message_fields, &response)) {
                printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
                continue;
            }

            printf("%c%c%c%c", (stream.bytes_written >> 0) & 0xFF, (stream.bytes_written >> 8) & 0xFF, (stream.bytes_written >> 16) & 0xFF, (stream.bytes_written >> 24) & 0xFF);
            for (uint32_t i = 0; i < stream.bytes_written; i++) {
                putchar(buffer[i]);
            }
        }
    }

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

    while (true) {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);
    }
}