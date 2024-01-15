#!/bin/bash
make clean
make
# packing for MSVC
version=$(git describe --tags)
builds=("Win32" "ARM64" "x64")
for build in "${builds[@]}"; do
    if [ "$build" == "Win32" ]; then
        msbild="x86" 
    else
        msbild="$build"
    fi
    "/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" ShareAudio.sln -t:Clean -p:Configuration=Release -p:Platform="$msbild"
    "/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" ShareAudio.sln -p:Configuration=Release -p:Platform="$msbild"
    "/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" ShareAudio.sln -t:Clean -p:Configuration=Debug -p:Platform="$msbild"
    "/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" ShareAudio.sln -p:Configuration=Debug -p:Platform="$msbild"
    mkdir -p "./build/$build" 
    cp "./$build/Release/"*.exe "./build/$build/" -f
    cp "./$build/Release/"*.dll "./build/$build/" -f
    zip "./build/ShareAudio_MSVC_$build($version).zip" "./build/$build/"* -j
    rm -rf "./build/$build"
done
# packing for x64_GNU_GCC
mkdir -p "./build/x64_GNU_GCC"
paDLL=$(which libportaudio.dll)
cp "$paDLL" "./build/x64_GNU_GCC/" -f
cp "./build/"*.exe "./build/x64_GNU_GCC" -f
zip "./build/ShareAudio_GNU-GCC_x64($version).zip" "./build/x64_GNU_GCC/"* -j
rm -rf "./build/x64_GNU_GCC"
