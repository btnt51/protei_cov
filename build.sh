#!/bin/bash

mkdir ./build
cd ./build

if [ -n "$7" ]; then
  if [ -n "${11}" ]; then
    cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 -DCMAKE_CXX_COMPILER=$7 -DBoost_INCLUDE_DIR=$9 -DBoost_LIBRARY_DIRS=${10} -DBoost_ROOT=${11} ../
  else
    cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 -DCMAKE_CXX_COMPILER=$7 ../
  fi
else
  cmake -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 ../
fi

if [ -n "${8}" ]; then
  cmake --build . --parallel ${8}
  cmake --build . --target $5 --parallel ${8}
else
  cmake --build .
  cmake --build . --target $5
fi

if [ -n "$6" ] && [ "$6" != "test" ]; then
  cmake -E tar cfv ./$6.zip --format=7zip ./$5 ./base.json
fi