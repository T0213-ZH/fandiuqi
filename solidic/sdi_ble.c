#include "sdi_ble.h"
#include "rc4.h"
#include "crc16.h"
#include "stdlib.h"
#include "led.h"
#include "battery.h"
#include "msa300.h"


#include <ti/sysbios/knl/Clock.h>
#include "util.h"
#include "../source/ti/blestack/boards/CC2640R2_LAUNCHXL/CC2640R2_LAUNCHXL.h"
//#include "../source/ti/blestack/profiles/roles/cc26xx/peripheral.h"
//#include "../source/ti/blestack/inc/bcomdef.h"

struct fish_config{
	unsigned char nID;                /*!< 设备ID         */
	unsigned char light_en;           /*!< 亮灯控制       */
	unsigned char light_flash;        /*!< 闪灯控制       */
	unsigned char bite_threshold;     /*!< 灵敏度调节     */
	unsigned char light_brightness;   /*!< 亮度控制       */
	unsigned char raw_data;           /*!< 传感器原始数据 */
};
struct fish_config g_fish_config = {0, 1, 1, 5, 0, 0};

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


extern unsigned short SDI_get_adc_value(void);
extern void SDI_TerminateConnection(void);

static void SDI_fish_send_data_to_app(unsigned char *ptr, unsigned char len){

	if(g_connect_status){
		//if(*ptr != FISH_EVT_VERSION){
		//	SDI_RC4_crypt(&ptr[2], ptr[1]);
		//}

		SDI_send_data_to_app(ptr, len);
	}
}

extern unsigned long times_tick_count;
void SDI_connection_ind(unsigned char type){

	g_connect_status = type;
	g_bat_times_count = 0;
	g_adv_val = 0;

	if(g_connect_status){ //已连接
		g_fish_state = SDI_FISH_INIT;

	}else{//断开连接
		g_fish_state = SDI_FISH_DISABLE;
	}	

	SDI_led_indication(SDI_LED_ON, 2, 200);
}

static void SDI_set_device_param(unsigned char *ptr, unsigned char length){

	if(length != 0x05) return;

	g_fish_config.nID = *ptr;
	g_fish_config.light_en = *(ptr + 1);
	g_fish_config.light_flash = *(ptr + 2);
	g_fish_config.bite_threshold = *(ptr + 3);
	g_fish_config.light_brightness = *(ptr + 4);

	SDI_led_indication(g_fish_config.light_en, 0, 0); //更新指示灯状态
}

static void SDI_set_ram_mode(unsigned char *ptr, unsigned char len){
	
	if(len != 0x02) return;

	g_fish_config.raw_data = *(ptr+1);
}

static void SDI_get_device_info(void){
	struct fish_data_format packet;

	packet.id = FISH_EVT_SET_PARAM;
	packet.length = 0x05;

	__memcpy(&packet.payload, &g_fish_config, packet.length);

	SDI_fish_send_data_to_app((void *)&packet, packet.length + 2);

	fish_conn_tick_count = 0;
}

static void SDI_change_device_name(unsigned char *ptr, unsigned char len){

	
}

static void SDI_get_firmware_version(void){
	struct fish_data_format packet;

	//.... 延迟断开连接

	packet.id = FISH_EVT_VERSION;
	packet.length = 0x03;

	packet.payload[0] = g_fish_config.nID;
	packet.payload[1] = (FIRMWARE_VER >> 4) & 0x0F; //固件版本 
	packet.payload[2] = (FIRMWARE_VER >> 0) & 0x0F; 	

	SDI_fish_send_data_to_app((void *)&packet, packet.length + 2);
}

void SDI_ble_data_parse(unsigned char *ptr, unsigned int len){

	struct fish_data_format *packet = (void *)ptr;

//	SDI_fish_send_data_to_app(ptr, len);
//	return;

	if(packet->id != FISH_CMD_FW_VER){

		SDI_RC4_crypt((unsigned char *)&packet->payload, packet->length);

		switch(packet->id){
			case FISH_CMD_SETTING:      SDI_set_device_param((unsigned char *)&packet->payload, packet->length);  break;
			case FISH_CMD_RAM_DATA:     SDI_set_ram_mode((unsigned char *)&packet->payload, packet->length);      break;
			case FISH_CMD_DEVICES_INFO: SDI_get_device_info(); break;
			case FISH_CMD_NIGHT_MODE:   break;
			case FISH_CMD_DEVICE_NAME:  SDI_change_device_name((unsigned char *)&packet->payload, packet->length);break;
			default: break;
		}
	}else{
		SDI_get_firmware_version();
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

unsigned char SDI_get_msa_data(unsigned long times){

	struct fish_data_format packet;
	static unsigned long time_count = 0;

	g_acc_val_old[0] = g_acc_val[0];		
	g_acc_val_old[1] = g_acc_val[1];	
	g_acc_val_old[2] = g_acc_val[2];

	msa_read_acc();

	if(!g_fish_config.raw_data){
//	if(0){	
		if(fish(g_fish_config.bite_threshold)){
			packet.id = FISH_EVT_BITE;
			packet.length = 0x02;
			packet.payload[0] = g_fish_config.nID;
			packet.payload[1] = 0x01; //固件版本 


			if(times > time_count){
				time_count = times + 30;
				SDI_fish_send_data_to_app((void *)&packet, packet.length + 2);
					
				if(g_fish_config.light_flash){
					SDI_led_indication(g_fish_config.light_en, 4, 200);
				}
			}	
		}
	}else{
		unsigned char len = 0;

		packet.payload[len++] = g_fish_config.nID;
						
		packet.payload[len++] = (g_acc_val[0] >> 8) & 0xFF;
		packet.payload[len++] = (g_acc_val[0] >> 0) & 0xFF;
		
		packet.payload[len++] = (g_acc_val[1] >> 8) & 0xFF;
		packet.payload[len++] = (g_acc_val[1] >> 0) & 0xFF;
					
		packet.payload[len++] = (g_acc_val[2] >> 8) & 0xFF;
		packet.payload[len++] = (g_acc_val[2] >> 0) & 0xFF;

		packet.payload[len++] = 2;//bat_level
		packet.payload[len++] = 14;//temperature

		packet.id = FISH_EVT_TEST_DATA;
		packet.length = len;

		SDI_fish_send_data_to_app((void *)&packet, len + 2);
	}
        return 0;
}


#define BAT_LEVEL_MAX (4)
const unsigned short battery_level_table[BAT_LEVEL_MAX]={
	0x1B0,//0x01F0, //2.4
	0x1F0,//0x021E, //2.6
	0x21E,//0x0247, //2.8
	0x0240, //3.0
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
			struct fish_data_format packet;
			unsigned char len = 0;
			unsigned char level = 0;
			

			g_adv_val = g_bat_buf[6];
			for(level=1; level<BAT_LEVEL_MAX; level++){
				if(g_adv_val < battery_level_table[level])
					break;
			}
			g_bat_times_count = 0;

			if(level < g_bat_level) g_bat_level = level;

			packet.id = 0x03;
			packet.length = 0x03;
			packet.payload[len++] = g_fish_config.nID;		
			packet.payload[len++] = g_bat_level;//on_vcc_complete();;
			packet.payload[len++] = 14;//get_temperature();
			//packet.payload[len++] = (g_adv_val >> 8) & 0xFF;//on_vcc_complete();
			//packet.payload[len++] = (g_adv_val >> 0) & 0xFF;//get_temperature();				

			SDI_fish_send_data_to_app((void *)&packet, len + 2);
			g_adv_val = 0;
		}			
	}
#endif	
}

void SDI_handle_process(unsigned long times){	

	static unsigned char msa_init_flag = 0;

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
				
				SDI_led_indication(SDI_LED_ON, 2, 200);
				g_fish_state = SDI_FISH_NORMAL;
				fish_time_count = times + 2;

				fish_conn_tick_count = 0;
			}	
		}
	}else if(SDI_FISH_NORMAL == g_fish_state){


		SDI_get_msa_data(times);

		fish_conn_tick_count++;
		if(fish_conn_tick_count > FISH_CONN_TICK_TIMEOUT){
			SDI_TerminateConnection();
			fish_conn_tick_count = 0;
		}
	}else if(SDI_FISH_DISABLE == g_fish_state){
	
		SDI_I2C_init(0);
		//...disable ota
		g_fish_state = SDI_FISH_IDLE;
	}
}


