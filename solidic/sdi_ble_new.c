#include "sdi_ble.h"
//#include "rc4.h"
#include "crc16.h"
#include "stdlib.h"
#include "led.h"
#include "battery.h"
#include "msa300.h"
#include "ota.h"

#include <ti/sysbios/knl/Clock.h>
#include "util.h"
#include "../source/ti/blestack/boards/CC2640R2_LAUNCHXL/CC2640R2_LAUNCHXL.h"


static unsigned char g_connect_status = 0;
unsigned char  g_fish_send_buf[20] = {0};


#define CONN_STATUS_IDLE   (0)
#define CONN_STATUS_APPID  (1)
#define CONN_STATUS_USEID  (2)
#define CONN_STATUS_NORMAL (3)
static unsigned char conn_status_flow = CONN_STATUS_IDLE;

static const unsigned char es_app_id[4] = {0x45, 0x53, 0x5A, 0x48};//"ESTZ"


extern unsigned short SDI_get_adc_value(void);
extern void SDI_TerminateConnection(void);
//extern unsigned char SDI_OTA_Process(unsigned long times);
extern unsigned char SDI_OTA_GetStatus(void);
extern void sdi_change_device_name(uint8_t *p, uint8_t len);

extern unsigned long times_tick_count;


struct protocol send_packet = {
	.header = 0x504D,
	.length = 0,
	.id = 0,
	.payload = {
		.rsp = {
			.status = 0,
		},
	},
};

typedef struct {

	unsigned char bandle_id[4];
	unsigned char adv_interval_time;
	unsigned char speaker_work_time;
}es_config_info;

es_config_info g_es_config_info = {

	{0xFF, 0xFF, 0xFF, 0xFF},
	0,
	0,
};


void SDI_connection_ind(unsigned char type){

	g_connect_status = type;
	if(type)
		conn_status_flow = CONN_STATUS_APPID;
}

void protocol_send_event(unsigned char id){

	if(!g_connect_status) return;
}

unsigned char connect_flow_check(struct protocol *pOwn, struct protocol *p){

	unsigned char index = 0;
	unsigned char rsp = 0;
	
	if((pOwn->payload.data[0] == 0x00) && (conn_status_flow == CONN_STATUS_APPID)){ //APP ID check
		if(pOwn->length != 0x08){
			rsp = 1;
		}else{
			if((pOwn->payload.data[1] == es_app_id[0]) && (pOwn->payload.data[2] == es_app_id[1]) &&
				(pOwn->payload.data[3] == es_app_id[2]) && (pOwn->payload.data[4] == es_app_id[3])){
				conn_status_flow = CONN_STATUS_USEID;
			}else{
				rsp = 2;
			}
		}		
	}

	if((pOwn->payload.data[0] == 0x01) && (conn_status_flow == CONN_STATUS_USEID)){ //bandle id check
		if(pOwn->length != 0x08){
			rsp = 1;
		}else{
			if(((pOwn->payload.data[1] == g_es_config_info.bandle_id[0]) && (pOwn->payload.data[2] == g_es_config_info.bandle_id[1]) &&
				(pOwn->payload.data[3] == g_es_config_info.bandle_id[2]) && (pOwn->payload.data[4] == g_es_config_info.bandle_id[3])) ||
				((0xFF == g_es_config_info.bandle_id[0]) && (0xFF == g_es_config_info.bandle_id[1]) &&
				(0xFF == g_es_config_info.bandle_id[2]) && (0xFF == g_es_config_info.bandle_id[3]))){
			    conn_status_flow = CONN_STATUS_NORMAL;
			}else{
				rsp = 2;
			}
		}
	}

	if(!rsp){
		p->payload.rsp.status = ES_RSP_SUCCESS;
	}else{
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;
		conn_status_flow = CONN_STATUS_IDLE;
	}

	return index;
}

unsigned char get_version(struct protocol *pOwn, struct protocol *p){
	unsigned char index = 0;

	if(pOwn->length == 0x03){
		p->payload.rsp.status = ES_RSP_SUCCESS;
		p->payload.rsp.data[index++] = FIRMWARE_VER;
	}else{
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;
	}
	return index;
}

extern  uint8_t g_ownAddress[];
unsigned char get_mac_address(struct protocol *pOwn, struct protocol *p){
	unsigned char index = 0;

	if(pOwn->length == 0x03){
			
		p->payload.rsp.status = ES_RSP_SUCCESS;
		p->payload.rsp.data[index++] = g_ownAddress[5];
		p->payload.rsp.data[index++] = g_ownAddress[4];
		p->payload.rsp.data[index++] = g_ownAddress[3];
		p->payload.rsp.data[index++] = g_ownAddress[2];
		p->payload.rsp.data[index++] = g_ownAddress[1];
		p->payload.rsp.data[index++] = g_ownAddress[0];
	}else{	
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;	
	}
	return index;
}

unsigned char get_set_bandle(struct protocol *pOwn, struct protocol *p){

	unsigned char rsp = 0;
	unsigned char index = 0;

	if(pOwn->payload.data[0] == 0x00){
		
		if(pOwn->length != 0x08){
			rsp = 1;
		}else{
			g_es_config_info.bandle_id[0] = pOwn->payload.data[1];
			g_es_config_info.bandle_id[1] = pOwn->payload.data[2];
			g_es_config_info.bandle_id[2] = pOwn->payload.data[3];
			g_es_config_info.bandle_id[3] = pOwn->payload.data[4];
		}
	}else if(pOwn->payload.data[0] == 0x01){
	
		if(pOwn->length != 0x04){
			rsp = 1;
		}else{
			g_es_config_info.bandle_id[0] = 0xFF;
			g_es_config_info.bandle_id[1] = 0xFF;
			g_es_config_info.bandle_id[2] = 0xFF;
			g_es_config_info.bandle_id[3] = 0xFF;
		}
	}else{
		rsp = 1;
	}

	if(!rsp){
		p->payload.rsp.status = ES_RSP_SUCCESS;
	}else{
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;
	}
	return index;
}

unsigned char get_set_adv_interval(struct protocol *pOwn, struct protocol *p){

	unsigned char rsp = 0;
	unsigned char index = 0;

	if(pOwn->payload.data[0] == 0x00){
		
		if(pOwn->length != 0x04){
			rsp = 1;
		}else{
			p->payload.rsp.data[index++] = g_es_config_info.adv_interval_time;
		}
	}else if(pOwn->payload.data[0] == 0x01){
	
		if(pOwn->length != 0x05){
			rsp = 1;
		}else{
			g_es_config_info.adv_interval_time = pOwn->payload.data[1];
		}
	}else{
		rsp = 1;
	}

	if(!rsp){
		p->payload.rsp.status = ES_RSP_SUCCESS;
	}else{
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;
	}
	return index;
}

extern void SDI_en_speaker(unsigned char en);
unsigned char en_speaker(struct protocol *pOwn, struct protocol *p){
	
	unsigned char index = 0;	

	if(pOwn->length == 0x04){
		p->payload.rsp.status = ES_RSP_SUCCESS;
		SDI_en_speaker(pOwn->payload.data[0] != 0 ? 1: 0);
	}else{	
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;	
	}

	return index;
}

unsigned char get_set_speaker_work_time(struct protocol *pOwn, struct protocol *p){

	unsigned char index = 0;
	unsigned char rsp;
	
	if(pOwn->payload.data[0] == 0x00){

		if(pOwn->length != 0x04){
			rsp = 1;
		}else{
			p->payload.rsp.data[index++] = g_es_config_info.speaker_work_time;
		}
	}else{
		if(pOwn->length != 0x05){
			rsp = 1;
		}else{
			g_es_config_info.speaker_work_time = pOwn->payload.data[1]; 
		}
	}

	if(!rsp){
		p->payload.rsp.status = ES_RSP_SUCCESS;
	}else{
		p->payload.rsp.status = ES_RSP_ERR_LENGTH;
	}

	return index;

}


unsigned char en_lose_mode(struct protocol *pOwn, struct protocol *p){

	unsigned char index = 0;

	p->payload.rsp.status = ES_RSP_SUCCESS;

	return index;
}

unsigned char SDI_protoco_format_check(unsigned char *ptr, unsigned char len){

	struct protocol *p = (void *)ptr;

	if(len < 6) return ES_RSP_ERR_LENGTH;

	if(p->length != len - 3) return ES_RSP_ERR_DATA;

	if(p->header != PROTOCOL_CMD_HEADER) return ES_RSP_ERR_FORMAT;

	if(p->id > ES_CMD_NUM) return ES_RSP_ERR_ID;

	return ES_RSP_SUCCESS;
}

void SDI_ble_data_parse(unsigned char *ptr, unsigned int len){
	unsigned char index = 0;
	unsigned char rsp;

	if(SDI_OTA_GetStatus()) return;

	struct protocol *packet = (void *)ptr;

	SDI_send_data_to_app((void *)ptr, len);

	rsp = SDI_protoco_format_check((void *)packet, len);

    if(rsp != ES_RSP_SUCCESS){
		send_packet.id = ES_RSP_FORMAT_ERR;
		send_packet.payload.rsp.status = rsp;
    }else{

		if(conn_status_flow != CONN_STATUS_NORMAL){
			if(packet->id == ES_CMD_CONNECT_FLOW){
				index = connect_flow_check((void *)packet, &send_packet);
			}else{
				send_packet.payload.rsp.status = ES_RSP_ERR_FLOW;
			}
		}else{
	
			switch(packet->id){

				//case ES_CMD_CONNECT_FLOW: index = connect_flow_check((void *)packet, &send_packet);  break;
				case ES_CMD_GET_VER:      index = get_version((void *)packet, &send_packet);         break;
				case ES_CMD_GET_MAC:      index = get_mac_address((void *)packet, &send_packet);     break;
				case ES_CMD_BANDLE:       index = get_set_bandle((void *)packet, &send_packet);      break;
				case ES_CMD_ADV_INTERVAL: index = get_set_adv_interval((void *)packet, &send_packet);break;
				case ES_CMD_EN_SPEAKER:   index = en_speaker((void *)packet, &send_packet);          break;
				case ES_CMD_SPEAKER_TIME: index = get_set_speaker_work_time((void *)packet, &send_packet);break;
				case ES_CMD_LOSE_MODE:    index = en_lose_mode((void *)packet, &send_packet);        break;
				case ES_CMD_EN_PHONE:     break;
				default: send_packet.payload.rsp.status = ES_RSP_ERR_INVALID_ID; break;
			}
		}
		send_packet.id = packet->id;					
	}
	//crc
	send_packet.payload.rsp.data[index++] = 0xFF;
	send_packet.payload.rsp.data[index++] = 0xFF;
			
	send_packet.header = PROTOCOL_RSP_HEADER;
	send_packet.length = index + 2;	 //for status and id byte
	
	SDI_send_data_to_app((void *)&send_packet, send_packet.length + 3);

}


#define BAT_LEVEL_MAX (4)
const unsigned short battery_level_table[BAT_LEVEL_MAX]={
	0x1B0,//0x01F0, //2.4
	0x1F0,//0x021E, //2.6
	0x21E,//0x0247, //2.8
	0x240, //3.0
};
unsigned char g_batter_level = 5;

void bat_process_test(unsigned long times){

	
}

uint64_t sys_time_tick_second = 0;
unsigned long SDI_BLE_GetCurTimes(void){
	return sys_time_tick_second;
}

extern void SDI_Watchdog_clear(void);
void SDI_handle_process(unsigned long times){	

	static uint32_t count_flag = 0;

	if(++count_flag == 20){
		count_flag = 0;
		sys_time_tick_second++;
		//SDI_Watchdog_clear();
	}

	if(!SDI_OTA_GetStatus())
		bat_process_test(times);

}

unsigned char button_send_buf[7] = {0x4D, 0x50, 0x01, 0x01, 0x00, 0xFF, 0xFF};

void SDI_send_app_data_fdq(unsigned char id, unsigned char count, unsigned char times){

	if(!g_connect_status) return;

	button_send_buf[4] = id;
	button_send_buf[5] = count;
	button_send_buf[6] = times;
	
	SDI_send_data_to_app(button_send_buf, 7);
}

void SDI_send_app_debug(unsigned char *p, unsigned char len){

	SDI_send_data_to_app(p, len);
}

