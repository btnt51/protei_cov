mkdir ./build
cd ./build
cmake  -DPATCH_VERSION=$1 -DCMAKE_BUILD_TYPE=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 ../
cmake --build .
cmake --build . --target $5
if [ -n "$6" ]; then
  cmake -E tar cfv ./$6.zip --format=7zip ./$5
fi
