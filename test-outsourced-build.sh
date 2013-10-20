#!/bin/bash
rm -rf _build _install
mkdir _build _install
cd _build
rm -f ../CMakeCache.txt ../vmime/config.hpp ../vmime/export-static.hpp ../vmime/export-shared.hpp
#cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_SHARED_LIBRARY=NO
#cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_TESTS=YES
cmake .. -DCMAKE_INSTALL_PREFIX=../_install -DVMIME_BUILD_TESTS=YES -DVMIME_BUILD_SAMPLES=YES -DVMIME_TLS_SUPPORT_LIB_IS_OPENSSL=ON -DVMIME_TLS_SUPPORT_LIB_IS_GNUTLS=OFF -DCMAKE_BUILD_TYPE=Release
cmake .. -L
make
make install

