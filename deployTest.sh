#!/bin/bash
cp -f ./x64/Release/*.exe /y/
cp -f ./portaudioMSVC2022/x64/Release/*.dll /y/
paDLL=$(which libportaudio.dll)
cp -f $paDLL /y/
cp -f ./build/*.exe /y/ShareAudio_GCC.exe