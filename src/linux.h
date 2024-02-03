#include <stdio.h>
#include <string.h>
#include "config.h"
#if defined(LINUX)

void memcpy_s(void* dest, size_t destsz, const void* src, size_t count);

unsigned short int fopen_s(FILE** file, const char* filename, const char* mode);

void printf_s(const char* format, ...);

void strcpy_s(char* dest, size_t destsz, const char* src);

void sscanf_s(const char* str, const char* format, ...);

void sprintf_s(char* dest, size_t destsz, const char* format, ...);

#endif
