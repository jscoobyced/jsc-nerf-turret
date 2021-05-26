#!/bin/bash

mkdir -p build
cd build
../configure --prefix=${PWD}/build/ && make