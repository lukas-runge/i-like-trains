/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pb_decode.h>
#include <pb_encode.h>

#include <string>

#include "message.pb.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

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

uint32_t readMessageLength() {
    uint32_t messageLength;

    messageLength = getchar();
    messageLength |= getchar() << 8;
    messageLength |= getchar() << 16;
    messageLength |= getchar() << 24;

    return messageLength;
}

void sendMessageLength(uint32_t messageLength) {
    putchar(messageLength & 0xFF);
    putchar((messageLength >> 8) & 0xFF);
    putchar((messageLength >> 16) & 0xFF);
    putchar((messageLength >> 24) & 0xFF);
}

void sendMessage(uint8_t *buffer, uint32_t messageLength) {
    for (uint32_t i = 0; i < messageLength; i++) {
        putchar(buffer[i]);
    }
}

struct {
    uint32_t messageLength;
    uint8_t buffer[SERIAL_MESSAGE_PB_H_MAX_SIZE];
} Message;

queue_t messageQueue;

void InputHandler() {
    while (true) {
        uint32_t messageLength = readMessageLength();

        uint8_t buffer[messageLength];
        for (uint32_t i = 0; i < messageLength; i++) {
            buffer[i] = getchar();
        }

        pb_istream_t stream = pb_istream_from_buffer(buffer, messageLength);

        serial_Message message = serial_Message_init_zero;

        if (!pb_decode(&stream, serial_Message_fields, &message)) {
            continue;
        }

        queue_add_blocking(&messageQueue, &message);

        gpio_put(LED_PIN, gpio_get(LED_PIN) ^ 1);
    }
}

void sendHandshakeResponse() {
    serial_Message response = serial_Message_init_zero;

    uint8_t buffer[SERIAL_MESSAGE_PB_H_MAX_SIZE];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    response.which_content = serial_Message_handshake_tag;
    response.content.handshake = serial_Handshake_init_default;
    response.content.handshake.type = serial_HandshakeType_RESPONSE;

    if (!pb_encode(&stream, serial_Message_fields, &response)) {
        return;
    }

    sendMessageLength(stream.bytes_written);
    sendMessage(buffer, stream.bytes_written);
}

void handleControlPacket(serial_ControlPacket &controlPacket) {
    uint8_t address = controlPacket.address;

    switch (controlPacket.which_command) {
        case serial_ControlPacket_drive_tag:
            printf("Drive command: %d \n", 0b01000000 | (serial_ControlPacket_Direction_FORWARD == controlPacket.command.drive.direction) << 5 | controlPacket.command.drive.speed);

            sendCommand(address, 0b01000000 | (serial_ControlPacket_Direction_FORWARD == controlPacket.command.drive.direction) << 5 | controlPacket.command.drive.speed);
            break;

        case serial_ControlPacket_halt_tag:
            sendCommand(address, HALT);
            break;

        case serial_ControlPacket_emergency_stop_tag:
            for (size_t i = 0; i < 5; i++) {
                sendCommand(address, EMERGENCY_STOP);
            }
            break;

        case serial_ControlPacket_light_tag:
            printf("Light command: %d \n", controlPacket.command.light.on);
            sendCommand(address, controlPacket.command.light.on ? LIGHTS_ON : LIGHTS_OFF);
            break;

        default:
            break;
    }
}

void handleProtobufMessage(serial_Message &message) {
    if (message.which_content == serial_Message_handshake_tag) {
        sendHandshakeResponse();
        printf("Handshake response sent\n");
        return;
    }

    printf("Packet with content: %d\n", message.which_content);

    if (message.which_content == serial_Message_control_packet_tag)
        handleControlPacket(message.content.control_packet);
}

void handleQueue() {
    serial_Message message;

    if (queue_try_remove(&messageQueue, &message)) handleProtobufMessage(message);
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

    queue_init(&messageQueue, sizeof(Message), 10);

    multicore_launch_core1(InputHandler);

    auto time = to_ms_since_boot(get_absolute_time());

    while (true) {
        sendCommand(IDLE_ADDRESS, IDLE_COMMAND);
        handleQueue();
    }
}