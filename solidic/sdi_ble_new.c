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

#define SDI_FISH_IDLE    (0)
#define SDI_FISH_INIT    (1)
#define SDI_FISH_INITING (2)
#define SDI_FISH_NORMAL  (3)
#define SDI_FISH_DISABLE (4)
static unsigned char g_fish_state = SDI_FISH_IDLE;

static unsigned long fish_time_count = 0;

static unsigned char g_bat_times_count = 0;
static unsigned short g_bat_buf[10] = {0};
static unsigned short g_adv_val = 0;
static unsigned char g_bat_level = 4;

#define FISH_CONN_TICK_TIMEOUT        (300) //10S
static unsigned short fish_conn_tick_count = 0;

int g_acc_val[3] = {0}; 
static int g_acc_val_old[3] = {0, 0, 0};

ctr_param g_ctr_param = {5, 0, 0, 1, 1};


extern unsigned short SDI_get_adc_value(void);
extern void SDI_TerminateConnection(void);
//extern unsigned char SDI_OTA_Process(unsigned long times);
extern unsigned char SDI_OTA_GetStatus(void);
extern void sdi_change_device_name(uint8_t *p, uint8_t len);

extern unsigned long times_tick_count;


#define SDI_data_crypt(x) 
//void SDI_data_crypt(unsigned char *ptr){
//	extern uint8_t advertData[];

//	*ptr = *ptr ^ advertData[11];
//}


void SDI_connection_ind(unsigned char type){

	g_connect_status = type;
	g_bat_times_count = 0;
	g_adv_val = 0;

	if(g_connect_status){ //已连接
		g_fish_state = SDI_FISH_INIT;

		g_ctr_param.raw_en = 0;
	}else{//断开连接
		g_fish_state = SDI_FISH_DISABLE;
	}	

	if(g_ctr_param.led_onoff){
		SDI_led_indication(SDI_LED_ON, 2, 200);
	}else{
		SDI_led_indication(SDI_LED_OFF, 2, 200);
	}
}

void protocol_send_event(unsigned char id){

	if(!g_connect_status) return;
	
	protocol_packet packet = {0};

	packet.id = id;
	packet.type = 0;
	packet.res = 0;

	if(packet.id == PRO_ID_TEST_CMD){

		packet.len = 6;
		packet.payload[0] = (g_acc_val[0] >> 8) & 0xFF;
		packet.payload[1] = (g_acc_val[0] >> 0) & 0xFF;
		
		packet.payload[2] = (g_acc_val[1] >> 8) & 0xFF;
		packet.payload[3] = (g_acc_val[1] >> 0) & 0xFF;
					
		packet.payload[4] = (g_acc_val[2] >> 8) & 0xFF;
		packet.payload[5] = (g_acc_val[2] >> 0) & 0xFF;

		SDI_data_crypt((void *)&packet);
		SDI_send_data_to_app((void *)&packet, packet.len+1);

	}else if(packet.id == PRO_ID_BITE_CMD){

		packet.len = 1;

		SDI_data_crypt((void *)&packet);
		SDI_send_data_to_app((void *)&packet, 1);
	}
}

void SDI_ble_data_parse(unsigned char *ptr, unsigned int len){

	if(SDI_OTA_GetStatus()) return;

	protocol_packet *packet = (void *)ptr;
	unsigned char rsp = 0;

	protocol_packet packet_rsp = {0};
	unsigned char length = 0;

	packet_rsp.id = packet->id;
	packet_rsp.type = 0;
	packet_rsp.res = 0;

	SDI_data_crypt(ptr);
	
	if(packet->id == PRO_ID_BAT_LEVEL_CMD){//0X70
		if(packet->type && !packet->len){
			
			packet_rsp.len = g_bat_level;
			length = 1;
			fish_conn_tick_count = 0;
			rsp = 1;
		}
		
	}else if(packet->id == PRO_ID_PARAM_SETGET_CMD){
		if(packet->len){//0x91 85
			unsigned char *p = (void *)&g_ctr_param;
			*p = packet->payload[0];
			packet_rsp.len = 0;

			SDI_led_indication(g_ctr_param.led_onoff, 0, 0); //更新指示灯状态
		}else{//0x98
			unsigned char *p = (void *)&g_ctr_param;

			packet_rsp.res = 1;
			packet_rsp.len = 1;
			packet_rsp.payload[0] = *p;
		}

		length = packet_rsp.len + 1;
		rsp = 1;
		
	}else if(packet->id == PRO_ID_GET_VER){//0xB0
		if(packet->type){

			packet_rsp.len = 1;
			packet_rsp.payload[0] = FISH_VERSION_R;
			length = packet_rsp.len + 1;
			
			rsp = 1;
		}
	}else if(packet->id == PRO_ID_DEVICE_NAME_CMD){//D6 54 5A 48 30 32 33

		if(packet->type){
			sdi_change_device_name(packet->payload, packet->len);
			packet_rsp.len = 0;
			length = packet_rsp.len + 1;
			
			rsp = 1;
		}
	}

	if(rsp){	
		
		SDI_data_crypt((void *)&packet_rsp);
		SDI_send_data_to_app((void *)&packet_rsp, length);
	}	
	
}

/************************ 鱼上钓算法 ***********************************/
const unsigned short threshold_table[11] = {4000,2500,2000,1500,1200,800,500,400,200,10,0};
#define X_MIN 0
#define X_MAX 10000
#define Y_MIN 10000  //13000
#define Y_MAX 30000  //25000
#define Z_MIN 0
#define Z_MAX 10000

#define X_DIFF_MIN 200    //400
#define X_DIFF_MAX 10000
#define Y_DIFF_MIN 800   //1000
#define Y_DIFF_MAX 10000
#define Z_DIFF_MIN 300
#define Z_DIFF_MAX 5000

static unsigned char fish(unsigned char level){ 
		
	unsigned int diff_X,diff_Y,diff_Z;
	if(level > 10) level = 10;

	diff_X = (g_acc_val_old[0] > g_acc_val[0]) ? (g_acc_val_old[0]-g_acc_val[0]) : (g_acc_val[0]-g_acc_val_old[0]);
	diff_Y = (g_acc_val_old[1] > g_acc_val[1]) ? (g_acc_val_old[1]-g_acc_val[1]) : (g_acc_val[1]-g_acc_val_old[1]);	
	diff_Z = (g_acc_val_old[2] > g_acc_val[2]) ? (g_acc_val_old[2]-g_acc_val[2]) : (g_acc_val[2]-g_acc_val_old[2]);

	if((g_acc_val_old[1] > g_acc_val[1])) return 0;

	if(((g_acc_val[0] > X_MIN/*ACC_X_LEVEL_MIN[level]*/ ) && (g_acc_val[0] < X_MAX))		
		&& ((g_acc_val[1] > Y_MIN) && (g_acc_val[1] < Y_MAX))
		&& ((g_acc_val[2] > Z_MIN) && (g_acc_val[2] < Z_MAX)) 		
		&& ((diff_X > (X_DIFF_MIN+threshold_table[level])) && (diff_X < X_DIFF_MAX))
		&& ((diff_Y > Y_DIFF_MIN) && (diff_Y < Y_DIFF_MAX))
		&& ((diff_Z > Z_DIFF_MIN) && (diff_Z < Z_DIFF_MAX))){
			return 1;
	}
			
	return 0;
}
/********************************************************************************************************************/

unsigned char SDI_get_msa_data(unsigned long times){

	static unsigned long time_count = 0;

	g_acc_val_old[0] = g_acc_val[0];		
	g_acc_val_old[1] = g_acc_val[1];	
	g_acc_val_old[2] = g_acc_val[2];


	if(msa_read_acc()){

	//if(0){
	if(!g_ctr_param.raw_en){
		if(fish(g_ctr_param.threan)){
			if(times > time_count){
				time_count = times + 30;
				//SDI_fish_send_data_to_app((void *)&packet, packet.length + 2);
				protocol_send_event(PRO_ID_BITE_CMD);
				
				if(g_ctr_param.led_flash){
					SDI_led_indication(g_ctr_param.led_onoff, 4, 200);
				}
			}	
		}
	}
	else{
		protocol_send_event(PRO_ID_TEST_CMD);
	}
	
    }
	return 0;
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

#if 0
//测试函数
{
	static unsigned long test_count = 0;
	static unsigned short adc_value = 0;
	unsigned char buf_log[5];
	if(times > test_count){
		adc_value = SDI_get_adc_value();
		test_count = times + 20;

		buf_log[0] = 0xAA;
		buf_log[1] = 0xBB;
		buf_log[2] = (adc_value >> 8) & 0xFF;
		buf_log[3] = (adc_value >> 0) & 0xFF;
		sdiProfile_SetParameter(0, 4, buf_log);

	}	
}

#else

	static unsigned long bat_times_get_adc_count = 0;

	if(times >= bat_times_get_adc_count + 10){			
		unsigned short adc_value;
		unsigned char i;
			
		bat_times_get_adc_count = times;
		adc_value = SDI_get_adc_value();

		if(adc_value == 0xFFFF) return;
		
		if(!g_bat_times_count){
			g_bat_buf[g_bat_times_count] = adc_value;
		}else{
			for(i=g_bat_times_count; i>0; i--){
				if(g_bat_buf[i-1] > adc_value){
					g_bat_buf[i] = g_bat_buf[i-1];
				}else{
					break;
				}
			}
			g_bat_buf[i] = adc_value; 
		}

		if(++g_bat_times_count >= 10){
			unsigned char level = 0;
			

			g_adv_val = g_bat_buf[6];
			for(level=1; level<BAT_LEVEL_MAX; level++){
				if(g_adv_val < battery_level_table[level])
					break;
			}
			g_bat_times_count = 0;

			if(level < g_bat_level) g_bat_level = level;
			g_adv_val = 0;
		}			
	}
#endif	
}

uint64_t sys_time_tick_second = 0;
unsigned long SDI_BLE_GetCurTimes(void){
	return sys_time_tick_second;
}

extern void SDI_Watchdog_clear(void);
void SDI_handle_process(unsigned long times){	

//	if(SDI_OTA_GetStatus() == SDI_OTA_END_STATE) 
//		return;
	
	static unsigned char msa_init_flag = 0;
	static uint32_t count_flag = 0;

	if(++count_flag == 20){
		count_flag = 0;
		sys_time_tick_second++;
		//SDI_Watchdog_clear();
	}

	if(!SDI_OTA_GetStatus())
		bat_process_test(times);

	if(SDI_FISH_IDLE == g_fish_state){		

	}else if(SDI_FISH_INIT == g_fish_state){	

		if(times > fish_time_count){
			SDI_I2C_init(1);
			msa_init_flag = 0;
			fish_time_count = 0;
			g_fish_state = SDI_FISH_INITING;
		}

	}else if(SDI_FISH_INITING == g_fish_state){ 	
		
		if(times > fish_time_count){
			if(!msa_init_flag){
				
				if(!MSA_init(0)){
					msa_init_flag = 1;
					fish_time_count = times + 2;
				}else{
					static unsigned char msa_init_err_flag = 0;
					SDI_I2C_init(0);			

					msa_init_err_flag = msa_init_err_flag ? 0: 1;
					SDI_led_indication(msa_init_err_flag, 0, 0);

					fish_time_count = times + 5;	
				}
			}else if(msa_init_flag){
				MSA_init(1);
				
				//SDI_led_indication(SDI_LED_ON, 2, 200);
				g_fish_state = SDI_FISH_NORMAL;
				fish_time_count = times + 2;

				fish_conn_tick_count = 0;
			}	
		}
	}else if(SDI_FISH_NORMAL == g_fish_state){

		if(SDI_OTA_Process(sys_time_tick_second)){
			SDI_led_indication(((sys_time_tick_second & 1) ? 1: 0), 0, 0);
			
		}else{
			SDI_get_msa_data(times);

			fish_conn_tick_count++;
			if(fish_conn_tick_count > FISH_CONN_TICK_TIMEOUT){
				SDI_TerminateConnection();
				fish_conn_tick_count = 0;
			}		
		}
		
	}else if(SDI_FISH_DISABLE == g_fish_state){

		SDI_I2C_init(0);
		//...disable ota
		g_fish_state = SDI_FISH_IDLE;
	}

}


