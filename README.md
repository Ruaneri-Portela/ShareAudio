# ShareAudio

ShareAudio is a software designed for sharing audio between computers using TCP/IP. It utilizes PortAudio as an audio backend and Winsock2 with a network API on Windows. The current build is specifically for Windows, and it is written in C.

## Features

- **Low Resource Consumption:** ShareAudio is built with the goal of minimizing RAM and CPU usage, making it suitable for running as a systemd/sysv service on Linux or as a Windows Service. Both the client and server components use less than 4MB of RAM.

- **Transmission Method:** The software employs a datagram over TCP/IP for audio transmission. It encapsulates RAW audio via the network, resulting in high-fidelity (HI-FI) quality audio. For example, a 32-bit depth audio at 96000 Hz 2.0 consumes around 12 Mbps, while a 48000 audio uses around 6 Mbps.

- **Future Developments:**
  - Compression and Encryption: Future versions aim to implement compression and encryption between the audio-sharing pair.
  - Platform Compatibility: The code is intended to be portable and compatible with any system that has PortAudio, including Linux, MacOS, BSD, and Haiku.

## Usage

- **Binary**
  - Download .Zip on Builds Diretory
  - Make sure you have Visual Studio Redistributable 2022 64-bit
  - Run

- **Building and Running:**
  - Clone the repository.
  - You can choice build system, as Makefile to MSYS2/MinGw environment or Visual Studio 2022
  - To build in VS 2022 there are two ways, download portaudio via VCPKG or clone portaudio's git, for the second option see PortAudio's git for instructions on how to build PortAuduio in VS.

## Technical Details

- **Audio Backend:** PortAudio
- **Networking:** Winsock2 with a network API on Windows
- **Audio Quality:** HI-FI quality with transmission rates mentioned above

## Compatibility

For now only Windows is compatible

## License

This software is distributed under the MIT license. See the LICENSE file for more details.

## Contributing

We welcome contributions! Feel free to open issues or submit pull requests to help improve ShareAudio.

