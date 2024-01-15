#!/bin/bash
cp -f ./x64/Release/*.exe /y/
cp -f ./x64/Release/*.dll /y/
paDLL=$(which libportaudio.dll)
cp -f $paDLL /y/
cp -f ./build/*.exe /y/ShareAudio_GCC.exe