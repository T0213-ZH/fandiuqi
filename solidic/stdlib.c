/**
* @file     stdlib.c                                                       
* @brief    �����C��׼��                                                  
* @author   zhihua.tao                                                     
*                                                                          
* @date     2017-08-31                                                     
* @version  1.0.0                                                          
* @copyright www.solidic.net                                               
*/

#include "stdlib.h"


/**
* @brief �ַ���dest��src�Ƚ�
* @param *dest ָ���ַ���ָ��
* @param *src  ָ���ַ���ָ��
* @param  size �Ƚϳ���
* @return 0:���, -1:destС��src, 1:dest����src
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
* @brief ����srcָ���ַ��size�����ݵ�desָ��ĵ�ַ
* @param *dest ָ��Ŀ�ĵ�ַָ��
* @param *src  ָ��Դ��ַָ��
* @param size �������ݴ�С
**/
void __memcpy(void *dest, const void *src, unsigned int size){
	char *_dest = (char *)dest;
	char *_src  = (char *)src;

	while(size--)
		*_dest++ = *_src++;
}

/**
* @brief ����destָ��ĵ�ַ��size�ռ����ֵΪvalue
* @param *dest  ָ����ʼ��ַ
* @param value  �����ֵ
* @param size   ����С
**/
void __memset(void *dest, unsigned char value, unsigned int size){
	char *_dest = (char *)dest;
	while(size--)
		*_dest++ = value;
}

/**
* @brief �Ƚ�dest��srcָ���ַ��size����ֵ
* @param *m1 ָ��Ŀ���ַ
* @param *m2  ָ��Դ��ַ
* @param n     �Ƚϴ�С
* @return      0:���, -1:�ַ���2�����ַ���1, 1:�ַ���1�����ַ���2
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
* @brief ��ȡ�ַ����ĳ���
* @param *str ָ���ַ���ָ��
* @return �����ַ�������
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


