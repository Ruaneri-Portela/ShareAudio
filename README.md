# ShareAudio

ShareAudio is software written in C designed for sharing desktop audio between computers, with the intention of being lightweight, portable and multiplatform, working over TCP/IP and using PortAudio as an audio backend.

Your intention is to create something that can run as a service, which, after installed and properly configured, is plugin-and-play

## Features

- **Low Resource Consumption:** ShareAudio is built with the goal of minimizing RAM and CPU usage, making it suitable for running as a systemd/sysv service on Linux or as a Windows Service. Both the client and server components use less than 4MB of RAM.

- **Transmission Method:** The software employs a datagram over TCP/IP for audio transmission. It encapsulates RAW audio via the network, resulting in high-fidelity (HI-FI) quality audio. For example, a 32-bit depth audio at 96000 Hz 2.0 consumes around 6 Mbps, while a 48000 audio uses around 3 Mbps.

- **Next Genereation Plataforms** As it is a new project, we already support Windows ARM64, making it software optimized for the new architecture for Windows PCs without the need for emulation or translation, As it is open source, unlike other solutions, it can run on MIPS, RISV-V, ARMv7 and other lesser-known platforms

- **Future Developments:**
  - Compression: Future versions aim to implement compression between the audio-sharing pair.
  - Platform Compatibility: The code is intended to be portable and compatible with any system that has PortAudio, including Linux, MacOS, BSD, and Haiku.
  - Modularization: Using other audio backends besides portaudio, in the future there will be support for Google's Oboe to support Android. Convert parts into libraries to allow integration with other codes and languages
  - Multi-depth audio: Allowing the use of audio in different sizes such as the most famous 16/24/32 bits, downplaing systems to allow devices with higher sampling to be reproduced on smaller ones

## Usage

- **Binary**
  - Download .zip on Releases
  - On Windows make sure you have Visual Studio Redistributable 2022 for your CPU architecture
  - On Linux make sure you have libportaudio on your system
  - Run

- **Building and Running:**
  - Clone the repository.

  ***Windows***
  - You can choice build system, as Makefile to MSYS2/MinGw environment or Visual Studio 2022
  - To build via MSVC, you must clone Portaudio's git and also download ASIO, the ASIO SDK must be placed in the asio folder in the project directory
  - You must compile OpenSSL using the .bat in the git root, but remember to download NASM and Perl as described in the OpenSSL "How to compile" tutorial, they must be in the directories that will be exported to the path use what you have in the .bat as a reference
  - You can do this for both OpenSSL and PortuAudio or for one exclusively, activate VCPKG in the project settings
  - It is also possible to compile via VCPKG, you must dereference the project, and use global includes instead of the directory, , it is not recommended for the following reason
  - On Windows, only the most recent git compilation of Portaudio supports Audio Loopback, which means that the version via VCPKG and MSYS/MinGW will not be able to replicate the desktop audio
  - For the build using the MinGw toolchains, the project comes with the Makefile
  - For MinGw builds they will be placed inside the build directory, for MSVC look inside the respective directories for the architecture

  ***Linux***
  - To build on Linux, you only need the development toolchain, the famous build-essential in the case of those based on Debian, or the similar meta package in your distro and libportaudio-dev and openssl-dev
  - The executable will be placed in ./build

## Technical Details

- **Audio Backend:** PortAudio
- **Networking:** Berkeley Socket in Linux and Winsock2 on Windows, work over TCP/IP
- **Audio Quality:** HI-FI quality with transmission rates mentioned above
- **Encryption:** Symmetric using AES CBC 256 bits deep using OpenSSL EVP

## Compatibility

Compatible with Windows and Linux

## License

This software is distributed under the MIT license. See the LICENSE file for more details.

## Contributing

We welcome contributions! Feel free to open issues or submit pull requests to help improve ShareAudio.

