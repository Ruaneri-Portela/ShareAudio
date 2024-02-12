#include <openssl/evp.h>

void genIV(unsigned char* iv);

int setupKey(const char* key, unsigned char keyData[256]);

EVP_CIPHER_CTX* iniCtx();

void closeCtx(EVP_CIPHER_CTX* ctx);

int prepareEncryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv);

int prepareDecryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv);

int encryptData(unsigned char* rawData, int rawDataSize, unsigned char* cipherText, EVP_CIPHER_CTX* ctx);

int decryptData(unsigned char* cipherText, int cipherTextLen, unsigned char* rawData, EVP_CIPHER_CTX* ctx);

int test();
