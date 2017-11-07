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
uint16_t line_count = 0;                          //包号索引
uint16_t getvaliddata=0;
uint16_t data_error=0;
uint16_t get_error_data=0;
uint16_t flash_add=0;
int8_t stat;//升级程序用
extern uint8_t boot_flag;
extern uint8_t rec_flag;

extern uint16_t com_delay;
extern uint8_t test;
extern uint16_t crc_delay;


uint8_t Source_Addr = 0;													//源地址缓冲

uint8_t Count_idx;																//小包序号
uint8_t cmd_idx;																	//命令行缓冲区的索引
uint16_t last_idx;																//记录上一次的缓冲索引，仅用于二级缓冲


#define IDX_DES_ADDR1		1
#define IDX_DES_ADDR2		3
#define IDX_SOR_ADDR1		5
#define IDX_SOR_ADDR2		7
#define IDX_SRL					9
#define IDX_LEN					10
#define IDX_SRL_LINE		11
#define IDX_DATA	13


//协议命令字
#define CMD_UPDATE_TRIGGER	0x09						//升级触发
#define CMD_UPDATE_HEX			0x0A						//升级数据包
#define CMD_UP_REPEAT				0x8B						//上行：请求重发
#define CMD_GET_VERSION			0x0C						//获取版本号
#define CMD_OPEN_UPDATE			0x0D						//启动升级功能

#define CMD_UP_Msk					0x80						//上报数据掩码


/*解析HEX文件*/

#define	HEX_LEN			1
#define HEX_ADDR		2
#define HEX_TYPE		4
#define HEX_DATA		5





uint8_t business_step = STEP_TRIGGER;


/*******************************************************
函数名:校验hex格式文件，并更新单行内容到片上Flash
参  数:-*pcode，Ascii表示的行数据，含校验码
       -len,传送长度，最后一个字节为待校准的校验和数值
				包含0D 0A
返  回:-FAILED，校验失败
			 -SUCCESSED，更新代码成功
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
	if(ascii2hex(pdata->Hex_handle_buff+1,len) == FAILED)																		//ascii转换为十六进制
	{	
	return WRITE_FAILED_1;}
	//数据包检验
	if(checksum(pdata->Hex_handle_buff+1,len) == FAILED)																			//校验
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
函数名:擦除应用程序区代码
参  数:无 
返  回:-FAILED，擦除失败，固定应用程序大小4K bytes
			 -SUCCESSED，擦除成功
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
函数名:释放boot程序占用的资源
参  数:无
返  回:无
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
	//确保所有中断在进入APP之前都是关闭状态
		for(i = POWER_CLOCK_IRQn;i <= SWI5_IRQn;i++)
		NVIC_DisableIRQ( i );
}
/*******************************************************
函数名:检验应用程序堆栈有效性，并跳转
参  数:无
返  回:-0，始终调用函数，堆栈地址有效才实现跳转。
*******************************************************/
int8_t app_code_jump(void)
{
		if(((*(uint32_t *)APP_CODE_BEGIN)& 0x2FFE0000)==0x20000000)
	{
		if(AppFlag == __ON_BOOT)//应用程序接管时，直接跳转
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

/*获取有效数据*/
uint8_t * GetValidData(void)
{
	uint16_t crc;
	uint16_t len;
		
		if(rcv_ok == 1)
		{
					
				//		crc_timer();  //进入此判断开始计时 
                          //超时就上报要求重发
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

	//串口纠错
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
	static uint8_t flag;//行结束标志
	//clear_hex_write(&linebuffer);
 	for(i = 0;(i + line_idx)<BUFFER_LEN;i++)
	{
		if(package_idx >= len){														//有数据未处理
			*rlen = i;
			return -2;																			//需要下一包拼接
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
				*rlen = i + 1;																//返回长度
				return 0;																			//可处理数据
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
	app_code_erase();//擦除App区，AA AA FE 00 00 00 SS 09 00 00 5a 5a
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
		//准备下一包
		get_NextUart();
			return;
	}
	
	switch(pdata[FRAME_CMD])																							//命令字
	{
		
		case CMD_UPDATE_TRIGGER:
			stop_auto_timer();//关闭自动跳转应用程序的计时器
			app_code_erase();																								//擦除App区
			Com_Response(pdata,0);
			business_step = STEP_GET_APP;
			line_count = 0;
			line_idx = 0;
			update_finished = 0;
			break;
		case CMD_UPDATE_HEX:
			if(business_step != STEP_GET_APP)
				break;
			
			stop_auto_timer();//关闭自动跳转应用程序的计时器
				if((pdata[FRAME_LEN + 1] > 4) && (line_count == (((pdata[FRAME_DATA+2] & 0x00FF)<<8) | pdata[FRAME_DATA+3])))//当前包号，从0开始
				{
					package_idx = 0;														  						//
					udlen = 0;																								//由接收缓冲转移到行缓冲的长度，用于判断一个数据包的升级数据处理结束
					tt = 0;																										//当前包中的升级数据行数
					do{
						stat= GetLine(linebuffer.Hex_handle_buff ,pdata + FRAME_DATA + 4,pdata[FRAME_LEN + 1] - 4,&udlen);										//获取单行，返回完整行处理标志
						//判断是否为完整包
						switch(stat){
							case 0://可处理数据
								line_idx += udlen;//拼接或完整一包后的长度
								if(hex_write(&linebuffer,line_idx) == WRITE_SUCCESS_1)
								{			
									line_idx = 0;																				//需要拼接时，指向新数据在linebuffer的索引
									tt++;																							//单包中的行数
								}else{
								if(tt == 0)																				//单包中的首包非法，可能是拼接错误，或者发送错误
									{
										if(line_count)																		//退回一个包，并重新请求
											line_count--;
									}
									Com_Repeat(pdata,line_count);
								}
								break;
							case -1://非法数据
								Com_Repeat(pdata,line_count);
								break;
							case -2://待拼接数据
								line_idx += udlen;																		//拼接前的长度
								line_count  ++;
								Com_Response(pdata,0);
								break;
							default://非预期报错
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
					
					//正常包响应
					if(!udlen && !stat)
					{
						tsm_Array[FRAME_DATA] = (uint8_t)(line_count >> 8);
						tsm_Array[FRAME_DATA + 1] = (uint8_t)(line_count);
						get_error_data++;
						Com_Response(pdata,2);
						//判断最后一包
						if(update_finished)
						{
							nrf_delay_ms(500);
							NVIC_SystemReset();
						}
					}
				}
				else//请求重发
				{
					Com_Repeat(pdata,line_count);
				}
       
			break;
		case CMD_GET_VERSION:
			version();//重新读取版本缓冲
			switch(pdata[FRAME_DATA])
				{
				case 0x00:
					len = get_string_len((uint8_t *)App_Version);														//不含结束符
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
			len = get_string_len((uint8_t *)App_Version);														//不含结束符
			memcpy(tsm_Array+FRAME_DATA,App_Version,len + 1);
			Com_Response(pdata,len+1);
			break;
		default:
			break;
	}
	
	if(rcv_ok )
	//准备下一包	
	get_NextUart();
   
	if(rcv_ok)
	{
		
		com_delay = 0;
		step = COM_FIND_SOH;
		rcv_ok = 0;
	}
}
