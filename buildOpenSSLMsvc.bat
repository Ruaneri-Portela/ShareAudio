cd D:\Data\ShareAudio\openssl
set PATH=D:\Data\ShareAudio\msvc\openssl\nasm;D:\Data\ShareAudio\msvc\openssl\perl\perl\bin;%PATH%
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
perl Configure VC-WIN64A && nmake && nmake test
(copy)
nmake clean
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
perl Configure VC-WIN32 && nmake && nmake test
(copy)
nmake clean
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
perl Configure VC-WIN64-ARM && nmake && nmake test
(copy)
nmake clean
