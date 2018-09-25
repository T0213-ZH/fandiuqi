/**
* @file     crc16.h
* @brief    Э�����ݽ���CRC����API
* @author   zhihua.tao
*
* @date     2016-11.14
* @version  1.0.0
* @copyright www.solidic.net
**/
#ifndef __CRC16_H
#define __CRC16_H

/**
* ����CRC������ֵ
* @param[in] data ��Ҫ�������ֵ
* @param[in] len  ����
* @return ��������ֵ
**/
extern unsigned short CRC16_count(unsigned char *ptr, unsigned short len);

#endif
