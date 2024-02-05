#ifdef _MSC_VER
#define COMPILE "MSVC"
#elif __GNUC__
#define COMPILE "GCC"
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WINDOWS
#define ISWIN 1
#elif defined(__linux__)
#define LINUX
#define ISWIN 0
#include "linux.h"
#endif

#if defined(DLL_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include "VERSION.h"