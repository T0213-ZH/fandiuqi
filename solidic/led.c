#include <ti/sysbios/knl/Clock.h>
#include "util.h"
#include "../source/ti/blestack/boards/CC2640R2_LAUNCHXL/CC2640R2_LAUNCHXL.h"

#include "led.h"


static unsigned char g_led_count = 0;
static Clock_Struct g_led_period_clock;

static void SDI_led_clockHandler(UArg arg)
{
  // Wake up the application.
  //Event_post(syncEvent, arg);

   led_gpio_toggle();
	g_led_count--; 
	if(!g_led_count){
		Util_stopClock(&g_led_period_clock);
	}
}

void SDI_led_indication(unsigned char status, unsigned char count, int period_ms){

	static unsigned char led_ctr_init_flag = 0;

	if(!led_ctr_init_flag){
		Util_constructClock(&g_led_period_clock, SDI_led_clockHandler,
	                      period_ms, period_ms, false, NULL);
		led_ctr_init_flag = 1;
	}
	
	if(count){		
		Util_restartClock(&g_led_period_clock, period_ms);
		g_led_count = count << 1;
	}else{
		Util_stopClock(&g_led_period_clock);
	}	
	led_gpio_set(status);
}

