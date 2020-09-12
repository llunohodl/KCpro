#ifndef __UTILS_H
#define __UTILS_H

//**********************************************************************************************
// Version device
//
// VERSION OF FIRMWARE -> MAJOR_VER.MINOR_VER 
// (for example 1.0 - major = 1, minor = 0)

#define MAJOR_VER		3
#define MINOR_VER		6

//**********************************************************************************************
// Target device
//
//#define WRITER_RW1990
//#define WRITER_CYFRAL
//#define BOOT_UPDATE
//#define RFID_CHECK
//

//**********************************************************************************************
// Options
//			
//#define HI_VOLTAGE
//#define WRITE_DEBUG 		// strobe
#define LCD
//#define READ_WET_CODE
//
//**********************************************************************************************

#define PUMP		LATCbits.LATC5
#define TM_VPP		LATCbits.LATC7
#define TM_VDD		LATBbits.LATB7

#define REG_DAT		LATBbits.LATB4
#define REG_RCK		LATBbits.LATB5
#define REG_SCK		LATBbits.LATB6

#define LCD_E		LATBbits.LATB5
#define LCD_DAT		LATBbits.LATB4
#define LCD_CLK		LATBbits.LATB6

#define TM_IN		PORTAbits.RA3
#define TM_OUT		LATCbits.LATC4

#ifdef WRITE_DEBUG
	#define DEBUG		LATCbits.LATC6		// RF IN
#endif

#define EEPROM_COUNTER_ADDR		0

#define LED_OFF		0
#define LED_OK		1
#define LED_ERROR	2
#define LED_READ	4
#define LED_WRITE	8

#define NO_KEY		0
#define DS1990		1
#define CYFRAL		2
#define TM2002		4
#define EM4100		8



typedef union
{
	struct
	{
		unsigned int major_version;
		unsigned int minor_version;
	};
	unsigned char    bytes[8];
} idreg_t;

typedef struct {
	union {
		unsigned char		b[8];
		unsigned long long	v;
	}serialNumber;
	

	//uint8_t	buffer[RFID_BUFF_LEN+1];
	//uint8_t	bcounter;
        unsigned long long	raw;
	unsigned char	flags;
} rfid_t;

typedef enum
{
  RFID_NO,
  RFID_YES,
  RFID_read,
  RFID_write,
  Write_OK,
  Read_OK,

  
} RFIDflags;

extern idreg_t const idreg;

#define __OPTIMIZE_OFF	_asm nop _endasm

typedef union _WORD
{
    unsigned int w;
    struct
    {
        unsigned char l;
        unsigned char h;
    };
} word;
/*
typedef union
{
   struct
   {
      unsigned int major_version;
      unsigned int minor_version;
   };
   unsigned char    bytes[8];
} idreg_t;
*/

//extern unsigned char LedData;

void BootUpdate(void);

unsigned char Decode_Cyfral(void);

//void REG_Send(unsigned char data);

void LED(unsigned char led);

void StackPointerInit(void);

void LCDSend(unsigned char data, unsigned char rs);

void Tone(unsigned char freq, unsigned int duration);
void Beep(void);
void LongBeep(void);
void ErrorBeep(void);

void Encode_Cyfral(unsigned char CodeHi, unsigned char CodeLow);

unsigned char EEPROMRead(unsigned char Addr);
void EEPROMWrite(unsigned char Addr, unsigned char Byte);
void NonLinearInc(unsigned char *DataH, unsigned char *DataL);


#endif
