#ifndef __SDI_BLE_H
#define __SDI_BLE_H


//#define __NEW_PROTOCOL

extern unsigned char sdiProfile_SetParameter( unsigned char param, unsigned char len, void *value );
#define SDI_send_data_to_app(buf, len)   sdiProfile_SetParameter(0, len, buf)

#define PROTOCOL_CMD_HEADER (0x4D50)
#define PROTOCOL_RSP_HEADER (0x504D)

//#ifndef __NEW_PROTOCOL

//recvive from app command
#define ES_CMD_CONNECT_FLOW (0x00)
#define ES_CMD_GET_VER      (0x01)
#define ES_CMD_GET_MAC      (0x02)
#define ES_CMD_BANDLE       (0x03)
#define ES_CMD_ADV_INTERVAL (0x04)
#define ES_CMD_EN_SPEAKER   (0x05)
#define ES_CMD_SPEAKER_TIME (0x06)
#define ES_CMD_LOSE_MODE    (0x07)
#define ES_CMD_EN_PHONE     (0x08)
#define ES_CMD_NUM          ES_CMD_EN_PHONE
#define ES_RSP_FORMAT_ERR   (0x20)

#define ES_RSP_SUCCESS      (0x00) //Ӧ��ɹ�
#define ES_RSP_ERR_FORMAT   (0x01) //���ݸ�ʽ����
#define ES_RSP_ERR_CRC      (0x02) //����CRC����
#define ES_RSP_ERR_LENGTH   (0x03) //���ݳ��Ȳ�ƥ��
#define ES_RSP_ERR_DATA     (0x04) //�������ݴ���
#define ES_RSP_ERR_ID       (0x05) //����ID������

#define ES_RSP_ERR_INVALID_ID   (0x10)
#define ES_RSP_ERR_APP_ID   (0x11)
#define ES_RSP_ERR_FLOW     (0x12)


struct protocol{

    unsigned short header;
	unsigned char length;
	unsigned char id;

	union payload_t{
		struct{
			unsigned char status;
			unsigned char data[15];
		}rsp;
		unsigned char data[16];
	}payload;	
};


extern void SDI_ble_data_parse(unsigned char *ptr, unsigned int len);
extern void SDI_handle_process(unsigned long times);


/* type: 0-init, 1-close, 2-write, 3-read */
extern void SDI_I2C_init(unsigned char enable);
extern void SDI_I2C_write(unsigned char device_id, unsigned char reg, unsigned char val);
extern unsigned char SDI_I2C_read(unsigned char device_id, unsigned char reg);



#define CMD_ID_LONG_PRESS (0x01)
#define CMD_ID_TRIG_PRESS (0x02)
extern void SDI_send_app_data_fdq(unsigned char id, unsigned char count, unsigned char times);//�����������ݽӿ�

#define FIRMWARE_VER   (0x31) //��ʾ3.0�汾
/************** �汾���¼�¼ ***********************
* ��ʽ�ͷŰ汾 31��2018.01.07
*     
* �汾 32��2018.06.22
*   1���޸�Ư����Ҳ�ܴ������Ϲ���Ϣ��Bug,
*
*
*/


#endif
