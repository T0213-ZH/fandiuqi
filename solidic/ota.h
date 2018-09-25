#ifndef __OTA_H
#define __OTA_H

/**
* @brief OTA升级状态
*/
enum ota_state{
	
	SDI_OTA_IDLE,                 ///< 初始状态
	SDI_OTA_REQ_FW_VER,           ///< 获取版本
	SDI_OTA_UPDATE_CONN_PARAM,    ///< 更新参数
	SDI_OTA_START_STATE,          ///< 开始传输数据
	SDI_OTA_END_STATE,            ///< 传输完成
	SDI_OTA_TIMEOUT_STATE,        ///< 传输超时
	SDI_OTA_WRITE_ERROR_STATE,    ///< 出错状态
};

	
extern void SDI_OTA_init(unsigned int new_firmware_addr, unsigned int update_complete_flag_addr);
extern void SDI_OTA_rcceive_data(unsigned char* ptr, unsigned char len);
extern unsigned char SDI_OTA_Process(unsigned long times);
extern void SDI_OTA_timeout(void);


#endif
