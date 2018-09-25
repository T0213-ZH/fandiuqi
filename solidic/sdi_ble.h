#ifndef __SDI_BLE_H
#define __SDI_BLE_H


#define __NEW_PROTOCOL

extern unsigned char sdiProfile_SetParameter( unsigned char param, unsigned char len, void *value );
#define SDI_send_data_to_app(buf, len)   sdiProfile_SetParameter(0, len, buf)


#ifndef __NEW_PROTOCOL

//recvive from app command
#define FISH_CMD_SETTING         (0x01)
#define FISH_CMD_RAM_DATA        (0x02)
#define FISH_CMD_DEVICES_INFO    (0x03)
#define FISH_CMD_FW_VER          (0x04)
#define FISH_CMD_NIGHT_MODE      (0x05)
#define FISH_CMD_DEVICE_NAME     (0x06)


//send to app command
#define FISH_EVT_TEST_DATA       (0x01)
#define FISH_EVT_BITE            (0x02)
#define FISH_EVT_BAT_IND         (0x03)
#define FISH_EVT_SET_PARAM       (0x04)
#define FISH_EVT_VERSION         (0x05)


struct fish_data_format{
	unsigned char id;
	unsigned char length;

	unsigned char payload[18];
};

#else

#define FISH_VERSION_R          (0x31)


#define PRO_ID_MASK             (0xE0)
#define PRO_TYPE_MASK           (0x10)
#define PRO_LEN_MASK            (0x07)

#define PRO_ID_TEST_CMD         (0x01)
#define PRO_ID_BITE_CMD         (0x02)
#define PRO_ID_BAT_LEVEL_CMD    (0x03)
#define PRO_ID_PARAM_SETGET_CMD (0x04)
#define PRO_ID_GET_VER          (0x05)
#define PRO_ID_DEVICE_NAME_CMD  (0x06)


#define PARAM_LED_ONOFF         (0x80)
#define PARAM_LED_FLASH         (0x40)
#define PARAM_LED_BRIGH         (0x30)
#define PARAM_THREAN            (0x0F)

typedef struct protocol_packet_t{
		
	unsigned char len:3;
	unsigned char res:1;
	unsigned char type:1;
	unsigned char id:3;

	unsigned char payload[7];
}protocol_packet;

typedef struct ctr_param_t{
	unsigned char threan:4;
	unsigned char res:1;
	unsigned char raw_en:1;
	unsigned char led_flash:1;	
	unsigned char led_onoff:1;
}ctr_param;

#endif

extern void SDI_ble_data_parse(unsigned char *ptr, unsigned int len);
extern void SDI_handle_process(unsigned long times);


/* type: 0-init, 1-close, 2-write, 3-read */
extern void SDI_I2C_init(unsigned char enable);
extern void SDI_I2C_write(unsigned char device_id, unsigned char reg, unsigned char val);
extern unsigned char SDI_I2C_read(unsigned char device_id, unsigned char reg);


#define FIRMWARE_VER   (0x32) //表示3.0版本
/************** 版本更新记录 ***********************
* 正式释放版本 31，2018.01.07
*     
* 版本 32，2018.06.22
*   1、修改漂倒着也能触发鱼上钩消息的Bug,
*
*
*/


#endif
