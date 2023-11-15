mkdir ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=$1 -DPATCH_VERSION=$2 -DWITH_TESTS=$3 -DLIGHTWEIGHTBABY=$4 ../
cmake --build .
cmake --build . --target $5
if [ "$6" -eq  "0" ]
  then
    exit 0
  else
    cmake -E tar cfv ./$6.zip --format=7zip ./$5
fi
