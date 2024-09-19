#!/bin/bash

git submodule update --init --recursive &&
mkdir -p firmware/picotool/build &&
cd firmware/picotool/build &&
printf "\n--- CONFIGURING ---\n" &&
cmake -DPICO_SDK_PATH=../../pico-sdk .. &&
printf "\n--- BUILDING ---\n" &&
make &&
cd ../../.. &&
mkdir -p firmware/build/protobuf &&
protoc --cpp_out=firmware/build/protobuf protobuf/message.proto