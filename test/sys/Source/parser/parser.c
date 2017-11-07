/*parser.c
������
*/
#include <stdint.h>
#include "parser.h"
	uint16_t add=0;
/*******************************************************
������:У��Ascii�ַ��Ƿ��ܱ�ʾhex
��  ��:
��  ��:
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
������:ascii��ʽ�ַ���ת��Ϊhex�ַ���
��  ��:-*pdata��ascii�ַ���
       -len,ת������ֽ��� 
��  ��:-ERR��ת��ʧ��
			 -TRUE��ת���ɹ������ݴ��pdata��
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
������:����У��ͼ��
��  ��:-*pdata��16�����ַ���
       -len,���ͳ��ȣ����һ���ֽ�Ϊ��У׼��У�����ֵ 
��  ��:-ERR��У����ȷ
			 -TRUE��ת���ɹ������ݴ��pdata��
*******************************************************/
int8_t checksum(uint8_t * pdata,uint8_t len)
{
	uint8_t sum = 0;
	uint8_t i;
	//�߽���
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
	//�߽���
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
������:�������Ա�
��  ��:-*pdata��16�����ַ���
       -len,���ͳ��ȣ����һ���ֽ�Ϊ��У׼��У�����ֵ
			 -xor,���Ա�ֵ
��  ��:-ERR��У��ʧ��
			 -TRUE��У��ɹ�
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
������:��ȡ�ַ�������
��  ��:
��  ��:
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
