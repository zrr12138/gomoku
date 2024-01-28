set -uexo pipefail
export CXX=/opt/rh/devtoolset-8/root/usr/bin/g++
export CC=/opt/rh/devtoolset-8/root/usr/bin/gcc
mkdir -p build && cd build
cmake ..
make -j40
cd ..