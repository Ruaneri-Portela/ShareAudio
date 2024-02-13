#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "log.h"

void SA_GenIV(unsigned char* iv)
{
    for (int i = 0; i < 128; i++)
    {
        iv[i] = rand() % 256;
    }
}

int SA_SetupKey(const char* key, unsigned char keyData[256])
{
    size_t len = strlen(key);
    if (len == 0)
    {
        SA_Log("Null key", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    else
    {
        len > 256 ? len = 256 : len;
        memset(keyData, 0, 256);
        memcpy(keyData, key, len);
    }
    return 1;
}

EVP_CIPHER_CTX* SA_IniCtx()
{
    EVP_CIPHER_CTX* ctx;
    if ((ctx = EVP_CIPHER_CTX_new()))
    {
        return ctx;
    }
    else
    {
        SA_Log("Cannot stated CTX", LOG_CRIPTO, LOG_CLASS_ERROR);
        return NULL;
    }
}

void SA_CloseCtx(EVP_CIPHER_CTX* ctx)
{
    EVP_CIPHER_CTX_free(ctx);
}

int SA_PrepareEncryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv)
{
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        SA_Log("Cannot Setup Key", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    else
    {
        return 1;
    }
}

int SA_PrepareDecryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv)
{
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        SA_Log("Cannot Setup Key", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    else
    {
        return 1;
    }
}

int SA_EncryptData(unsigned char* rawData, int rawDataSize, unsigned char* cipherText, EVP_CIPHER_CTX* ctx)
{
    int lenSize = 0;
    int cipherLen = 0;
    if (1 != EVP_EncryptUpdate(ctx, cipherText, &lenSize, rawData, rawDataSize))
    {
        SA_Log("Cannot Encrypy Data 1/2", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    cipherLen = lenSize;
    if (1 != EVP_EncryptFinal_ex(ctx, cipherText + lenSize, &lenSize))
    {
        SA_Log("Cannot Encrypy Data 2/2", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    cipherLen += lenSize;
    return cipherLen;
}

int SA_DecryptData(unsigned char* cipherText, int cipherTextLen, unsigned char* rawData, EVP_CIPHER_CTX* ctx)
{
    int lenSize = 0;
    int plainLen = 0;
    if (1 != EVP_DecryptUpdate(ctx, rawData, &lenSize, cipherText, cipherTextLen))
    {
        SA_Log("Cannot Decrypt Data 1/2", LOG_CRIPTO, LOG_CLASS_ERROR);
        return 0;
    }
    plainLen = lenSize;
    if (1 != EVP_DecryptFinal_ex(ctx, rawData + lenSize, &lenSize))
    {
        SA_Log("Cannot Decrypt Data 2/2", LOG_CRIPTO, LOG_CLASS_WARNING);
        return 0;
    }
    plainLen += lenSize;
    return plainLen;
}

size_t SA_GetEncriptySize(size_t size) {
    if(size % 16 == 0) {
		return size + 16;
    }
    else {
        return (size_t)ceil((double)size / 16) * 16;
    }
}