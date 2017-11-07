//local_para.c
#include "stdint.h"
#include "nrf.h"
#include "local_para.h"
//功率对应表
const uint8_t radio_pwr_list[PWR_LIST_LEN] = {RADIO_TXPOWER_TXPOWER_Pos4dBm,//PWR_LIST_LEN
																	RADIO_TXPOWER_TXPOWER_0dBm,
																	RADIO_TXPOWER_TXPOWER_Neg4dBm,
																	RADIO_TXPOWER_TXPOWER_Neg8dBm,
																	RADIO_TXPOWER_TXPOWER_Neg12dBm,
																	RADIO_TXPOWER_TXPOWER_Neg16dBm,
																	RADIO_TXPOWER_TXPOWER_Neg20dBm,
																	RADIO_TXPOWER_TXPOWER_Neg30dBm};
const uint8_t radio_rate_list[RATE_LIST_LEN] = {RADIO_MODE_MODE_Nrf_250Kbit,//RATE_LIST_LEN
																		RADIO_MODE_MODE_Nrf_2Mbit,
																		RADIO_MODE_MODE_Nrf_1Mbit};
//本地参数
uint8_t LogicAddr[4];//ADDR_LEN
// uint8_t LogicPwr = 0x00;
// uint8_t LogicRate = 0x02;
// uint8_t LogicPipe = 85;
//
uint8_t App_Version[VER_LEN];
uint8_t Mb_Version[VER_LEN];
																		
																		
const uint8_t Boot_Version[VER_LEN] __attribute__((at(BOOT_BEGIN + VER1_IDX))) = "ST03UbootV02";					
const uint8_t Hw_Version[VER_LEN] __attribute__((at(BOOT_BEGIN + VER2_IDX))) = "ST03UMBV0.1.1";																		
//#if defined(__ST03U)
//const uint8_t Boot_Version[VER_LEN] __attribute__((at(BOOT_BEGIN + VER1_IDX))) = "ST03UbootV02";
//#elif defined(__NTN206)
//const uint8_t Boot_Version[VER_LEN] __attribute__((at(BOOT_BEGIN + VER1_IDX))) = "NTN206bootV02";
//#endif
//const uint8_t Hw_Version[VER_LEN] __attribute__((at(BOOT_BEGIN + VER2_IDX))) = "ST03UMBV0.1.1";
//FILE END
