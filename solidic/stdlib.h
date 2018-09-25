#ifndef __STDLIB_H
#define __STDLIB_H

extern int  __strncmp(const void *dest, const void *src, unsigned int size);
extern void __memcpy(void *dest, const void *src, unsigned int size);
extern void __memset(void *dest, unsigned char value, unsigned int size);
extern int __memcmp(const void *m1, const void *m2, unsigned int n);
extern unsigned int __strlen(const char *str);

#endif
