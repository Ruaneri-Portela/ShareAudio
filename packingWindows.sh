#!/bin/bash
make clean
make
# packing for MSVC
version=$(git describe --tags)
builds=("Win32" "ARM64" "x64")
deps=(./portaudioMSVC2022)
for build in "${builds[@]}"; do
    mkdir -p ./build/$build 
    for dep in "${deps[@]}"; do
        cp $dep/$build/Release/*.dll ./build/$build/ -f
    done
    cp ./$build/Release/*.exe ./build/$build/ -f
    zip "./build/ShareAudio_MSVC_$build($version).zip" ./build/$build/* -j
    rm -rf ./build/$build
done
# packing for x64_GNU_GCC
mkdir -p ./build/x64_GNU_GCC
paDLL=$(which libportaudio.dll)
cp $paDLL ./build/x64_GNU_GCC/ -f
cp ./build/*.exe ./build/x64_GNU_GCC -f
zip "./build/ShareAudio_GNU-GCC_x64($version).zip" ./build/x64_GNU_GCC/* -j
rm -rf ./build/x64gcc