/**
* @file     crc16.h
* @brief    协议数据进行CRC较验API
* @author   zhihua.tao
*
* @date     2016-11.14
* @version  1.0.0
* @copyright www.solidic.net
**/
#ifndef __CRC16_H
#define __CRC16_H

/**
* 计算CRC较验数值
* @param[in] data 需要较验的数值
* @param[in] len  长度
* @return 较验后的数值
**/
extern unsigned short CRC16_count(unsigned char *ptr, unsigned short len);

#endif
