#include "sdi_ble.h"
#include "msa300.h"

#define MSA_DEVICE_ID_ADDR              (0x4C)
#define MSA_ACC_RANG                    (0xFFFF)//(0x3FFF)

#define msa_i2c_read(reg)          SDI_I2C_read(MSA_DEVICE_ID_ADDR, reg)
#define msa_i2c_write(reg, value)  SDI_I2C_write(MSA_DEVICE_ID_ADDR, reg, value)

//g_acc_val[0]  = msa_i2c_read(MSA_REG_ACC_X_LSB);
//g_acc_val[0] |= msa_i2c_read(MSA_REG_ACC_X_MSB) << 8;
//if(g_acc_val[0] & 0x8000) g_acc_val[0] = MSA_ACC_RANG - g_acc_val[0] + 1; 	
//	
//g_acc_val[1]  = msa_i2c_read(MSA_REG_ACC_Y_LSB);
//g_acc_val[1] |= msa_i2c_read(MSA_REG_ACC_Y_MSB) << 8;
//if(g_acc_val[1] & 0x8000) g_acc_val[1] = MSA_ACC_RANG - g_acc_val[1] + 1; 
//		
//g_acc_val[2]  = msa_i2c_read(MSA_REG_ACC_Z_LSB);
//g_acc_val[2]  |= msa_i2c_read(MSA_REG_ACC_Z_MSB) << 8;
//if(g_acc_val[2]  & 0x8000) g_acc_val[2]  = MSA_ACC_RANG - g_acc_val[2]  + 1;

unsigned char msa_read_acc(void){

	extern int g_acc_val[];
	unsigned char ret = 0;
	unsigned int x, y, z;

	msa_i2c_write(MSA_REG_POWERMODE_BW, 0x0A);
	//NULL FOR Delay swap to work mode
	msa_i2c_read(MSA_REG_ACC_X_LSB);
	msa_i2c_read(MSA_REG_ACC_X_MSB);
		
	x  = msa_i2c_read(MSA_REG_ACC_X_LSB);
	x |= msa_i2c_read(MSA_REG_ACC_X_MSB) << 8;
	if(x & 0x8000) g_acc_val[0] = MSA_ACC_RANG - x + 1; else g_acc_val[0] = x;		
		
	y  = msa_i2c_read(MSA_REG_ACC_Y_LSB);
	y |= msa_i2c_read(MSA_REG_ACC_Y_MSB) << 8;
	if(y & 0x8000) g_acc_val[1] = MSA_ACC_RANG - y + 1; else g_acc_val[1] = y;	
			
	z  = msa_i2c_read(MSA_REG_ACC_Z_LSB);
	z |= msa_i2c_read(MSA_REG_ACC_Z_MSB) << 8;
	if(z & 0x8000) g_acc_val[2]  = MSA_ACC_RANG - z  + 1; else g_acc_val[2] = z;

	msa_i2c_write(MSA_REG_POWERMODE_BW, 0xCA);

	if(((x < 8000) || (x > 55000)) &&
		((y > 42000) && (y < 55000)) &&
		((x < 4000) || (x > 55000)) ) 
		ret = 1;

	return ret;
}

int MSA_init(unsigned char type){

	if(!type){
		unsigned char value;

		//读MSA300 ID
		value = msa_i2c_read(MSA_REG_WHO_AM_I);
		if(0x13 != value) return -1;

		//软件复位
		//msa_reg_mask_write(MSA_REG_SPI_I2C, 0x24, 0x24);
		msa_i2c_write(MSA_REG_SPI_I2C, 0x24);

	}else{	
//		//设置POWER MODE
		msa_i2c_write(MSA_REG_POWERMODE_BW, 0x0A & 0xDE);

//		//设置量程 lkk modify  0x10, 14BIT, +/-2g
//		msa_reg_mask_write(MSA_REG_G_RANGE, 0x0F, 0x00);

//		msa_i2c_read(MSA_REG_SWAP_POLARITY);
		msa_i2c_write(MSA_REG_SWAP_POLARITY, 0x06);
	}	

	return 0;
}


