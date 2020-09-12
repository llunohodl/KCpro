
#include "delay.h"
#include "utils.h"
#include "analog.h"
#include "nrf_gpio.h"

// ------------------------------------------------------------------------
///#pragma udata 	MAIN_RAM
///#pragma romdata MAIN_DATA
///#pragma code 	MAIN_CODE
// ------------------------------------------------------------------------

///#pragma udata access USB_ACS

unsigned char LedData;
extern unsigned long RF_CNT;

///#pragma udata

//////////////////////////////////////////////////////

void StackPointerInit(void)
{
/*	_asm
	    lfsr 1, _stack			// Initialize the stack pointer.
	    lfsr 2, _stack
	_endasm;
*/
}


// ------------------------------------------------------------------------
#ifdef LCD
// Only for LCD code...
void LCDSend(unsigned char data, unsigned char rs)
{
/*	unsigned char n;

	if (rs != 0) LCD_DAT = 1; else LCD_DAT = 0;
	LCD_CLK = 1;
	Delay10TCYx(1);
	LCD_CLK = 0;
	n = 6;
	do {
		if ((data & 0x08) != 0) LCD_DAT = 1; else LCD_DAT = 0;
		LCD_CLK = 1;
		data <<= 1;
		LCD_CLK = 0;
	} while(--n);
*/
}

#endif
// ------------------------------------------------------------------------

void LED(unsigned char led)
{
	LedData = 0;
	if ((led & LED_READ)  != 0) LedData |= 0x01;
	if ((led & LED_WRITE) != 0) LedData |= 0x02;
	if ((led & LED_OK) 	  != 0) LedData |= 0x04;
	if ((led & LED_ERROR) != 0) LedData |= 0x08;
#ifndef LCD
	REG_Send(LedData);
#endif
}

void Gen(unsigned char freq,  unsigned int duration)
{
	// Only for LED code...
	while (duration !=0 )
	{                
		nrf_gpio_pin_set(28);
                RF_CNT = 0;
		while(RF_CNT < freq);
		nrf_gpio_pin_clear(28);
                RF_CNT = 0;
		while(RF_CNT < freq);
		duration--;
	}
}

// ------------------------------------------------------------------------

void LongBeep(void)
{
   Gen(63, 1000);
}

void Beep(void)
{
   Gen(73, 128);
}

void ErrorBeep(void)
{
   Gen(200, 60);
   Delay1KTCYx(30);
   Delay1KTCYx(30);
   Gen(250, 105);
}

// ------------------------------------------------------------------------
#ifdef WRITER_CYFRAL

// Only for ERITER code...
unsigned char EncodeTwoBits(unsigned char c)
{    
   	switch (c & 0x03) 
	{
		case 0x00:	return(0x7);
		case 0x01:	return(0xB);
		case 0x02:	return(0xD);
   		case 0x03:	return(0xE);
   	} 
}

// Only for ERITER code...
void Encode_Cyfral(unsigned char CodeHi, unsigned char CodeLow)
{    
	DataCodTemp[3]  = EncodeTwoBits(CodeHi);
	CodeHi >>= 2;
  	DataCodTemp[3] |= EncodeTwoBits(CodeHi) << 4;
	CodeHi >>= 2;
	DataCodTemp[2]  = EncodeTwoBits(CodeHi);
	CodeHi >>= 2;
  	DataCodTemp[2] |= EncodeTwoBits(CodeHi) << 4;

	DataCodTemp[1]  = EncodeTwoBits(CodeLow);
	CodeLow >>= 2;
  	DataCodTemp[1] |= EncodeTwoBits(CodeLow) << 4;
	CodeLow >>= 2;
	DataCodTemp[0]  = EncodeTwoBits(CodeLow);
	CodeLow >>= 2;
  	DataCodTemp[0] |= EncodeTwoBits(CodeLow) << 4;
}

// Only for ERITER code...
void Ror(unsigned char *data)
{
	unsigned char n;

	n = *data >> 1;
	if ((*data & 0x01) != 0) n |= 0x80;
	*data = n;
}

// Only for ERITER code...
void Rol(unsigned char *data)
{
	unsigned char n;

	n = *data << 1;
	if ((*data & 0x80) != 0) n |= 0x01;
	*data = n;
}

// Only for ERITER code...
void NonLinearInc(unsigned char *DataH, unsigned char *DataL)
{
	word n, t;
	unsigned char bl, bh;

	n.h = *DataH;
	n.l = *DataL;

	bl = (n.l & 0b01010101) | (n.h & 0b10101010); 
	bh = (n.l & 0b10101010) | (n.h & 0b01010101);
	n.l = bl; n.h = bh;

	n.w ^= 0x6EB4;

	Ror(&n.l); Ror(&n.l); Ror(&n.l);
	Rol(&n.h); Rol(&n.h); Rol(&n.h);

	
	n.w += 0x0001;


	Rol(&n.l); Rol(&n.l); Rol(&n.l);
	Ror(&n.h); Ror(&n.h); Ror(&n.h);

	n.w ^= 0x6EB4;

	*DataL = (n.l & 0b01010101) | (n.h & 0b10101010); 
	*DataH = (n.l & 0b10101010) | (n.h & 0b01010101); 
}

#endif

