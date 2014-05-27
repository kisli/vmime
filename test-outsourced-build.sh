#!/bin/bash
rm -rf _build _install
mkdir _build _install
cd _build
rm -f ../CMakeCache.txt ../src/vmime/config.hpp ../src/vmime/export-static.hpp ../src/vmime/export-shared.hpp
#cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_SHARED_LIBRARY=NO
#cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_TESTS=YES
cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_TESTS=YES -DVMIME_BUILD_SAMPLES=YES -DVMIME_TLS_SUPPORT_LIB=openssl -DCMAKE_BUILD_TYPE=Debug
cmake .. -L
make
make install

