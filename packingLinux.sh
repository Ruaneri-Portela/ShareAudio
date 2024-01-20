#!/bin/bash
GIT_VERSION=$(git describe --tags)
rm -f ./src/VERSION.h
echo "#define VERSION \"$GIT_VERSION\"" >> ./src/VERSION.h
make clean
make
zip -j "build/ShareAudio_GNU-GCC_x64($GIT_VERSION)_linux.zip" build/ShareAudio
