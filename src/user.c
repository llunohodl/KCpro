#include <stdlib.h>
#include "delay.h"
#include "lcd.h"
#include "rfid.h"
#include "rw1990.h"
#include "analog.h"
#include "utils.h"
#include "nrfx_timer.h"
#include "nrf_drv_timer.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
//#include "nrfx_gpiote.h"
#include "nrf_drv_pwm.h"
#include "nrfx_pwm.h"
#include "f10x-pcd8544.h"

//#include "interrupt.h"
//#include "boot_update.h"



#define READ_TIME_OUT  0x8000//0x80
#define RFID_BITS_COUNT 64



idreg_t const idreg =
{
	{ 

#ifdef WRITER_CYFRAL
		9, 			// MAJOR VER
		0			// MINOR VER
#endif

#ifdef RFID_CHECK
		9, 			// MAJOR VER
		1			// MINOR VER
#endif

#ifdef WRITER_RW1990
		9, 			// MAJOR VER
		2			// MINOR VER
#endif

#ifndef WRITER_CYFRAL
#ifndef WRITER_RW1990
#ifndef RFID_CHECK
		MAJOR_VER,
		MINOR_VER 
#endif
#endif
#endif
	}
};

//#pragma romdata

// ------------------------------------------------------------------------

extern void TrfInitialSettings(void);
extern void TimerAHandler(void);
//extern void Port_B(void);

uint8_t i, res, key, RFIDcnt, RF_IN_U, RF_IN_D, RF_IN_CNT, RF_NO_CNT;
//extern unsigned char PIR0, PIR1, PIR2;
//unsigned char RF_U[256], RF_D[256];
unsigned long First, Last, RF_CNT;
unsigned char str[30];
extern unsigned char TXBuff[];
unsigned char tm_buff[8];
unsigned char wr_buff[8];


volatile bool	bits[RFID_BITS_COUNT];


rfid_t RFID;

RFIDflags	FLAG;

// =======================================================================


// =========================================================================
// Main function with init and an endless loop
//
//


// ------------------------------------------------------------------------
//#pragma udata 	MAIN_RAM
//#pragma romdata MAIN_DATA
//#pragma code 	MAIN_CODE
// ------------------------------------------------------------------------

#ifdef WRITER_CYFRAL
	unsigned char GenL, GenH, gh, gl;
#endif

#ifdef WRITER_RW1990
	unsigned char GenL, GenH, GenL1, GenH1, gh, gl;
#endif

// ------------------------------------------------------------------------
/*
#ifdef LCD
	const char txt_read[] ={"Read..."};         
	const char txt_write[]={"Write..."};         
#endif
*/
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
        
const nrf_drv_timer_t TIMER0 = NRF_DRV_TIMER_INSTANCE(0);
const nrf_drv_timer_t TIMER1 = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_timer_t TIMER2 = NRF_DRV_TIMER_INSTANCE(2);

unsigned int TC, TIME_OUT;
unsigned long CC;

#define USED_PWM(idx) (1UL << idx)
nrf_pwm_values_individual_t m_pwm_seq_values;
nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            12 | NRF_DRV_PWM_PIN_INVERTED, // channel 0
            NRFX_PWM_PIN_NOT_USED, // channel 1
            NRFX_PWM_PIN_NOT_USED, // channel 2
            NRFX_PWM_PIN_NOT_USED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP_AND_DOWN,
        .top_value    = 64,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

nrf_pwm_sequence_t const    m_pwm_seq =
{
    .values.p_individual = &m_pwm_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_pwm_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};
nrf_drv_pwm_t const m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

void delay1ms(unsigned int time)               // Delay from RFID 125 kHz PWM interrupt
{
	RF_CNT = 0;
        while(RF_CNT < 125 * time);
}
void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
//Trf796xPortB();
}
void timer0_event_handler(nrf_timer_event_t event_type, void* p_context)
{
//TimerAHandler();//Trf796x timer interrupt
}
void timer1_event_handler(nrf_timer_event_t event_type, void* p_context)
{
//PIR1 |= 0x01;
}
void timer2_event_handler(nrf_timer_event_t event_type, void* p_context)
{
//PIR2 |= 0x02;
}
bool checkParity(bool *b) {
	uint8_t	count = 0;
	for (uint8_t i=0; i<4; i++) if (b[i]) count++;	// ???????????? ?????????? ????????
	return ((count%2 == 0) == (b[4] == 0));
}
void pwm_handler(nrf_drv_pwm_evt_type_t event_type)     //RFID 125 kHz PWM interrupt
{
  uint8_t	i, ii;
  RF_CNT ++;
  if(nrf_gpio_pin_read(11)) 
  {
    RF_NO_CNT = 0;
    if(FLAG == RFID_NO) FLAG = RFID_YES;
  }
  else 
  {
   if(RF_NO_CNT < 125/*255*/) RF_NO_CNT ++;
   else FLAG = RFID_NO;
  }
   if(FLAG == RFID_read)
   {
    if(nrf_gpio_pin_read(11) == 1) 
    {
      RF_IN_U ++;
      if(RF_IN_CNT <= 45) RF_IN_CNT ++; 
      if(RF_IN_D)
      {  
      RF_IN_D = 0;
      if(RF_IN_CNT > 45) 
        {
          RF_IN_CNT = 0;
          First <<= 1;
          if(Last & 0x80000000) First |= 1;
          else First &= 0xFFFFFFFE;
          Last <<= 1;
          Last |= 1;
        }
    }
  }
  else 
  {
    RF_IN_D ++;
    if(RF_IN_CNT <= 45) RF_IN_CNT ++; 
    if(RF_IN_U)
    { 
    RF_IN_U = 0;
    if(RF_IN_CNT > 45) 
      {
        RF_IN_CNT = 0;
        First <<= 1;
        if(Last & 0x80000000) First |= 1;
        else First &= 0xFFFFFFFE;
        Last <<= 1;
        Last &= 0xFFFFFFFE;
        
      }
    }

  }
 /* Parce bits */
  if(RFID.serialNumber.b[4] == 0)               // Decode operation RFID 125 kHz 
      if((First & 0xFF800000) == 0xFF800000)
        if(!(Last & 0x01))
        {
  	bool	*p = (bool*)bits + 9, *p2;

          for(i = 0; i < 32; i ++)
          {
            if(First & (0x80000000 >> i)) bits[i] = 1;
               else bits[i] = 0;
          }
          for(i = 0; i < 32; i ++)
          {
            if(Last & (0x80000000 >> i)) bits[i + 32] = 1;
               else bits[i + 32] = 0;
          }

	// check column parity bits
              for (i=0; i<4; i++) 
               {
		volatile uint8_t	count = 0;
		for (ii=0; ii<10; ii++)	if (p[(5 * ii) + i]) count++;
		
		if (!((count%2 == 0) == (p[(5 * 10) + i] == 0))) 
                  break;//return false;
               }
        if(i == 4)
          {
	// version & serial
	p += (5*10);	
	RFID.serialNumber.v = 0;
              for (i=0; i<5; i++) 
               {
		p -= 10;	
		p2 = p+5;

		if (!checkParity(p) || !checkParity(p2)) break;//return false;
		for (ii=0; ii<8; ii++) if (ii<4 ? p2[3-ii] : p[3-(ii-4)]) RFID.serialNumber.b[i] |= (1<<ii);// sbi(RFID.serialNumber.b[i], ii);
                
               }
             if(i ==  5)
             {
                RFID.raw = First;
                RFID.raw <<= 32;
                RFID.raw += Last;
                
                FLAG = Read_OK;
                First = 0;
                Last = 0;
             }
          }
       }
 }
}

void MainApp(void)
{
  uint32_t time_ticks;
  unsigned int i, n, tmp;

  ret_code_t err_code;
  err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(18, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(18, true);
    
    // GPIO config
  
        /*RF_IN*/
	nrf_gpio_cfg_input(11, NRF_GPIO_PIN_NOPULL);//TRISCbits.TRISC6 = 1;
        /*TM_C1*/
	nrf_gpio_cfg_input(6, NRF_GPIO_PIN_NOPULL);
        /*RF_OUT*/
//	nrf_gpio_cfg_output(12);//LATCbits.LATC3 = 0; TRISCbits.TRISC3 = 0;
        /*PUMP*/
	nrf_gpio_cfg_output(0);//LATCbits.LATC5 = 0; TRISCbits.TRISC5 = 0;
 	/*TM_OUT*/
	nrf_gpio_cfg_output(1);//LATCbits.LATC4 = 0; TRISCbits.TRISC4 = 0;
 	/*VDD_ON*/
	nrf_gpio_cfg_output(2);//LATBbits.LATB7 = 0; TRISBbits.TRISB7 = 0;
 	/*VPP_ON*/
	nrf_gpio_cfg_output(3);//LATCbits.LATC7 = 0; TRISCbits.TRISC7 = 0;
        
        nrf_gpio_cfg_output(20);//LSD_nSS
        nrf_gpio_cfg_output(19);//LSD_D
        nrf_gpio_cfg_output(15);//SPI_SDO
        nrf_gpio_cfg_output(13);//SPI_CLK
        nrf_gpio_cfg_output(10);//RES
        
        nrf_gpio_cfg_output(14);//RF13_EN
        nrf_gpio_cfg_output(23);//RF13.56_M
        
        nrf_gpio_cfg_output(28);//BUZZER
        
        nrf_gpio_cfg_input(4, NRF_GPIO_PIN_NOPULL);//TM_IN
        nrf_gpio_cfg_input(16, NRF_GPIO_PIN_NOPULL);//SPI_SDI

        nrf_drv_timer_config_t timer_cfg;                       // = NRF_DRV_TIMER_DEFAULT_CONFIG;
        timer_cfg.frequency = (nrf_timer_frequency_t)4;// 1M
        timer_cfg.bit_width = (nrf_timer_bit_width_t)3;//32b
        timer_cfg.mode = (nrf_timer_mode_t)0;//Timer
        timer_cfg.interrupt_priority = 0; 
        timer_cfg.p_context = NULL; 
        nrf_drv_timer_init(&TIMER0, &timer_cfg, timer0_event_handler);
        time_ticks = 100000;


        nrf_drv_timer_extended_compare(&TIMER0, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
        nrf_drv_timer_enable(&TIMER0);
        
    
    
        APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, pwm_handler));

        m_pwm_seq_values.channel_0 = 32;
        m_pwm_seq_values.channel_1 = 0;
        m_pwm_seq_values.channel_2 = 0;
        m_pwm_seq_values.channel_3 = 0;


	nrf_gpio_pin_clear(3);//TM_VPP = 0;
        
	nrf_gpio_pin_clear(2);//TM_VDD = 0;
        

        
        LCD_Display("   œ–»ÀŒ∆»“≈  ", 
                    "      À ﬁ ◊   ", 
                    "              ", 
                    "              ", 
                    "              ", 
                    "              ");

nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRF_DRV_PWM_FLAG_LOOP);

	Beep();

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
#ifdef WRITER_RW1990

	Delay1KTCYx(10);
        
	do {
		LED(LED_READ);
                tmp = nrf_drv_timer_capture(&TIMER0, NRF_TIMER_CC_CHANNEL1);
		gh = (tmp >> 8);//TMR0H;
		gl = tmp;//TMR0L;

		GenH = (gh & 0xAA/*0b10101010*/) | (gl & 0x55/*0b01010101*/);
		GenL = (gl & 0xAA/*0b10101010*/) | (gh & 0x55/*0b01010101*/);

		gh = (tmp >> 8);//TMR1H;
		gl = tmp;//TMR1L;

		GenH1 = (gl & 0xAA/*0b10101010*/) | (gh & 0x55/*0b01010101*/);
		GenL1 = (gh & 0xAA/*0b10101010*/) | (gl & 0x55/*0b01010101*/);

		tm_buff[0] = 0x01;
		tm_buff[1] = GenL;
		tm_buff[2] = GenH1;
		tm_buff[3] = GenL1;
		tm_buff[4] = GenH >> 4 | GenH & 0x0F;
		if (tm_buff[4] == 0) tm_buff[4] = 0x0E;
		tm_buff[5] = 0x00;
		tm_buff[6] = 0x00;
		tm_buff[7] = CalcCRCTM();

		while (DetectDS1990(160) != 0);

		LED(LED_WRITE);
                n ++;

		for(i = 0; i < 8; i++) wr_buff[i] = tm_buff[i];

		res = Write_DS();

		if (res == 0)
		{	
			LED(LED_OK);
			Beep();
		}
		else
		{
			LED(LED_ERROR);
			ErrorBeep();
		}
		WaitNoDS1990(10);
	} while(1);
}
#endif

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------


#ifdef WRITER_CYFRAL

	gl = TMR0L;
	gh = TMR0H;

	GenH = (gh & 0b10101010) | (gl & 0b01010101);
	GenL = (gl & 0b10101010) | (gh & 0b01010101);

	do {
//		GenH = 0xE2;
//		GenL = 0xFE;
		Encode_Cyfral(GenL, GenH);

		DataCodMem[0] = DataCodTemp[0]; 
		DataCodMem[1] = DataCodTemp[1]; 
		DataCodMem[2] = DataCodTemp[2]; 
		DataCodMem[3] = DataCodTemp[3];
		DataCod[0] = DataCodTemp[0]; 
		DataCod[1] = DataCodTemp[1]; 
		DataCod[2] = DataCodTemp[2]; 
		DataCod[3] = DataCodTemp[3];

		Decode_Cyfral();

		#ifdef LCD
			LCD_CLR();
			LCD_code(CYFRAL); 
		#endif

		LED(LED_WRITE);

		WaitAnaolog(20, 0);

		#ifdef LCD
			lcd8544_putstr(0, 0, "Write...", 0x0C);
		#endif

		res = WrAnalog(CYFRAL);

		#ifdef LCD
			LCD_Error(res);
		#endif

		if (res == 0)
		{
			NonLinearInc(&GenH, &GenL);
			LED(LED_OK);
			Beep();
		}
		else
		{
			LED(LED_ERROR);
			ErrorBeep();
		}
		WaitNoAnaolog(20);
	} while(1);
}
#endif

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

#ifdef RFID_CHECK

	do {

		Delay1KTCYx(10);
                FLAG = RFID_read;
                while(FLAG != Read_OK && FLAG != RFID_NO);
                if(FLAG == Read_OK)
                  {
                    n = 0;
                   for(i = 0; i < 5; i ++) 
                   {
                     tmp = LCD_HEX(RFID.serialNumber.b[4 - i]);
                     str[n++] = tmp >> 8;
                     str[n++] = tmp;
                     str[n++] = 0x20;
                       
                   }
                   str[n++] = 0;
            LCD_Display("   ”—œ≈ÿÕŒ    ", 
                        "   EM MARIN   ", 
                        str, 
                        "              ", 
                        "              ", 
                        "              ");
                    Beep();
                    while(FLAG != RFID_NO);
            LCD_Display("   œ–»ÀŒ∆»“≈  ", 
                    "      À ﬁ ◊   ", 
                    "              ", 
                    "              ", 
                    "              ", 
                    "              ");
                    while(FLAG == RFID_NO);
                  }
		 
                
		Delay1KTCYx(10);
		if ((tm_buff[4] == 0x56) && (tm_buff[3] == 0x5A))
		{
			Beep(); 
			Beep(); 
			Beep(); 
		}
//		RFIDFree(5);
	} while(1);
}

#endif

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

#ifndef RFID_CHECK
	#ifndef WRITER_CYFRAL 
	#ifndef WRITER_RW1990

	do {
          LCD_Display("   œ–»ÀŒ∆»“≈  ", 
                    "      À ﬁ ◊   ", 
                    "              ", 
                    "              ", 
                    "              ", 
                    "              ");

		delay1ms(500);
		do {
			key = NO_KEY; 
			if (key == NO_KEY) key = ReadRFID();
			if (key == NO_KEY) key = OneReadDS1990(1);
                        //if (key == NO_KEY) key = ReadAnalog();
                        //Iso14443aFindTag();//FindTags(TAG_IT_PROTOCOL);
		} while (key == NO_KEY);
		Beep(); 
		delay1ms(500);

		if (key == EM4100) 
		{
        n = 0;
        for(i = 0; i < 5; i ++) 
          {
           tmp = LCD_HEX(RFID.serialNumber.b[4 - i]);
           str[n++] = tmp >> 8;
           str[n++] = tmp;
           str[n++] = 0x20;    
          }
            str[n++] = 0;
            LCD_Display("  —◊»“€¬¿Õ»≈  ",
                        "    ”—œ≈ÿÕŒ   ", 
                        "    EM4100    ", 
                        str, 
                        "    ”¡≈–»“≈   ", 
                        "      À ﬁ ◊   " 
                        );
            while(FLAG != RFID_NO);
            LCD_Display("  œ–»ÀŒ∆»“≈   ", 
                        "  «¿√Œ“Œ¬ ”   ", 
                        "    EM4100    ",
                        "              ",
                        "              ", 
                        "              " 
                        );
            while(FLAG != RFID_NO);
            while(FLAG == RFID_NO);
			RFIDFree(5);

			res = WriteRFID(10);
			if (res == 0)
			{
				Beep();
                                n = 0;
        for(i = 0; i < 5; i ++) 
          {
           tmp = LCD_HEX(RFID.serialNumber.b[4 - i]);
           str[n++] = tmp >> 8;
           str[n++] = tmp;
           str[n++] = 0x20;    
          }
            str[n++] = 0;
            LCD_Display("    «¿œ»—‹    ", 
                        "    ”—œ≈ÿÕŒ   ", 
                        "    EM4100    ",
                        str, 
                        "    ”¡≈–»“≈   ", 
                        "      Àﬁ◊     " 
                        );
            while(FLAG != RFID_NO);
            RFIDFree(10);
			}
			if (res == 2)
			{
            LCD_Display("              ", 
                        "    Œÿ»¡ ¿    ", 
                        "              ",
                        "    «¿œ»—»    ",
                        "              ", 
                        "              " 
                        );
				LongBeep(); 
			}
		}
		if (key == DS1990) 
		{
	n = 0;
        for(i = 0; i < 7; i ++) 
          {
           tmp = LCD_HEX(tm_buff[i]);
           str[n++] = tmp >> 8;
           str[n++] = tmp;
          }
            str[n++] = 0;
            LCD_Display("  —◊»“€¬¿Õ»≈  ",
                        "    ”—œ≈ÿÕŒ   ", 
                        "    DS1990    ", 
                        str, 
                        "    ”¡≈–»“≈   ", 
                        "      À ﬁ ◊   " 
                        );
			WaitNoDS1990(10);
            LCD_Display("  œ–»ÀŒ∆»“≈   ", 
                        "  «¿√Œ“Œ¬ ”   ", 
                        "    DS1990    ",
                        "              ",
                        "              ", 
                        "              " 
                        );

			for(i = 0; i < 8; i++) wr_buff[i] = tm_buff[i];

			if (WaitDS1990(20, 400) == 0)
			{

				res = Write_DS();


				if (res == 0)
				{	                      
        n = 0;
        for(i = 0; i < 7; i ++) 
          {
           tmp = LCD_HEX(tm_buff[i]);
           str[n++] = tmp >> 8;
           str[n++] = tmp;
          }
            str[n++] = 0;
            LCD_Display("    «¿œ»—‹    ", 
                        "    ”—œ≈ÿÕŒ   ", 
                        "    DS1990    ",
                        str, 
                        "    ”¡≈–»“≈   ", 
                        "      Àﬁ◊     " 
                        );
					Beep();
				}
				else
				{
            LCD_Display("              ", 
                        "    Œÿ»¡ ¿    ", 
                        "              ",
                        "    «¿œ»—»    ",
                        "              ", 
                        "              " 
                        );
					ErrorBeep();
				}
				WaitNoDS1990(20);
			} 
			else 
			{ 
				LongBeep(); 
			}
		}

		if ((key == CYFRAL) || (key == TM2002))
		{
			WaitNoAnaolog(10);
			LED(LED_WRITE);

			if (WaitAnaolog(20, 400) == 0)
			{	

				res = WrAnalog(key);
				if (res == 0)
				{
//					LED(LED_OK);
					Beep();
				}
				else
				{
//					LED(LED_ERROR);
					ErrorBeep();
				}
				WaitNoAnaolog(20);
			} 
			else 
			{ 
				LongBeep(); 
			}
		} 
                

	} while(1);	
}

	#endif
	#endif
#endif
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

