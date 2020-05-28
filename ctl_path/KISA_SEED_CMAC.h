#ifndef _SEED_CMAC_H_
#define _SEED_CMAC_H_

#define BLOCK_SIZE_SEED         16

#ifdef __cplusplus
extern "C"
{
#endif
void SEED_KeySched(unsigned char *mKey, unsigned int *rKey );
void SEED_Encrypt(unsigned int *pOut, unsigned int *pIn, unsigned int *rKey);
void SEED_Decrypt(unsigned int *pOut, unsigned int *pIn, unsigned int *rKey);
void Byte2Word(unsigned int *dst, const unsigned char *src, const int srcLen);
void Word2Byte(unsigned char *dst, const unsigned int *src, const int srcLen);
void SEED_CMAC_SubkeySched(unsigned char *sKey);
int SEED_Generate_CMAC(unsigned char *pMAC, int macLen, unsigned char *pIn, int inLen, unsigned char *mKey);
int SEED_Verify_CMAC(unsigned char *pMAC, int macLen, unsigned char *pIn, int inLen, unsigned char *mKey);
void print_title(const char *title);
void print_hex(const char *valName, const unsigned char *data, const int dataLen);
void print_result(const char *func, int ret);
int asc2hex(unsigned char *dst, const char *src);
#ifdef __cplusplus
}
#endif


#else
#endif
