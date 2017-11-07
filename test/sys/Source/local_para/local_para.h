//local_para.h
#include "stdint.h"
#include "nrf.h"
#define PWR_LIST_LEN 				8
#define RATE_LIST_LEN 			3
//代码区域划分

#define BLE_BEGIN												0x0UL//96KB
#define BOOT_BEGIN 											BLE_BEGIN//0x18000UL//8KB
#define APP_BEGIN 											0x1A000UL				//140K大小,代码区
#define ID_BEGIN												0x3D000					//1KB,硬件信息区,前4字节放ID号
#define PARA_BEGIN 											0x3D400UL			//11KB,参数区

#define CODE_PROTECT_REGION_BEGIN				(BOOT_BEGIN)
#define CODE_PROTECT_REGION_END					(APP_BEGIN -1)
#define APPLICATION_ADDRESS							(APP_BEGIN + 4)
#define APPLICATION_CODE								(APP_BEGIN)

#define jishu                           (PARA_BEGIN)

#define APP_CODE_BEGIN									(APP_BEGIN)
#define APP_CODE_END										(ID_BEGIN -1)
#define APP_CODE_EXE										(APP_BEGIN + 4)

#define UID_ADDR 												(ID_BEGIN)								//UID
#define HW_VERSION_ADDR 								(UID_ADDR + 4)						//
//Vector_Table 192B
#define VECTOR_TABLE_SIZE								200
//BOOT_Ver	20B
//Boot_MB		20B
#define VER_LEN													20
#define VER1_IDX												200
#define VER2_IDX												220
#define CODE_FLAG_LEN										1
#define CODE_FLAGIDX										240

#define APP_VERSION_ADDR								(APP_BEGIN + VECTOR_TABLE_SIZE)				//应用程序固件版本
#define MB_VERSION_ADDR									(APP_BEGIN + VECTOR_TABLE_SIZE + 20)	//应用程序主板版本

#define __ON_BOOT												0
#define __ON_APP												0xa5
#define __CMD1													0x5a
#define __CMD2													0xb5

extern const uint8_t radio_pwr_list[PWR_LIST_LEN];
extern const uint8_t radio_rate_list[RATE_LIST_LEN];

//最长20字节
extern uint8_t Mb_Version[VER_LEN];
extern uint8_t App_Version[VER_LEN];
extern const uint8_t Boot_Version[VER_LEN];
extern const uint8_t Hw_Version[VER_LEN];

extern uint8_t LogicAddr[4];
#define AppFlag NRF_POWER->GPREGRET

// extern uint8_t LogicRate;
// extern uint8_t LogicPipe;
//FILE END
