#!/bin/bash

git submodule update --init --recursive &&
mkdir -p picotool/build &&
cd picotool/build &&
printf "\n--- CONFIGURING ---\n" &&
cmake -DPICO_SDK_PATH=../../pico-sdk .. &&
printf "\n--- BUILDING ---\n" &&
make &&
cd ../.. &&
mkdir -p build/protobuf
