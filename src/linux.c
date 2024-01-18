#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
void memcpy_s(void* dest, size_t destsz, const void* src, size_t count)
{
    if (destsz < count)
    {
        return;
    }
    memcpy(dest, src, count);
}


unsigned short int fopen_s(FILE** file, const char* filename, const char* mode)
{
    *file = fopen(filename, mode);
    if(file == NULL){
        return 0;
    }
    return 1;
}

void printf_s(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void strcpy_s(char* dest, size_t destsz, const char* src)
{
    if (destsz < strlen(src))
    {
        return;
    }
    strcpy(dest, src);
}

void sscanf_s(const char* str, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsscanf(str, format, args);
    va_end(args);
}
#endif