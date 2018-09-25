#ifndef __RC4_H
#define __RC4_H


extern void SDI_RC4_SetKey(unsigned char *key);
extern void SDI_RC4_crypt(unsigned char *ptr, unsigned int len);

#endif

