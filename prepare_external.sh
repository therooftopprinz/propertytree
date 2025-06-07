#!/bin/bash

function build_dependency()
{
    pushd .
    mkdir -p build_dependency/$2
    cd build_dependency/$2
    cmake ../../$1 -DCMAKE_INSTALL_PREFIX=../../install -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=true
    make -j$(nproc) install
    popd
}

git submodule update --init --recursive

mkdir -p build_dependency
mkdir -p install

build_dependency cxx_dependency/bfc     bfc
build_dependency cxx_dependency/cum     cum
build_dependency cxx_dependency/logless logless