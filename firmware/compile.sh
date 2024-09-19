#!/bin/bash

mkdir -p build &&
cd build &&
printf "\n--- CONFIGURING ---\n" &&
cmake -DPICO_SDK_PATH=../pico-sdk .. &&
printf "\n--- BUILDING ---\n" &&
make &&
cd ..