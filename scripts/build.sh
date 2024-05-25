#!/bin/bash

if [[ $1 == "clean" ]]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

mkdir build -p && cd build && cmake .. && make -j
