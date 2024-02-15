cd ShareAudio\openssl
mkdir ..\msvc
mkdir ..\msvc\openssl
mkdir ..\msvc\openssl\builds
set PATH=D:\Data\ShareAudio\msvc\openssl\nasm;D:\Data\ShareAudio\msvc\openssl\perl\perl\bin;%PATH%
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
nmake clean
perl Configure VC-WIN64A && nmake && nmake test
mkdir ..\msvc\openssl\builds\x64\
copy .\*.dll ..\msvc\openssl\builds\x64\
copy .\*.lib ..\msvc\openssl\builds\x64\
copy .\*.exp ..\msvc\openssl\builds\x64\
copy .\*.def ..\msvc\openssl\builds\x64\
copy .\*.pdb ..\msvc\openssl\builds\x64\
nmake clean
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
perl Configure VC-WIN32 && nmake && nmake test
mkdir ..\msvc\openssl\builds\Win32\
copy .\*.dll ..\msvc\openssl\builds\Win32\
copy .\*.lib ..\msvc\openssl\builds\Win32\
copy .\*.exp ..\msvc\openssl\builds\Win32\
copy .\*.def ..\msvc\openssl\builds\Win32\
copy .\*.pdb ..\msvc\openssl\builds\Win32\
nmake clean
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
perl Configure VC-WIN64-ARM && nmake && nmake test
mkdir ..\msvc\openssl\builds\ARM64\
copy .\*.dll ..\msvc\openssl\builds\ARM64\
copy .\*.lib ..\msvc\openssl\builds\ARM64\
copy .\*.exp ..\msvc\openssl\builds\ARM64\
copy .\*.def ..\msvc\openssl\builds\ARM64\
copy .\*.pdb ..\msvc\openssl\builds\ARM64\
