# I Like Trains!

## get submodules recursively
```bash
git submodule update --init --recursive
```

## configure
```bash
cd build && cmake -DPICO_SDK_PATH=../pico-sdk .. & cd ..
```

## make
```bash
cd build && make -j14 & cd ..
```

## connect Pico

## flash
```bash
cp build/main.uf2 /Volumes/RPI-RP2
```