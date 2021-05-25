#!/bin/bash

apt-get update
apt-get upgrade -y
apt-get install -y build-essential module-assistant dkms \
     gcc-arm-linux-gnueabihf libbluetooth-dev autoconf \
     gdb-multiarch libc6-armhf-cross libc6-dev-arm64-cross \
     zlib1g libwiringpi2 wiringpi libwiringpi-dev libtool \
     pkg-config libglib2.0-dev libdbus-glib-1-dev libdbus-1-dev \
     libgio3.0-cil-dev 