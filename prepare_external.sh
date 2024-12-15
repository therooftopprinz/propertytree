#!/bin/bash

git submodule update --init --recursive

pushd .

cd Logless
mkdir -p build
cd build

pwd

cmake ..
make

popd