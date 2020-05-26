#!/bin/sh

git submodule update --init --recursive

pushd .

cd Logless && mkdir -p build && cd build && ../configure.py && make logless.a && make spawner

popd
