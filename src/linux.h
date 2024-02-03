#include <stdio.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

void memcpy_s(void* dest, size_t destsz, const void* src, size_t count);

unsigned short int fopen_s(FILE** file, const char* filename, const char* mode);

void printf_s(const char* format, ...);

void strcpy_s(char* dest, size_t destsz, const char* src);

void sscanf_s(const char* str, const char* format, ...);

void sprintf_s(char* dest, size_t destsz, const char* format, ...);

#endif
