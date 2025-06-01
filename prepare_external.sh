#!/bin/bash

function build_dependency()
{
    pushd .
    mkdir -p build_dependency/$2
    cd build_dependency/$2
    cmake ../../$1 -DCMAKE_INSTALL_PREFIX=../../install_dependency
    make -j$(nproc) install
    popd
}

git submodule update --init --recursive

mkdir -p build_dependency
mkdir -p install_dependency

build_dependency cxx_dependency/bfc     bfc
build_dependency cxx_dependency/cum     cum
build_dependency cxx_dependency/logless logless