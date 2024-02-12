#ifdef _MSC_VER
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <stdlib.h>

void genIV(unsigned char* iv)
{
    for (int i = 0; i < 128; i++)
    {
        iv[i] = rand() % 256;
    }
}

int setupKey(const char* key, unsigned char keyData[256])
{
    size_t len = strlen(key);
    if (len == 0)
    {
        printf("Null key ?");
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

EVP_CIPHER_CTX* iniCtx()
{
    EVP_CIPHER_CTX* ctx;
    if ((ctx = EVP_CIPHER_CTX_new()))
    {
        return ctx;
    }
    else
    {
        printf("Cannot stated CTX");
        return NULL;
    }
}

void closeCtx(EVP_CIPHER_CTX* ctx)
{
    EVP_CIPHER_CTX_free(ctx);
}

int prepareEncryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv)
{
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        printf("Cannot Setup Key");
        return 0;
    }
    else
    {
        return 1;
    }
}

int prepareDecryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv)
{
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        printf("Cannot Setup Key");
        return 0;
    }
    else
    {
        return 1;
    }
}

int encryptData(unsigned char* rawData, int rawDataSize, unsigned char* cipherText, EVP_CIPHER_CTX* ctx)
{
    int lenSize = 0;
    int cipherLen = 0;
    if (1 != EVP_EncryptUpdate(ctx, cipherText, &lenSize, rawData, rawDataSize))
    {
        printf("Cannot Encrypy Data");
        return 0;
    }
    cipherLen = lenSize;
    if (1 != EVP_EncryptFinal_ex(ctx, cipherText + lenSize, &lenSize))
    {
        printf("Cannot Encrypy Data");
        return 0;
    }
    cipherLen += lenSize;
    return cipherLen;
}

int decryptData(unsigned char* cipherText, int cipherTextLen, unsigned char* rawData, EVP_CIPHER_CTX* ctx)
{
    int lenSize = 0;
    int plainLen = 0;
    if (1 != EVP_DecryptUpdate(ctx, rawData, &lenSize, cipherText, cipherTextLen))
    {
        printf("Cannot Decrypt Data");
        return 0;
    }
    plainLen = lenSize;
    if (1 != EVP_DecryptFinal_ex(ctx, rawData + lenSize, &lenSize))
    {
        printf("Cannot Decrypt Data");
        return 0;
    }
    plainLen += lenSize;
    return plainLen;
}

int test()
{
    unsigned char key[256];
    unsigned char iv[128];
    genIV(iv);
    setupKey("essa e uma senha forte?", key);
    unsigned char* plainText = (unsigned char*)("Essa e uma mensagem para ser encriptada");

    // Encrypt
    unsigned char toCipher[128];
    memset(toCipher, 0, 128);
    int plainTextLen = (int)strlen((char*)plainText);
    if((plainTextLen) % 16 != 0)
	{
        printf("%d",((plainTextLen/16) +1) * 16);
	}
    EVP_CIPHER_CTX* eCtx = iniCtx();
    prepareEncryptCtx(eCtx, key, iv);
    int cipherSize = encryptData(plainText, plainTextLen, toCipher, eCtx);
    printf("%d\n", cipherSize);
    //BIO_dump_fp(stdout, (const char*)toCipher, cipherSize);
    closeCtx(eCtx);

    // Decrypt
    unsigned char toPlain[128];
    memset(toPlain, 0, 128);
    EVP_CIPHER_CTX* dCtx = iniCtx();
    prepareDecryptCtx(dCtx, key, iv);
    decryptData(toCipher, cipherSize, toPlain, dCtx);
    printf("%s\n", toPlain);
    closeCtx(dCtx);

    return 0;
}
#else
test() {
	return 0;
}
#endif