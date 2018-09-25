#ifndef __OTA_H
#define __OTA_H

/**
* @brief OTA����״̬
*/
enum ota_state{
	
	SDI_OTA_IDLE,                 ///< ��ʼ״̬
	SDI_OTA_REQ_FW_VER,           ///< ��ȡ�汾
	SDI_OTA_UPDATE_CONN_PARAM,    ///< ���²���
	SDI_OTA_START_STATE,          ///< ��ʼ��������
	SDI_OTA_END_STATE,            ///< �������
	SDI_OTA_TIMEOUT_STATE,        ///< ���䳬ʱ
	SDI_OTA_WRITE_ERROR_STATE,    ///< ����״̬
};

	
extern void SDI_OTA_init(unsigned int new_firmware_addr, unsigned int update_complete_flag_addr);
extern void SDI_OTA_rcceive_data(unsigned char* ptr, unsigned char len);
extern unsigned char SDI_OTA_Process(unsigned long times);
extern void SDI_OTA_timeout(void);


#endif
