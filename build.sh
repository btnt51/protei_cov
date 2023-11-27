#!/bin/bash

mkdir ./build
cd ./build

if [ -n "$7" ]; then
  if [ -n "${10}" ]; then
    cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 -DCMAKE_CXX_COMPILER=$7 \
      -DBoost_INCLUDE_DIR=$8 -DBoost_LIBRARY_DIRS=$9 -DBoost_ROOT=${10} ../
  else
    cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 -DCMAKE_CXX_COMPILER=$7 ../
  fi
else
  cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 ../
fi

cmake --build .
cmake --build . --target $5

if [ -n "$6" ]; then
  cmake -E tar cfv ./$6.zip --format=7zip ./$5
fi