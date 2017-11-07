#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "radio_config.h"
#include "led.h"
#include "simple_uart.h"
#include "uart.h"
#include "string.h"
#include "nrf_nvmc.h"
#include "nrf_delay.h"
#include "time.h"
#include "local_para.h"
#include "crc16.h"
#include "variable.h"
#include "com.h"
#include "nrf51_bitfields.h"
#include "parser.h"
void version(void);
uint8_t update_finished = 0;
//----------------------------
uint8_t error_code;

uint32_t code_start;
uint16_t code_extent;
uint8_t lineaddr_valid = false;
uint16_t line_count = 0;                          //��������
uint16_t getvaliddata=0;
uint16_t data_error=0;
uint16_t get_error_data=0;
uint16_t flash_add=0;
int8_t stat;//����������
extern uint8_t boot_flag;
extern uint8_t rec_flag;

extern uint16_t com_delay;
extern uint8_t test;
extern uint16_t crc_delay;


uint8_t Source_Addr = 0;													//Դ��ַ����

uint8_t Count_idx;																//С�����
uint8_t cmd_idx;																	//�����л�����������
uint16_t last_idx;																//��¼��һ�εĻ��������������ڶ�������


#define IDX_DES_ADDR1		1
#define IDX_DES_ADDR2		3
#define IDX_SOR_ADDR1		5
#define IDX_SOR_ADDR2		7
#define IDX_SRL					9
#define IDX_LEN					10
#define IDX_SRL_LINE		11
#define IDX_DATA	13


//Э��������
#define CMD_UPDATE_TRIGGER	0x09						//��������
#define CMD_UPDATE_HEX			0x0A						//�������ݰ�
#define CMD_UP_REPEAT				0x8B						//���У������ط�
#define CMD_GET_VERSION			0x0C						//��ȡ�汾��
#define CMD_OPEN_UPDATE			0x0D						//������������

#define CMD_UP_Msk					0x80						//�ϱ���������


/*����HEX�ļ�*/

#define	HEX_LEN			1
#define HEX_ADDR		2
#define HEX_TYPE		4
#define HEX_DATA		5





uint8_t business_step = STEP_TRIGGER;


/*******************************************************
������:У��hex��ʽ�ļ��������µ������ݵ�Ƭ��Flash
��  ��:-*pcode��Ascii��ʾ�������ݣ���У����
       -len,���ͳ��ȣ����һ���ֽ�Ϊ��У׼��У�����ֵ
				����0D 0A
��  ��:-FAILED��У��ʧ��
			 -SUCCESSED�����´���ɹ�
*******************************************************/

WRITE_HEX hex_write(HEX_buff pdata,uint8_t len)
{
	static uint8_t jumpbuf[4];
	WRITE_HEX rat;
	uint16_t tempaddr,wtemp;
	uint32_t wtempaddr;
	if(*pdata->Hex_handle_buff == NULL)
			{	
	return WRITE_FAILED_1;}
	if(pdata->Hex_handle_buff[0] != ':')
			{	
	return WRITE_FAILED_1;}
	len --;
	pdata->count++;
	if(len > 8)
	{
		if(pdata->Hex_handle_buff[len - 1] == 0x0D && pdata->Hex_handle_buff[len] == 0x0A)
		{ len =len- 2;}
	}else
			{	
	return WRITE_FAILED_1;}
	
	len /= 2;
	if(ascii2hex(pdata->Hex_handle_buff+1,len) == FAILED)																		//asciiת��Ϊʮ������
	{	
	return WRITE_FAILED_1;}
	//���ݰ�����
	if(checksum(pdata->Hex_handle_buff+1,len) == FAILED)																			//У��
	{
		
	return WRITE_FAILED_1;}
	
	rat = 0;
	
	switch(pdata->Hex_handle_buff[HEX_TYPE])
	{
		case  0x01:
					if(pdata->Hex_handle_buff[HEX_LEN] == 0 && pdata->Hex_handle_buff[HEX_DATA] == 0xff)
			{	
			//	pdata->count=0;
				rat = WRITE_SUCCESS_1;																									
				nrf_nvmc_write_bytes(APP_BEGIN,jumpbuf,4);										
				update_finished = 1;
			}
			break;
			case 0x02:
			break;
		case 0x03:
			break;
		case 0x04:
			if(pdata->Hex_handle_buff[HEX_LEN] == 0x02)
			{
			pdata->Code_extent  = (pdata->Hex_handle_buff[HEX_DATA] << 16) + pdata->Hex_handle_buff[HEX_DATA+1];
				pdata->Lineaddr_valid = 1;
	  	rat = WRITE_SUCCESS_1;	
			}else{
				pdata->Lineaddr_valid = 0;
			}
			break;
		case 0x05:
			rat = WRITE_SUCCESS_1;
			break;
		case 0x00:
			if(pdata->Lineaddr_valid)
			{
				
				tempaddr = (pdata->Hex_handle_buff[HEX_ADDR] <<8)+pdata->Hex_handle_buff[HEX_ADDR+1];
				wtemp = pdata->Hex_handle_buff[HEX_LEN];
				wtempaddr = ((uint32_t)pdata->Code_extent << 16) + tempaddr;
				
				if(((wtempaddr + wtemp) > (APP_CODE_END + 1)) || (wtempaddr < APP_CODE_BEGIN))
				{	
					rat = WRITE_FAILED_1;
					break;
				}
				
				if(wtempaddr == APP_BEGIN && wtemp > 8)
				{
					memcpy(jumpbuf,pdata->Hex_handle_buff+HEX_DATA,4);
					memset(pdata->Hex_handle_buff+HEX_DATA,0xff,4);
				}
				flash_add++;
					rat = WRITE_SUCCESS_1;	
				nrf_nvmc_write_bytes(wtempaddr,pdata->Hex_handle_buff+HEX_DATA,pdata->Hex_handle_buff[HEX_LEN]);

			}
			break;
		default:
			break;
	}
	pdata->count=0;
		return rat;
}


/*******************************************************
������:����Ӧ�ó���������
��  ��:�� 
��  ��:-FAILED������ʧ�ܣ��̶�Ӧ�ó����С4K bytes
			 -SUCCESSED�������ɹ�
*******************************************************/
int8_t app_code_erase(void)
{
	uint32_t page_addr = 0;
	uint16_t i;
	uint32_t page_size = NRF_FICR->CODEPAGESIZE;
  uint32_t code_size = NRF_FICR->CODESIZE;
	
	page_addr = APP_CODE_BEGIN;
	
	for(i = (uint16_t)((code_size) - APP_CODE_BEGIN/page_size);i>0;i--)
	{
		if(page_addr >= APP_CODE_BEGIN && page_addr < APP_CODE_END)
		{
			nrf_nvmc_page_erase(page_addr);
		}
		page_addr += page_size;
	}
	
	return 0;
}
/*******************************************************
������:�ͷ�boot����ռ�õ���Դ
��  ��:��
��  ��:��
*******************************************************/
void release_bootSource(void)
{
		IRQn_Type i;

	NVIC_DisableIRQ( TIMER0_IRQn );
	NRF_TIMER0 ->TASKS_STOP = 1;
	NRF_TIMER0 ->TASKS_CLEAR = 1;
	NRF_TIMER0 -> INTENCLR 			= 0xFFFFFFFF;
	
	NVIC_DisableIRQ( UART0_IRQn );
	NRF_UART0->TASKS_STOPRX    = 1;
  NRF_UART0->TASKS_STOPTX    = 1;
	NRF_UART0->EVENTS_RXDRDY    = 0;
	NRF_UART0->INTENCLR					= 0xFFFFFFFF;
	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
	//ȷ�������ж��ڽ���APP֮ǰ���ǹر�״̬
		for(i = POWER_CLOCK_IRQn;i <= SWI5_IRQn;i++)
		NVIC_DisableIRQ( i );
}
/*******************************************************
������:����Ӧ�ó����ջ��Ч�ԣ�����ת
��  ��:��
��  ��:-0��ʼ�յ��ú�������ջ��ַ��Ч��ʵ����ת��
*******************************************************/
int8_t app_code_jump(void)
{
		if(((*(uint32_t *)APP_CODE_BEGIN)& 0x2FFE0000)==0x20000000)
	{
		if(AppFlag == __ON_BOOT)//Ӧ�ó���ӹ�ʱ��ֱ����ת
		{
		nrf_delay_ms(200);
		release_bootSource();
		}
		(*(void (*)(void))(*(uint32_t*)APP_CODE_EXE))();//00 01 a1 05
	}else
	{
    
	}
	return 0;
}

/*��ȡ��Ч����*/
uint8_t * GetValidData(void)
{
	uint16_t crc;
	uint16_t len;
		
		if(rcv_ok == 1)
		{
					
				//		crc_timer();  //������жϿ�ʼ��ʱ 
                          //��ʱ���ϱ�Ҫ���ط�
			len = rcv_uart[FRAME_LEN+1];
			crc = ((rcv_uart[len + 10] & 0x00ff)<<8) | rcv_uart[len + 11];
				
			if(crc16(rcv_uart + FRAME_ID,len + 8) != crc)
			{
				
				return NULL;
			}
		
			if(memcmp(LogicAddr,rcv_uart+FRAME_ID,4) == 0)
			{
				getvaliddata++;
				return (rcv_uart);
			}
		}
	return NULL;
}


void Com_Response(uint8_t *p,uint8_t len)
{
	uint16_t crc;
	memcpy(tsm_Array,p,7);
	tsm_Array[FRAME_CMD] = p[FRAME_CMD] | CMD_UP_Msk;
	tsm_Array[FRAME_LEN] = 0;
	tsm_Array[FRAME_LEN + 1] = len;
	
	crc = crc16(tsm_Array + FRAME_ID,len+8);
	tsm_Array[FRAME_DATA + len] = (uint8_t)(crc>>8);
	tsm_Array[FRAME_DATA + len +1] = (uint8_t)crc;
  simple_uart_array(tsm_Array,len + FRAME_EXT_LEN);
}


void Com_Repeat(uint8_t *p,uint16_t line)
{
	uint8_t len = 2;
	uint16_t crc;
	memcpy(tsm_Array,p,7);
	tsm_Array[FRAME_CMD] = CMD_UP_REPEAT;
	tsm_Array[FRAME_LEN] = 0;
	tsm_Array[FRAME_LEN + 1] = len;
	tsm_Array[FRAME_DATA] = (uint8_t)(line >> 8);
	tsm_Array[FRAME_DATA+1]=(uint8_t)line;
	crc = crc16(tsm_Array+FRAME_ID,len+8);
	tsm_Array[FRAME_DATA + len] = (uint8_t)(crc>>8);
	tsm_Array[FRAME_DATA + len + 1] = (uint8_t)crc;
	data_error++;
	
	simple_uart_array(tsm_Array,2 + FRAME_EXT_LEN);
   

}

void get_NextUart(void)
{

	//���ھ���
	if(NRF_UART0->EVENTS_ERROR)
	{
		rcv_ok = 0;
	
		NRF_UART0->ERRORSRC = 1;
		NRF_UART0->EVENTS_ERROR = 0;
	}	
}
void clear_hex_write(HEX_buff pdata)
{
	uint16_t xx=0;
	for(xx=0;xx<pdata ->count;xx--) 
	pdata ->Hex_handle_buff [pdata ->count ]=0;
	
}


int8_t GetLine(uint8_t *p,uint8_t *pfrom,uint8_t len,uint8_t *rlen)
{
	uint8_t i;
	static uint8_t flag;//�н�����־
	//clear_hex_write(&linebuffer);
 	for(i = 0;(i + line_idx)<BUFFER_LEN;i++)
	{
		if(package_idx >= len){														//������δ����
			*rlen = i;
			return -2;																			//��Ҫ��һ��ƴ��
		}
		p[i+line_idx] = pfrom[package_idx];
		package_idx ++;
		if(p[i + line_idx] == 0x0d)
		{
			flag = 1;
		}else 
		if(p[i+line_idx] == 0x0a)
		{
			if(flag)
				{
				*rlen = i + 1;																//���س���
				return 0;																			//�ɴ�������
			}
		}
		else{
			flag = 0;
		}
	}
																										
	return -1;
}


void UpdateReady(uint8_t SS)
{
	uint16_t crc;
	app_code_erase();//����App����AA AA FE 00 00 00 SS 09 00 00 5a 5a
	business_step = STEP_GET_APP;
	line_count = 0;
	line_idx = 0;
	update_finished = 0;
	tsm_Array[FRAME_HEAD] = 0xAA;
	tsm_Array[FRAME_HEAD+1] = 0xAA;
	memcpy(tsm_Array+FRAME_ID,LogicAddr,4);//fffc0063
	tsm_Array[FRAME_SR] = SS;
	tsm_Array[FRAME_CMD] = 0x89;
	tsm_Array[FRAME_LEN] = 0x00;
	tsm_Array[FRAME_LEN + 1] = 0x00;

	crc = crc16(tsm_Array+FRAME_ID,8);
	tsm_Array[FRAME_DATA] = (uint8_t)(crc>>8);
	tsm_Array[FRAME_DATA + 1] = (uint8_t)crc;
	simple_uart_array(tsm_Array,12);
}



void com(void)
{
	uint8_t * pdata;
	uint8_t len,udlen,tt;
	

	if(boot_flag)
	{
		boot_flag = 0;
		app_code_jump();
		return;
	}
	
	pdata = GetValidData();
	if(pdata == NULL)																				
	{	
		//׼����һ��
		get_NextUart();
			return;
	}
	
	switch(pdata[FRAME_CMD])																							//������
	{
		
		case CMD_UPDATE_TRIGGER:
			stop_auto_timer();//�ر��Զ���תӦ�ó���ļ�ʱ��
			app_code_erase();																								//����App��
			Com_Response(pdata,0);
			business_step = STEP_GET_APP;
			line_count = 0;
			line_idx = 0;
			update_finished = 0;
			break;
		case CMD_UPDATE_HEX:
			if(business_step != STEP_GET_APP)
				break;
			
			stop_auto_timer();//�ر��Զ���תӦ�ó���ļ�ʱ��
				if((pdata[FRAME_LEN + 1] > 4) && (line_count == (((pdata[FRAME_DATA+2] & 0x00FF)<<8) | pdata[FRAME_DATA+3])))//��ǰ���ţ���0��ʼ
				{
					package_idx = 0;														  						//
					udlen = 0;																								//�ɽ��ջ���ת�Ƶ��л���ĳ��ȣ������ж�һ�����ݰ����������ݴ������
					tt = 0;																										//��ǰ���е�������������
					do{
						stat= GetLine(linebuffer.Hex_handle_buff ,pdata + FRAME_DATA + 4,pdata[FRAME_LEN + 1] - 4,&udlen);										//��ȡ���У����������д����־
						//�ж��Ƿ�Ϊ������
						switch(stat){
							case 0://�ɴ�������
								line_idx += udlen;//ƴ�ӻ�����һ����ĳ���
								if(hex_write(&linebuffer,line_idx) == WRITE_SUCCESS_1)
								{			
									line_idx = 0;																				//��Ҫƴ��ʱ��ָ����������linebuffer������
									tt++;																							//�����е�����
								}else{
								if(tt == 0)																				//�����е��װ��Ƿ���������ƴ�Ӵ��󣬻��߷��ʹ���
									{
										if(line_count)																		//�˻�һ����������������
											line_count--;
									}
									Com_Repeat(pdata,line_count);
								}
								break;
							case -1://�Ƿ�����
								Com_Repeat(pdata,line_count);
								break;
							case -2://��ƴ������
								line_idx += udlen;																		//ƴ��ǰ�ĳ���
								line_count  ++;
								Com_Response(pdata,0);
								break;
							default://��Ԥ�ڱ���
								break;
						}
						if(stat!=0)
							break;
						if(update_finished)
						{	
							udlen = 0;
							break;
						}
					}while(udlen);
					
					//��������Ӧ
					if(!udlen && !stat)
					{
						tsm_Array[FRAME_DATA] = (uint8_t)(line_count >> 8);
						tsm_Array[FRAME_DATA + 1] = (uint8_t)(line_count);
						get_error_data++;
						Com_Response(pdata,2);
						//�ж����һ��
						if(update_finished)
						{
							nrf_delay_ms(500);
							NVIC_SystemReset();
						}
					}
				}
				else//�����ط�
				{
					Com_Repeat(pdata,line_count);
				}
       
			break;
		case CMD_GET_VERSION:
			version();//���¶�ȡ�汾����
			switch(pdata[FRAME_DATA])
				{
				case 0x00:
					len = get_string_len((uint8_t *)App_Version);														//����������
					memcpy(tsm_Array+FRAME_DATA,App_Version,len + 1);
					break;
				case 0x01:
					len = get_string_len((uint8_t *)Boot_Version);														
					memcpy(tsm_Array+FRAME_DATA,Boot_Version,len + 1);
					break;
				case 0x02:
					len = get_string_len((uint8_t *)Mb_Version);															
					memcpy(tsm_Array+FRAME_DATA,Mb_Version,len + 1);
					break;
				case 0x03:
					len = get_string_len((uint8_t *)Hw_Version);															
					memcpy(tsm_Array+FRAME_DATA,Hw_Version,len + 1);
					break;
				default:
					len = 0;
					break;					
				}
			Com_Response(pdata,len+1);
			break;
		case CMD_OPEN_UPDATE:
			len = get_string_len((uint8_t *)App_Version);														//����������
			memcpy(tsm_Array+FRAME_DATA,App_Version,len + 1);
			Com_Response(pdata,len+1);
			break;
		default:
			break;
	}
	
	if(rcv_ok )
	//׼����һ��	
	get_NextUart();
   
	if(rcv_ok)
	{
		
		com_delay = 0;
		step = COM_FIND_SOH;
		rcv_ok = 0;
	}
}
