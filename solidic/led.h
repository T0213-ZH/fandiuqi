#ifndef __LED_H
#define __LED_H

#define led_gpio_set(x)   GPIO_writeDio(CC2640R2_SDI_PIN_LED, x)
#define led_gpio_toggle() GPIO_toggleDio(CC2640R2_SDI_PIN_LED)

#define SDI_LED_ON   (1)
#define SDI_LED_OFF  (0)

extern void SDI_led_indication(unsigned char status, unsigned char count, int period_ms);

#endif

