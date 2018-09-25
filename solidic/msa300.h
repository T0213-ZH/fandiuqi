#ifndef __MSA300_H
#define __MSA300_H

#define MSA_REG_SPI_I2C                 0x00
#define MSA_REG_WHO_AM_I                0x01

#define MSA_REG_ACC_X_LSB               0x02
#define MSA_REG_ACC_X_MSB               0x03
#define MSA_REG_ACC_Y_LSB               0x04
#define MSA_REG_ACC_Y_MSB               0x05
#define MSA_REG_ACC_Z_LSB               0x06
#define MSA_REG_ACC_Z_MSB               0x07

#define MSA_REG_MOTION_FLAG				0x09
#define MSA_REG_G_RANGE                 0x0f
#define MSA_REG_ODR_AXIS_DISABLE        0x10
#define MSA_REG_POWERMODE_BW            0x11
#define MSA_REG_SWAP_POLARITY           0x12
#define MSA_REG_FIFO_CTRL               0x14
#define MSA_REG_INTERRUPT_SETTINGS1     0x16
#define MSA_REG_INTERRUPT_SETTINGS2     0x17
#define MSA_REG_INTERRUPT_MAPPING1      0x19
#define MSA_REG_INTERRUPT_MAPPING2      0x1a
#define MSA_REG_INTERRUPT_MAPPING3      0x1b
#define MSA_REG_INT_PIN_CONFIG          0x20
#define MSA_REG_INT_LATCH               0x21
#define MSA_REG_ACTIVE_DURATION         0x27
#define MSA_REG_ACTIVE_THRESHOLD        0x28
#define MSA_REG_TAP_DURATION            0x2A
#define MSA_REG_TAP_THRESHOLD           0x2B
#define MSA_REG_CUSTOM_OFFSET_X         0x38
#define MSA_REG_CUSTOM_OFFSET_Y         0x39
#define MSA_REG_CUSTOM_OFFSET_Z         0x3a
#define MSA_REG_ENGINEERING_MODE        0x7f
#define MSA_REG_SENSITIVITY_TRIM_X      0x80
#define MSA_REG_SENSITIVITY_TRIM_Y      0x81
#define MSA_REG_SENSITIVITY_TRIM_Z      0x82
#define MSA_REG_COARSE_OFFSET_TRIM_X    0x83
#define MSA_REG_COARSE_OFFSET_TRIM_Y    0x84
#define MSA_REG_COARSE_OFFSET_TRIM_Z    0x85
#define MSA_REG_FINE_OFFSET_TRIM_X      0x86
#define MSA_REG_FINE_OFFSET_TRIM_Y      0x87
#define MSA_REG_FINE_OFFSET_TRIM_Z      0x88
#define MSA_REG_SENS_COMP               0x8c
#define MSA_REG_SENS_COARSE_TRIM        0xd1

extern int MSA_init(unsigned char type);
extern void MSA_disable(void);
extern unsigned char msa_read_acc(void);
//extern void msa_get_acc(short *x, short *y, short *z);
extern void msa_test_func(void);

#endif

