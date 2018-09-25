/**
* @file     stdlib.c                                                       
* @brief    精简版C标准库                                                  
* @author   zhihua.tao                                                     
*                                                                          
* @date     2017-08-31                                                     
* @version  1.0.0                                                          
* @copyright www.solidic.net                                               
*/

#include "stdlib.h"


/**
* @brief 字符串dest与src比较
* @param *dest 指向字符串指针
* @param *src  指向字符串指针
* @param  size 比较长度
* @return 0:相等, -1:dest小于src, 1:dest大于src
**/
int __strncmp(const void *dest, const void *src, unsigned int size){
	char *_dest = (char *)dest;
	char *_src = (char *)src;

	while(size--){
		if(*_dest != *_src){
			if(*_dest > *_src) return 1;
			else return -1;
		}
		_dest++;
		_src++;
	}
	return 0;
}

/**
* @brief 复制src指向地址的size个数据到des指向的地址
* @param *dest 指向目的地址指针
* @param *src  指向源地址指针
* @param size 复制数据大小
**/
void __memcpy(void *dest, const void *src, unsigned int size){
	char *_dest = (char *)dest;
	char *_src  = (char *)src;

	while(size--)
		*_dest++ = *_src++;
}

/**
* @brief 设置dest指向的地址后size空间后数值为value
* @param *dest  指向起始地址
* @param value  填充数值
* @param size   填充大小
**/
void __memset(void *dest, unsigned char value, unsigned int size){
	char *_dest = (char *)dest;
	while(size--)
		*_dest++ = value;
}

/**
* @brief 比较dest与src指向地址后size个数值
* @param *m1 指向目标地址
* @param *m2  指向源地址
* @param n     比较大小
* @return      0:相等, -1:字符串2大于字符串1, 1:字符串1大于字符串2
**/
int __memcmp(const void *m1, const void *m2, unsigned int n){

	unsigned char *_s1 = (unsigned char *) m1;
	unsigned char *_s2 = (unsigned char *) m2;

	while (n--) {
		if (*_s1 != *_s2) {
			return *_s1 - *_s2;
		}
		_s1++;
		_s2++;
	}
	return 0;
}

/**
* @brief 获取字符串的长度
* @param *str 指向字符串指针
* @return 返回字符串长度
**/
unsigned int __strlen(const char *str) {

	unsigned int len = 0;

	if (str) {
		while (*str++) {
			len++;
		}
	}

	return len;
}


