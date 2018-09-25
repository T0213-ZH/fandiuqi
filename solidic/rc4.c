/**
* @file     rc4.c
* @brief    RC4º”√‹À„∑®
* @author   zhihua.tao
*
* @date     2017-04.27
* @version  1.0.0
* @copyright www.solidic.net
*/

//#include <stdio.h>

/* encryption/decryption key */
#define S_BOX_LEN   (16)   //2^x
#define S_BOX_MASK  (0x0F) //(S_BOX_LEN - 1)
#define KEY_LEN     (3)
static unsigned char Key[KEY_LEN]= {0xFF, 0xFF, 0xFF};
static unsigned char S_box[S_BOX_LEN]={0};

static unsigned char g_rc4_init_complete = 0;

/**
 @func   rc4_int
 @desc   initization interface
 @param  
		S 
		Key
		Len      
 @return 
*/
static void rc4_int(unsigned char *S, unsigned char const *Key, unsigned int Len)
{
	unsigned int i=0,j=0;
	unsigned char k[S_BOX_LEN]={0};
	unsigned char temp = 0;
	
	for(i=0; i<S_BOX_LEN; i++)
	{
		S[i] = i;
		k[i] = Key[i%Len];
	}	
	for(i=0; i<S_BOX_LEN; i++)
	{
		j = (j+S[i]+k[i]) & S_BOX_MASK;
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}
}

/**
 @func   rc4_crypt
 @desc   encryption/decryption interface
 @param  
		s_box_ptr
		Data
		Len
 @return 
*/
static void rc4_crypt(unsigned char const *s_box_ptr, unsigned char *Data, unsigned int Len)
{
	unsigned int i=0,j=0,t=0;
	unsigned int k=0;
	unsigned char temp;
	unsigned char S[S_BOX_LEN] = {0};
	
	for(k=0; k<S_BOX_LEN; k++){
		S[k]= *s_box_ptr++;
	}
	
	for(k=0; k<Len; k++)
	{
		i = (i+1) & S_BOX_MASK;
		j = (j+S[i]) & S_BOX_MASK;
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
		t = (S[i]+S[j]) & S_BOX_MASK;
		Data[k]^=S[t];	
	}
}

/**
 @func   SDI_RC4_crypt
 @desc   encryption/decryption interface
 @param  
		Data
		len
 @return 
*/

void SDI_RC4_crypt(unsigned char *ptr, unsigned int len){

	if(g_rc4_init_complete == 0){

		if((Key[0] == 0xFF) && (Key[1] == 0xFF) && (Key[2] == 0xFF) /*&& (Key[3] == 0xFF) && (Key[4] == 0xFF) && (Key[5] == 0xFF)*/) 
			return;
		rc4_int(S_box, Key, KEY_LEN);
		g_rc4_init_complete = 1;
	}

	if(g_rc4_init_complete)
		rc4_crypt(S_box, ptr, len);
}
/**
 @func   SDI_RC4_SetKey
 @desc   seting rc4 key value
 @param  
		key
 @return 
*/

void SDI_RC4_SetKey(unsigned char *key){

	Key[0] = *key++;
	Key[1] = *key++;
	Key[2] = *key;
//	Key[3] = *key++;
//	Key[4] = *key++;
//	Key[5] = *key++;
}

/**
 @func  main
 @desc  test code flow 
 @param 
*/
//int main(void){
//	unsigned char temp_key[]={0x12, 0x34, 0x56, 0x78, 0x90, 0x10};
//	unsigned char temp_str[]="0123456789";
//	unsigned char i, len;
//	
//	printf("process string: %s\n", temp_str);
//	SDI_RC4_SetKey(temp_key);
//	
//	printf("encryption string\n");
//	SDI_RC4_crypt(temp_str, sizeof(temp_str));
//	
//	len = sizeof(temp_str);
//	for(i=0; i<len; i++)
//		printf("0x%x ", temp_str[i]);
//	
//	printf("\ndecryption string\n");
//	SDI_RC4_crypt(temp_str, sizeof(temp_str));
//	for(i=0; i<len; i++)
//		printf("0x%x ", temp_str[i]);
//	
//	return 0;
//}

