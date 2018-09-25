#include "ota.h"
#include "stdlib.h"
#include "crc16.h"

#include "flash_interface.h"

extern unsigned char GAPRole_TerminateConnection(void);
	
#define SDI_OTA_flash_write(addr, len, buf)  writeFlash(addr, buf, len) 
#define SDI_OTA_flash_read(addr, len, buf)   readFlash(addr, buf, len) 
#define SDI_OTA_flash_erase(addr)            eraseFlash((addr) >> 12) 

extern void SDI_system_reset(void);
#define SDI_OTA_REBOOT()    SDI_system_reset()

extern unsigned char GAPRole_TerminateConnection(void);
#define SDI_OTA_DISCONN() 	GAPRole_TerminateConnection()

extern unsigned char sdiProfile_SetParameter( unsigned char param, unsigned char len, void *value );
#define SDI_ota_send_data_to_app(buf, len)   sdiProfile_SetParameter(0, len, buf)


/**
* @name OTA����ID��OTA��ɻ�ʧ�ܷ���ֵ
* @{
*/
#define SDI_OTA_CMD_FW_VERSION (0xFFFF) ///< ��ȡ�̼��汾
#define SDI_OTA_CMD_START_REQ  (0xFF03) ///< ����������Ӳ���
#define SDI_OTA_CMD_START      (0xFF01) ///< ��ʼ����OTA�н�Ч���ݰ�
#define SDI_OTA_CMD_END        (0xFF02) ///< OTA�������

#define BT_OTA_SUCCESS            (0x00) ///< �������
#define BT_OTA_ERR_WRITE_FLASH    (0x01) ///< дflash����
#define BT_OTA_ERR_CRC            (0x02) ///< crc�������
#define BT_OTA_ERR_LOSS_PACKET    (0x03) ///< �����ݰ�����
/** @} */



/**
* @brief OTA ���ݴ洢��ַ
*/
struct ota_update_addr{
	unsigned int store_new_firmware;  ///< �洢�¹̼���ַ
	unsigned int store_complete_flag; ///< �洢������ɱ�־
};

struct ota_update_addr g_ota_update_addr = {0}; ///< OTA �������ݵ�ַ

/** 
* @brief OTA������ݰ���ʽ
*/
struct ota_data_format{
//	union{
		unsigned short cmd;      ///< ����ID    
//		unsigned short index;    ///< ���ݰ����
//	};
	unsigned char  buf[16];     ///< ��Ч����  
	unsigned short crc;         ///< CRCЧ��ֵ 
};


/**
* @name OTA�̼��洢��ַ���洢��ɱ�־��ַ
* @{
*/
/*(0x20000)*/ /*(0x73000)*/
#define SDI_OTA_FRIMWARE_ADDR  (g_ota_update_addr.store_new_firmware)  ///<���¹̼��洢��ʼλ��
#define SDI_OTA_FLG_ADR		   (g_ota_update_addr.store_complete_flag) ///<�洢���¹̼���ɵı�־
#define SDI_OTA_FRIMWARE_ADDR_OFFSET(addr)  (SDI_OTA_FRIMWARE_ADDR + addr) ///<�̼�����ƫ�Ƶ�ַ
/** @} */


/**
* @brief ota����
*/
struct ota_ctr{
	enum ota_state state; ///< ota ��ǰ״̬ 
	int index;            ///< ota �����̼����ݰ���� 
	unsigned long times;  ///< ota ����������ʱ����� 
};

/** ota ���Ʊ��� */
struct ota_ctr g_ota_ctr;

static void SDI_OTA_reset_ctr_arg(void){
	g_ota_ctr.state = SDI_OTA_IDLE;
	g_ota_ctr.index = -1;
	g_ota_ctr.times = 0;
}

/**
* @brief ��ȡϵͳ����ʱ��
*/
extern unsigned long SDI_BLE_GetCurTimes(void);

/**
* @brief ��ʼ��OTA ���ݸ��µ�ַ
* @param new_firmware_addr  �洢�¹̼���ַ
* @param update_complete_flag_addr �洢������ɱ�־λ��ַ
*/
void SDI_OTA_init(unsigned int new_firmware_addr, unsigned int update_complete_flag_addr){
	
	g_ota_update_addr.store_new_firmware = new_firmware_addr;
	g_ota_update_addr.store_complete_flag = update_complete_flag_addr;

	SDI_OTA_reset_ctr_arg();
}

/**
* @brief OTA����������Ӧ������
* @param id    OTA����ID
* @param len   ���ݳ���
* @param state Ӧ����������
**/
//void SDI_OTA_response(unsigned short id, unsigned int len, unsigned char *ptr){

//	unsigned char buf[16] = {0};
//	unsigned char index = 0;

//	buf[index++] = (id >> 8) & 0xFF;
//	buf[index++] = (id >> 0) & 0xFF;

//	for(; len; len--)
//		buf[index++] = *ptr++;

//	///SDI_APP_protocol_send(PROTOCOL_EVENT, SL_CMD_OTA_RSP, index, buf);
//}	

void SDI_OTA_response(unsigned short id, unsigned int len, unsigned char data){

	unsigned char buf[16] = {0};
	unsigned char index = 0;
	unsigned short crc = 0;

	buf[index++] = 0x4D;
	buf[index++] = 0x50;
	
	buf[index++] = 0xEE;

	buf[index++] = 0x01;

	buf[index++] = data;

	crc = CRC16_count(&buf[2], 3);

	buf[index++] = (crc >> 0) & 0xFF;
	buf[index++] = (crc >> 8) & 0xFF;

	SDI_ota_send_data_to_app(buf, index);
}

/**
* @brief д���ݵ�flashָ����ַ
* @param addr ָ����������
* @param *ptr  ָ������buf
* @return 0:д���ݳɹ�, 1:д����ʧ��
**/
static unsigned char SDI_OTA_write_frimware(unsigned int addr ,unsigned char* ptr){
	
	unsigned char flash_check[16];
	
	SDI_OTA_flash_write(SDI_OTA_FRIMWARE_ADDR_OFFSET(addr), 16, ptr);
	SDI_OTA_flash_read(SDI_OTA_FRIMWARE_ADDR_OFFSET(addr),  16, flash_check);
	
	if(!__memcmp(flash_check, ptr, 16)){
		return 0;
	}
	return 1;
}

/**
* @brief ����ָ��flash��������
* @param index ָ����������
**/
static void SDI_OTA_erase_frimware(int index){
	int i;
	
	for(i=0; i<index; i+=256){
		SDI_OTA_flash_erase(SDI_OTA_FRIMWARE_ADDR_OFFSET(1 << 4));
	}
}

/**
* @brief ��ȡota��ǰ״̬
* @return g_ota_ctr.state ��ǰ״̬
**/
unsigned char SDI_OTA_GetStatus(void){
	return g_ota_ctr.state;
}

/**
* @brief OTA�������ݰ�����
* @param *ptr ָ�����ݰ�ָ��
* @param len ���ݰ�����
**/
//void SDI_OTA_FlashTest(void);
extern void SDI_ota_updata_conn_parameter(void);

void SDI_OTA_rcceive_data(unsigned char* ptr, unsigned char len){
	
	struct ota_data_format packet = {0};
	unsigned char result = BT_OTA_SUCCESS;
	
	__memcpy(&packet, ptr, len);
	
	if(SDI_OTA_CMD_FW_VERSION == packet.cmd){

		//SDI_OTA_response(SDI_OTA_CMD_FW_VERSION, __strlen(SMARTLOCK_VER), SMARTLOCK_VER);
	}else if((SDI_OTA_CMD_START_REQ == packet.cmd) && (SDI_OTA_IDLE == g_ota_ctr.state)){					

		g_ota_ctr.state = SDI_OTA_UPDATE_CONN_PARAM;

		SDI_OTA_response(SDI_OTA_CMD_START_REQ, 1, 0x03);
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 4;

//		SDI_OTA_FlashTest();
		SDI_ota_updata_conn_parameter();
	}else if((SDI_OTA_CMD_START == packet.cmd) && (SDI_OTA_UPDATE_CONN_PARAM == g_ota_ctr.state)){
		g_ota_ctr.state = SDI_OTA_START_STATE;
		g_ota_ctr.index = -1;

		SDI_OTA_response(SDI_OTA_CMD_START, 1, 0x01);

		// ��2��ʱ��ȴ���һ�����ݰ�
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 4; 
	}else if((SDI_OTA_CMD_END == packet.cmd) && (SDI_OTA_START_STATE == g_ota_ctr.state)){
		g_ota_ctr.state = SDI_OTA_END_STATE;

		SDI_OTA_response(SDI_OTA_CMD_END, 1, 0x02);

		// ��2��ʱ��ȴ�Ӧ�����ݷ������
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 4;
	}else if(SDI_OTA_START_STATE == g_ota_ctr.state){

		unsigned short ota_addr = packet.cmd;		
		if((g_ota_ctr.index + 1) == ota_addr){
			if(packet.crc == CRC16_count((void *)&packet, 18)){

				if(SDI_OTA_write_frimware((ota_addr << 4), packet.buf)){
					result = BT_OTA_ERR_WRITE_FLASH;
				}else{
					g_ota_ctr.index = ota_addr;
				}
			}else{
				result = BT_OTA_ERR_CRC;
			}			
		}else if(g_ota_ctr.index > ota_addr){
			
		}else{
			result = BT_OTA_ERR_LOSS_PACKET;
		}
		
		if(result){
			
			g_ota_ctr.state = SDI_OTA_WRITE_ERROR_STATE;
			SDI_OTA_response(SDI_OTA_CMD_START, 1, 0xA0 | result);
		}
    	// ��2��ʱ��ȴ���һ�����ݰ�
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 2;
	}
}

/**
* @brief OTA��ɺ�����
*/
static void SDI_OTA_reboot(void){
	
	unsigned int flag = 165;

	SDI_OTA_reset_ctr_arg();
	
	SDI_OTA_flash_erase(SDI_OTA_FLG_ADR);
	SDI_OTA_flash_write(SDI_OTA_FLG_ADR, sizeof(unsigned int), (void *)&flag);

	SDI_OTA_REBOOT();
}

/**
* @brief OTA��ʱ����
*/
void SDI_OTA_timeout(void){
	
	SDI_OTA_DISCONN();
	
	if(g_ota_ctr.index >= 0){
		SDI_OTA_erase_frimware(g_ota_ctr.index);
	}
	SDI_OTA_reset_ctr_arg();
}

/**
* @brief OTA�����ɹ����������߳�ʱ����
* @param[in] times ��ǰʱ��
**/
unsigned char SDI_OTA_Process(unsigned long times){	
		
	if((g_ota_ctr.times == times) && (g_ota_ctr.times != 0)){
		g_ota_ctr.times = 0;
	
		if(SDI_OTA_END_STATE == g_ota_ctr.state){
			SDI_OTA_reboot();			
		}else{
			SDI_OTA_timeout();
		}
	}

	return g_ota_ctr.state;
}


//void SDI_OTA_FlashTest(void){

//	unsigned char buf[] = "0123456789ABCDEF";
//	unsigned char buf1[] = "FFFFFFFFFFFFFFFF";
//	if(!SDI_OTA_write_frimware(0, buf)){
//		SDI_ota_send_data_to_app("123456", 6);
//		SDI_OTA_write_frimware(0, buf1);
//	}else{
//		SDI_ota_send_data_to_app("abcdef", 6);
//	}
//}

