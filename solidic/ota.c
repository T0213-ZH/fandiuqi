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
* @name OTA命令ID、OTA完成或失败返回值
* @{
*/
#define SDI_OTA_CMD_FW_VERSION (0xFFFF) ///< 获取固件版本
#define SDI_OTA_CMD_START_REQ  (0xFF03) ///< 请求更新连接参数
#define SDI_OTA_CMD_START      (0xFF01) ///< 开始发送OTA有交效数据包
#define SDI_OTA_CMD_END        (0xFF02) ///< OTA升级完成

#define BT_OTA_SUCCESS            (0x00) ///< 升级完成
#define BT_OTA_ERR_WRITE_FLASH    (0x01) ///< 写flash错误
#define BT_OTA_ERR_CRC            (0x02) ///< crc较验错误
#define BT_OTA_ERR_LOSS_PACKET    (0x03) ///< 丢数据包错误
/** @} */



/**
* @brief OTA 数据存储地址
*/
struct ota_update_addr{
	unsigned int store_new_firmware;  ///< 存储新固件地址
	unsigned int store_complete_flag; ///< 存储更新完成标志
};

struct ota_update_addr g_ota_update_addr = {0}; ///< OTA 更新数据地址

/** 
* @brief OTA命令及数据包格式
*/
struct ota_data_format{
//	union{
		unsigned short cmd;      ///< 命令ID    
//		unsigned short index;    ///< 数据包序号
//	};
	unsigned char  buf[16];     ///< 有效数据  
	unsigned short crc;         ///< CRC效验值 
};


/**
* @name OTA固件存储地址、存储完成标志地址
* @{
*/
/*(0x20000)*/ /*(0x73000)*/
#define SDI_OTA_FRIMWARE_ADDR  (g_ota_update_addr.store_new_firmware)  ///<更新固件存储起始位置
#define SDI_OTA_FLG_ADR		   (g_ota_update_addr.store_complete_flag) ///<存储更新固件完成的标志
#define SDI_OTA_FRIMWARE_ADDR_OFFSET(addr)  (SDI_OTA_FRIMWARE_ADDR + addr) ///<固件更新偏移地址
/** @} */


/**
* @brief ota控制
*/
struct ota_ctr{
	enum ota_state state; ///< ota 当前状态 
	int index;            ///< ota 升级固件数据包序号 
	unsigned long times;  ///< ota 升级过程中时间计数 
};

/** ota 控制变量 */
struct ota_ctr g_ota_ctr;

static void SDI_OTA_reset_ctr_arg(void){
	g_ota_ctr.state = SDI_OTA_IDLE;
	g_ota_ctr.index = -1;
	g_ota_ctr.times = 0;
}

/**
* @brief 获取系统运行时间
*/
extern unsigned long SDI_BLE_GetCurTimes(void);

/**
* @brief 初始化OTA 数据更新地址
* @param new_firmware_addr  存储新固件地址
* @param update_complete_flag_addr 存储更新完成标志位地址
*/
void SDI_OTA_init(unsigned int new_firmware_addr, unsigned int update_complete_flag_addr){
	
	g_ota_update_addr.store_new_firmware = new_firmware_addr;
	g_ota_update_addr.store_complete_flag = update_complete_flag_addr;

	SDI_OTA_reset_ctr_arg();
}

/**
* @brief OTA升级过程中应答数据
* @param id    OTA命令ID
* @param len   数据长度
* @param state 应答数据内容
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
* @brief 写数据到flash指定地址
* @param addr 指定数据区域
* @param *ptr  指向数据buf
* @return 0:写数据成功, 1:写数据失败
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
* @brief 擦除指定flash里面数据
* @param index 指定数据区域
**/
static void SDI_OTA_erase_frimware(int index){
	int i;
	
	for(i=0; i<index; i+=256){
		SDI_OTA_flash_erase(SDI_OTA_FRIMWARE_ADDR_OFFSET(1 << 4));
	}
}

/**
* @brief 获取ota当前状态
* @return g_ota_ctr.state 当前状态
**/
unsigned char SDI_OTA_GetStatus(void){
	return g_ota_ctr.state;
}

/**
* @brief OTA接收数据包处理
* @param *ptr 指向数据包指针
* @param len 数据包长度
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

		// 留2秒时间等待下一个数据包
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 4; 
	}else if((SDI_OTA_CMD_END == packet.cmd) && (SDI_OTA_START_STATE == g_ota_ctr.state)){
		g_ota_ctr.state = SDI_OTA_END_STATE;

		SDI_OTA_response(SDI_OTA_CMD_END, 1, 0x02);

		// 留2秒时间等待应答数据发送完成
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
    	// 留2秒时间等待下一个数据包
		g_ota_ctr.times = SDI_BLE_GetCurTimes() + 2;
	}
}

/**
* @brief OTA完成后重启
*/
static void SDI_OTA_reboot(void){
	
	unsigned int flag = 165;

	SDI_OTA_reset_ctr_arg();
	
	SDI_OTA_flash_erase(SDI_OTA_FLG_ADR);
	SDI_OTA_flash_write(SDI_OTA_FLG_ADR, sizeof(unsigned int), (void *)&flag);

	SDI_OTA_REBOOT();
}

/**
* @brief OTA超时处理
*/
void SDI_OTA_timeout(void){
	
	SDI_OTA_DISCONN();
	
	if(g_ota_ctr.index >= 0){
		SDI_OTA_erase_frimware(g_ota_ctr.index);
	}
	SDI_OTA_reset_ctr_arg();
}

/**
* @brief OTA升级成功后重启或者超时处理
* @param[in] times 当前时间
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

