#ifdef _MSC_VER
#define COMPILE "MSVC"
#pragma comment(lib, "ws2_32.lib")
#elif __GNUC__
#define COMPILE "GCC"
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#define WINDOWS
#define ISWIN 1
#elif defined(__linux__)
#include "linux.h"
#include <netinet/in.h>
#define LINUX
#define ISWIN 0
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define ADDRESS_FAMILY int
#define closesocket close
#endif

#if defined(DLL_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include "VERSION.h"

#define DATASIZE 1024
