void SA_GenIV(unsigned char* iv);

int SA_SetupKey(const char* key, unsigned char keyData[256]);

EVP_CIPHER_CTX* SA_IniCtx();

void SA_CloseCtx(EVP_CIPHER_CTX* ctx);

int SA_PrepareEncryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv);

int SA_PrepareDecryptCtx(EVP_CIPHER_CTX* ctx, unsigned char* key, unsigned char* iv);

int SA_EncryptData(unsigned char* rawData, int rawDataSize, unsigned char* cipherText, EVP_CIPHER_CTX* ctx);

int SA_DecryptData(unsigned char* cipherText, int cipherTextLen, unsigned char* rawData, EVP_CIPHER_CTX* ctx);

size_t SA_GetEncriptySize(size_t size);