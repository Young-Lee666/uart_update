//interrupt.c
#include "local_para.h"
#include "nrf.h"
extern void UART0_IRQHandler_boot(void);

extern void TIMER0_IRQHandler_Boot();
//extern uint8_t AppFlag;
void POWER_CLOCK_IRQHandler(void)//16 ; POWER_CLOCK
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (16 << 2))))();
	else{
	}
}
void RADIO_IRQHandler(void)//17 ;RADIO
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (17 << 2))))();
	else{
	}
}
void UART0_IRQHandler(void)//18 ;UART0
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (18 << 2))))();
	else{
		UART0_IRQHandler_boot();
	}
}
void SPI0_TWI0_IRQHandler(void)//19 ;SPI0_TWI0
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (19 << 2))))();
	else{
	}
}
void SPI1_TWI1_IRQHandler(void)//20 ;SPI1_TWI1
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (20 << 2))))();
	else{
	}
}
//21 0 ;Reserved
void GPIOTE_IRQHandler(void)//22;GPIOTE
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (22 << 2))))();
	else{
	}
}
void ADC_IRQHandler(void)//23 ;ADC
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (23 << 2))))();
	else{
	}
}
void TIMER0_IRQHandler(void)//24 ;TIMER0
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (24 << 2))))();
	else{
	TIMER0_IRQHandler_Boot();
	}
}
void TIMER1_IRQHandler(void)//25 ;TIMER1
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (25 << 2))))();
	else{
			
	}
}
void TIMER2_IRQHandler(void)//26 ;TIMER2
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (26 << 2))))();
	else{
	}
}
void RTC0_IRQHandler(void)//27 ;RTC0
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (27 << 2))))();
	else{
	}
}
void TEMP_IRQHandle(void)//28 ;TEMP
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (28 << 2))))();
	else{
	}
}
void RNG_IRQHandler(void)//29 ;RNG
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (29 << 2))))();
	else{
	}
}
void ECB_IRQHandler(void)//30 ;ECB
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (30 << 2))))();
	else{
	}
}
void CCM_AAR_IRQHandler(void)//31 ;CCM_AAR
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (31 << 2))))();
	else{
	}
}
void WDT_IRQHandler(void)//32 ;WDT
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (32 << 2))))();
	else{
	}
}
void RTC1_IRQHandler(void)//33 ;RTC1
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (33 << 2))))();
	else{
	}
}
void QDEC_IRQHandler(void)//34 ;QDEC
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (34 << 2))))();
	else{
	}
}
void LPCOMP_COMP_IRQHandler(void)//35 ;LPCOMP_COMP
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (35 << 2))))();
	else{
	}
}
void SWI0_IRQHandler(void)//36 ;SWI0
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (36 << 2))))();
	else{
	}
}
void SWI1_IRQHandler(void)//37 ;SWI1
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (37 << 2))))();
	else{
	}
}
void SWI2_IRQHandler(void)//38 ;SWI2
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (38 << 2))))();
	else{
	}
}
void SWI3_IRQHandler(void)//39 ;SWI3
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (39 << 2))))();
	else{
	}
}
void SWI4_IRQHandler(void)//40 ;SWI4
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (40 << 2))))();
	else{
	}
}
void SWI5_IRQHandler(void)//41 ;SWI5
{
	if(AppFlag == __ON_APP)
	(*(void (*)(void))(*(uint32_t*)(APP_BEGIN + (41 << 2))))();
	else{
	}
}

//FILE END
