/*parser.h*/
#include <stdint.h>

#define SUCCESSED	0
#define FAILED		-1

//通讯接口定义
#define INTERFACE_UNKNOW							0
#define INTERFACE_UART								1
#define INTERFACE_RADIO								2
#define INTERFACE_USB									3
#define INTERFACE_BLE									4

#define PROTOCOL_EXT_NUM	11
#define PROTOCOL_CHECKNUM	9
#define PROTOCOL_DES		0
#define PROTOCOL_SOR		4
#define PROTOCOL_LEN		8
#define PROTOCOL_CMD		9
#define PROTOCOL_DAT		11

//通讯接口定义
extern uint8_t interface_type;

//外部接口
int8_t ascii2hex(uint8_t * pdata,uint8_t len);
int8_t checksum(uint8_t * pdata,uint8_t len);
int8_t checkascii(uint8_t ch);
int8_t checkxor(uint8_t *pdata,uint8_t len);
uint8_t getsum(uint8_t * pdata,uint8_t len);
uint8_t get_string_len(uint8_t *pstr);

