#include <stdio.h>
#include <string.h>

#ifndef memcpy_s
void memcpy_s(void* dest, size_t destsz, const void* src, size_t count);
#endif

#ifndef fopen_s
unsigned short int fopen_s(FILE** file, const char* filename, const char* mode);
#endif

#ifndef printf_s
void printf_s(const char* format, ...);
#endif

#ifndef strcpy_s
void strcpy_s(char* dest, size_t destsz, const char* src);
#endif

#ifndef sscanf_s
void sscanf_s(const char* str, const char* format, ...);
#endif
