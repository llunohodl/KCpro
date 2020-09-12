
//#include "p18cxxx.h"
#include "delay.h"
#include "lcd.h"
#include "rw1990.h"
#include "analog.h"
#include "utils.h"
#include "nrf_drv_timer.h"
#include "nrf_gpio.h"

//////////////////////////////////////////////////////

//#define WR_OFFSET	(t_3 + (t_3 >> 2))
//#define WR_DELAY	(t_3 / 3)

#define WR_OFFSET	(t_3 + (t_3 / 4))
#define WR_DELAY	(t_3 / 2)

// ------------------------------------------------------------------------
//#pragma udata 	MAIN_RAM
//#pragma romdata MAIN_DATA
//#pragma code 	MAIN_CODE
// ------------------------------------------------------------------------

extern const nrf_drv_timer_t TIMER0;
extern const nrf_drv_timer_t TIMER1;
extern const nrf_drv_timer_t TIMER2;

word Tbit;


unsigned char TuneComparatorWR(void)
{
//	static unsigned char min, max, s;
//	static unsigned char b, i, count, f;
//	static unsigned int time;

#define TUNE_TIME 		0x100
#define TUNE_PERIODS 	4
/*
	REFCON0 = 0b10110000;				// REF - ENABLED
//	REFCON0 = 0b00000000;				// REF - DISABLED

	REFCON1 = 0b11000000;				// + = VDD
//	REFCON1 = 0b11001000;				// + = REF

	while (REFCON0bits.FVR1ST == 0);	// REF - STABLE
	i = 0x00; min = 0; max = 0; f = 0;
	do {
		REFCON2 = i;
		Delay10TCYx(10);

		count = 0; time = 0;
		s = 0; if (CM1CON0bits.C1OUT != 0) s++;
		do {
			Delay10TCYx(10);
			b = 0; if (CM1CON0bits.C1OUT != 0) b++;
			if (b != s) 
			{
				count++;
				s = b; 
				time = 0;
			}
			time++;
		} while ((time != TUNE_TIME) && (count < TUNE_PERIODS));

		if (count == TUNE_PERIODS)
		{
			if (f == 0)
			{
				min = i;
				f = 1;
			}
			else max = i;
		}
		i++;
	} while (i < 32);

	if (max == 0) return(1);
	i = min + ((max - min) >> 1); 
	if (i > 0x1F) return(1);
	REFCON2 = i;
        */
	return(0);
}

unsigned char WAIT_0(void)	
{    
//	TMR3H = 0xD0; TMR3L = 0; PIR2bits.TMR3IF = 0; 

    while (nrf_gpio_pin_read(6) == 1) 
	{ 
		//if (PIR2bits.TMR3IF == 1) return(1); 
	}
	return(0);
}

unsigned char WAIT_1(void)	
{    
//	TMR3H = 0xD0; TMR3L = 0; PIR2bits.TMR3IF = 0; 

    while (nrf_gpio_pin_read(6) == 0) 
	{ 
		//if (PIR2bits.TMR3IF == 1) return(1); 
	}
	return(0);
}

void WAIT_POINT(void)	
{    
	//while ((Tbit.l != TMR0L) || (Tbit.h != TMR0H));
}

void FrontDelay(void)
{
	Delay10TCYx(5);
}


unsigned char WriteAnalog(unsigned char key)	
{    
	static word t0, t1;
	static int dt;
	static unsigned char n, i, z;
	static unsigned char cod, zag;
	static unsigned int per, last_per, j;
	static unsigned int t_3;
	static unsigned long cod_zag, per_sum;
	
//	__OPTIMIZE_OFF
/*
	INTCON = 0;

	CM1CON0 = 0b11011110;	// Vref+, IN2- (RC2), Invert input
	CM2CON0 = 0b00000000;
	CM2CON1 = 0b00000000;	// Vrev to FVR
*/
	nrf_gpio_pin_set(2);//TM_VDD = 1;
	Delay10KTCYx(50);	// 200mc

	if (TuneComparatorWR() != 0) return('1');

	if (WAIT_0() != 0) return('2');
	FrontDelay();

	if (WAIT_1() != 0) return('2');
	t1.l = NRF_TIMER0->CC[0];//TMR0L;
	t1.h = NRF_TIMER0->CC[0];//TMR0H;

	n = 0; j = 0; per_sum = 0;
	do {
		if (WAIT_0() != 0) return('3');
		t0.l = NRF_TIMER0->CC[0];//;TMR0L;
		t0.h = NRF_TIMER0->CC[0];//TMR0H;
		per = t0.w - t1.w;

		n++;
		if (j == 600) return('4');

		if (WAIT_1() != 0) return('3');
		t1.l = NRF_TIMER0->CC[0];//TMR0L;
		t1.h = NRF_TIMER0->CC[0];//TMR0H;
		per += t1.w - t0.w;

		per_sum += per;
		dt = last_per - per;
		if ((j == 0) || ((dt > 4) || (dt < -4)))
		{
			n = 0;
			per_sum = 0;
		}
		j++;
		last_per = per;
	} while (n < 16);
	per = per_sum >> 4;

/*
	LCD_CLR();
	LCD_HEX(per>>8, 0);
	LCD_HEX(per, 3);
*/
	if (per < 0x0200) return('5');

	t_3 = per / 3;

z = 3;
do {
	t0.w = t1.w + (t_3 >> 1);
	Tbit.w = t0.w;

	#define ZAG_COD_HV	0x05169D45//0b00000101000101101001110101000101	// HV по напряжению !!!
	#define CODE_MASK	0x05FE64BF//0b00000101111111101101111101111111

	n = 3; j = 0; cod_zag = 0;
	do {
		cod_zag <<= 1; 
		WAIT_POINT();
		if (nrf_gpio_pin_read(6) == 1) cod_zag++;

		Tbit.w += t_3;
		if (--n == 0)
		{ 
			n = 3;
			t0.w += per; 
			Tbit.w = t0.w; 
		} 
		if (++j > 600) return('6');
	} while ((j < 27) || ((cod_zag & CODE_MASK) != ZAG_COD_HV));

// запись кода  
//	return(0);

	i = 0; cod = 0x80; 

	zag = 0x20;//0b00100000;		// Метаком
	if (key == CYFRAL) zag = 0x10;//0b00010000;		// Цифрал

	if (WAIT_1() != 0) return('7');
WR_PROJ:
	n = zag; if (zag == 0) n = DataCodMem[i];
	n &= cod;

	if (WAIT_0() != 0) return('7');
	nrfx_timer_clear(&TIMER0);//TMR0H = 0; TMR0L = 0;
	t0.w = 0;
	Tbit.w = WR_OFFSET;

	WAIT_POINT();

	if (n != 0) nrf_gpio_pin_set(3);//TM_VPP = 1;

	Tbit.w += WR_DELAY;
	WAIT_POINT();
	nrf_gpio_pin_clear(3);//TM_VPP = 0;

	t0.w += per;
	Tbit.w = t0.w + WR_OFFSET;

	if (zag != 0)
	{
		if (cod == 0x10/*0b00010000*/)
		{
			cod = 0x80;
			zag = 0;
		} else cod >>= 1;
		goto WR_PROJ;
	}	

	if (cod != 0x01)
	{ 
		cod >>= 1; 
		goto WR_PROJ; 
	}

   	cod = 0x80; 
   	i++;
	if (i < 4) goto WR_PROJ;

	WAIT_POINT();

//  ---- запись типа ключа и метки END ----

	if (WAIT_1() != 0) return('8');
	FrontDelay();

	if (WAIT_0() != 0) return('8');
	nrfx_timer_clear(&TIMER0);//TMR0H = 0; TMR0L = 0;

	t0.w = per * 3;
	Tbit.w = t0.w + WR_OFFSET;
	WAIT_POINT();

	if (key == TM2002) nrf_gpio_pin_set(3);//TM_VPP = 1;

	Tbit.w += WR_DELAY;
	WAIT_POINT();
	nrf_gpio_pin_clear(3);//TM_VPP = 0;

} while(--z);

	t0.w += per << 1;				// + 2 периода
	Tbit.w = t0.w + WR_OFFSET;
	WAIT_POINT();
	nrf_gpio_pin_set(3);//TM_VPP = 1;

	Tbit.w += WR_DELAY;
	WAIT_POINT();
	nrf_gpio_pin_clear(3);//TM_VPP = 0;
	return(0);
}





unsigned char WrAnalog(unsigned char key)	
{
	static unsigned char i, res;
/*
	OSCCON = 0b01100000;	// 48.0MHz
	T0CON = 0b10010011;		// Enabled, 16bit, 1:16

	PSTRCON = 0b00000101;
	CCP1CON = 0b00111100;

#ifdef HI_VOLTAGE
	T2CON = 0b00000101;		// Enabled, 1:4, 1:1
	PR2 = 0x80;
	CCPR1L = 0x3F;
#else
	T2CON = 0b00000100;		// Enabled, 1:1, 1:1
	PR2 = 0xFF;
	CCPR1L = 0x7F;
#endif
*/
	res = WriteAnalog(key);

	nrf_gpio_pin_clear(3);//TM_VPP = 0;
	nrf_gpio_pin_clear(2);//TM_VDD = 0;

#ifdef LCD
	if (res != 0) 
	{
		//OSCCON = 0b01100010;	// 8.0MHz
		return(res);
	}
#endif


	nrf_gpio_pin_set(1);//TM_OUT = 1;
	Delay1KTCYx(1);	// Reset key 200mc
	nrf_gpio_pin_clear(1);//TM_OUT = 0;
	Delay100TCYx(250);
/*
	INTCON = 0; 
	OSCCON = 0b01100000;	// 48.0MHz
	T0CON = 0b10010000;		// Enabled, 16bit, 1:2
*/

	i = 20;
	do {
		if (key == CYFRAL) res = ReadCyfral(1); else res = ReadMetacom(1);
		i--;
	} while ((i > 0) && (res != 0));

//	OSCCON = 0b01100010;	// 8.0MHz

	if (res != 0) return(0x15);

	if (DataCodMem[0] != DataCodTemp[0]) return(0x16);
	if (DataCodMem[1] != DataCodTemp[1]) return(0x16);
	if (DataCodMem[2] != DataCodTemp[2]) return(0x16);
	if (DataCodMem[3] != DataCodTemp[3]) return(0x16);
	return(0);
}

// ------------------------------------------------------------------------
//#pragma romdata
//#pragma code
//#pragma udata
// ------------------------------------------------------------------------



