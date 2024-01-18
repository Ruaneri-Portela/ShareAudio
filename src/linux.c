#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifndef memcpy_s
void memcpy_s(void* dest, size_t destsz, const void* src, size_t count)
{
    if (destsz < count)
    {
        return;
    }
    memcpy(dest, src, count);
}
#endif

#ifndef fopen_s
unsigned short int fopen_s(FILE** file, const char* filename, const char* mode)
{
    *file = fopen(filename, mode);
    if(file == NULL){
        return 0;
    }
    return 1;
}
#endif

#ifndef printf_s
void printf_s(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

#ifndef strcpy_s
void strcpy_s(char* dest, size_t destsz, const char* src)
{
    if (destsz < strlen(src))
    {
        return;
    }
    strcpy(dest, src);
}
#endif

#ifndef sscanf_s
void sscanf_s(const char* str, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsscanf(str, format, args);
    va_end(args);
}
#endif
