/*parser.c
解析器
*/
#include <stdint.h>
#include "parser.h"
	uint16_t add=0;
/*******************************************************
函数名:校验Ascii字符是否能表示hex
参  数:
返  回:
*******************************************************/
int8_t checkascii(uint8_t ch)
{
	int8_t ret = FAILED; 

	add++;
	if(ch>='0' && ch <= '9')
		return (ch - '0');
	else if(ch >= 'a' && ch <= 'f')
		return (ch - 'a'+ 10);
	else if(ch >= 'A' && ch <= 'F')
		return (ch - 'A'+ 10);
	return ret;
}
/*******************************************************
函数名:ascii格式字符串转化为hex字符串
参  数:-*pdata，ascii字符串
       -len,转换后的字节数 
返  回:-ERR，转换失败
			 -TRUE，转换成功，数据存放pdata中
*******************************************************/
int8_t ascii2hex(uint8_t * pdata,uint8_t len)
{
	int8_t tmp;
	uint8_t hex;
	uint8_t i = 0;
	uint8_t * pret;
	
	pret = pdata;
	while(len)
	{
		tmp = *pdata;
		if(i % 2 == 0)
		{	tmp = checkascii(tmp);
			if(tmp != FAILED)
			{
				hex = tmp << 4;
			}
			else
				return FAILED;
		}
		else
		{
			tmp =checkascii(tmp);
			if(tmp != FAILED)
			{
				hex |= tmp;
				*pret = hex;
				pret ++;
				len --;
			}
			else
				return FAILED;
		}
		i++;
		pdata ++;
	}
	return SUCCESSED;
}


/*******************************************************
函数名:数据校验和检查
参  数:-*pdata，16进制字符串
       -len,传送长度，最后一个字节为待校准的校验和数值 
返  回:-ERR，校验正确
			 -TRUE，转换成功，数据存放pdata中
*******************************************************/
int8_t checksum(uint8_t * pdata,uint8_t len)
{
	uint8_t sum = 0;
	uint8_t i;
	//边界检查
	if(len > 200)
		return FAILED;
	for(i = 0;i < len -1;i++)
	{
		sum += *(pdata + i);
	}
	
	sum = ~sum + 1;
	
	if(sum == pdata[len - 1])
		return SUCCESSED;
	else
		return FAILED;
}

uint8_t getsum(uint8_t * pdata,uint8_t len)
{
	uint8_t sum = 0;
	uint8_t i;
	//边界检查
	if(len > 200)
		return 0xFF;
	for(i = 0;i < len -1;i++)
	{
		sum += *(pdata + i);
	}
	
	sum = ~sum + 1;
	
	return sum;
}
/*******************************************************
函数名:数字异或对比
参  数:-*pdata，16进制字符串
       -len,传送长度，最后一个字节为待校准的校验和数值
			 -xor,待对比值
返  回:-ERR，校验失败
			 -TRUE，校验成功
*******************************************************/
int8_t checkxor(uint8_t *pdata,uint8_t len)
{
	uint8_t i,temp,chk;
	
	temp = 0;
	if(len < 2)
		return FAILED;
	chk = *pdata;
	pdata++;
	for(i = 0;i < len - 1;i++)
	{
		temp ^= (*pdata ++);
	}
	
	if(chk == temp)
		return SUCCESSED;
	else
		return FAILED;
}
/*******************************************************
函数名:获取字符串长度
参  数:
返  回:
*******************************************************/
uint8_t get_string_len(uint8_t *pstr)
{
	uint8_t i;
	for(i = 0;i < 20;i++)
	{
		if((pstr[i]) == '\0')
		{
			break;
		}
	}
	if(i == 20)
		pstr[19] = '\0';
	return i;
}
////////////////////////
