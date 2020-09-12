
//#include "p18cxxx.h"
#include "delay.h"
#include "rw1990.h"
#include "utils.h"
#include "nrf_gpio.h"
#include "nrf_drv_pwm.h"

extern void delay1ms(unsigned int time);
extern uint8_t ow_reset(void);
extern uint8_t ow_read_byte(void);
extern void ow_write_byte(uint8_t data);
extern uint8_t ow_rw1990_write_rom(uint8_t *new);

extern unsigned long RF_CNT;
extern unsigned char tm_buff[8];
extern unsigned char wr_buff[8];

extern nrf_pwm_sequence_t const    m_pwm_seq;
extern nrf_drv_pwm_t const m_pwm0;

// ------------------------------------------------------------------------
//#pragma udata 	MAIN_RAM
//#pragma romdata MAIN_DATA
//#pragma code 	MAIN_CODE
// ------------------------------------------------------------------------

unsigned char TmReset(void)
{
  return ow_reset();
}
                
void Tm_Read(void)
{
  unsigned char i;
    ow_write_byte(0x33);//byte_snd(0x33);
    for(i = 0; i < 8; i ++) tm_buff[i] = ow_read_byte();
}
unsigned char Compare(unsigned char *buff1, unsigned char *buff2, unsigned char len)
{
	unsigned char pos;

	pos = 0;
	while(len > 0)
	{
		if (buff1[pos] != buff2[pos]) return(1); 
		len--; pos++;
	}
	return(0);
}

void WaitNoDS1990(unsigned char wait_time)
{
	unsigned char n;

	n = wait_time;
	do {
            delay1ms(25);//Delay10KTCYx(5);
        if (TmReset() != 0) n--; else n = wait_time;
	} while(n != 0);
}


unsigned char ReadDS1990(void)
{
	unsigned int i;
	unsigned char buff[10];

          if (TmReset() != 0) return(1);

        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRFX_PWM_FLAG_STOP);

	Tm_Read();
	for(i = 0; i < 8; i++) buff[i] = tm_buff[i];

        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRF_DRV_PWM_FLAG_LOOP);

	if (TmReset() != 0) return(1);

        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRFX_PWM_FLAG_STOP);
	Tm_Read();

	if (Compare(buff, tm_buff, 8) != 0) 
        {
          nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRF_DRV_PWM_FLAG_LOOP);
          return(2);
        }
        
        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRF_DRV_PWM_FLAG_LOOP);
	if (TmReset() != 0) return(1);
        
        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRFX_PWM_FLAG_STOP);
	Tm_Read();
        nrf_drv_pwm_simple_playback(&m_pwm0, &m_pwm_seq, 1, NRF_DRV_PWM_FLAG_LOOP);
        
	if (Compare(buff, tm_buff, 8) != 0) return(2);
	return(0);
}

unsigned char DetectDS1990(unsigned int time)
{
	unsigned int count;

	count = time;
	do {
		if (TmReset() == 0) count--; else count = time;
	} while (count > 0);
        return 0;
}

unsigned char WaitDS1990(unsigned int time, unsigned int max)
{
	unsigned int count;

	count = time;
	do {
		Delay10KTCYx(5);
		if (TmReset() == 0) count--; else count = time;
		max--;
		if (max == 0) return(1);
	} while (count > 0);
	return(0);
}


unsigned char OneReadDS1990(unsigned char wait_time)
{
	do {
		if (ReadDS1990() == 0) return(DS1990);
 	} while (--wait_time);
	return(NO_KEY); 
}


#ifndef WRITER_RW1990

unsigned char Write_DS(void)
{
    return ow_rw1990_write_rom(wr_buff);
/*    
	unsigned char WrRes, WriteCount;

	WriteCount = 5;
	do {
		WrRes = 0xFF;

 		if (Detect_IZ() == 0)
		{
			WrRes = IZ_WF(1);
			if (WrRes == 0) WrRes = IZ_Write();
			if (WrRes == 0) WrRes = IZ_WF(0);
		}
		else
		{
			if (Detect_TM() == 0)
			{
				WrRes = TM2004Write();
			}
			else
			{
				WrRes = RW_WF(1);
				if (WrRes == 0) WrRes = RW_Write();
				if (WrRes == 0) WrRes = RW_WF(0);
			}
		}

		if (WrRes == 0)
		{ 	
			WrRes = 2;
			if (OneReadDS1990(10) == DS1990) 
			{ 
				if (Compare(tm_buff, wr_buff, 8) == 0) WrRes = 0;
			}
		}
		if (WrRes == 1) return(1);
		WriteCount--; 
	} while ((WriteCount != 0) && (WrRes != 0));
	return(WrRes);*/
}

#else

unsigned char Write_DS(void)
{
 
	unsigned char WrRes, WriteCount;

	WriteCount = 3;
	do {
		WrRes = 0xFF;

 		if (Detect_IZ() == 0)
		{
			WrRes = IZ_WF(1);
			if (WrRes == 0) WrRes = IZ_Write();
			if (WrRes == 0) WrRes = IZ_WF(0);
		}

		else
		{
			if (Detect_TM() == 0)
			{
				WrRes = TM2004Write();
			}
			else
			{
				WrRes = RW_WF(1);
				if (WrRes == 0) WrRes = RW_Write();
				if (WrRes == 0) WrRes = RW_WF(0);
			}
		}

		if (WrRes == 0)
		{ 	
			WrRes = 2;
			if (OneReadDS1990(10) == DS1990) 
			{ 
				if (Compare(tm_buff, wr_buff, 8) == 0) WrRes = 0;
			}
		}
		if (WrRes == 1) return(1);
		WriteCount--; 
	} while ((WriteCount != 0) && (WrRes != 0));
	return(WrRes);
}

#endif

// ------------------------------------------------------------------------
//#pragma romdata
//#pragma code
//#pragma udata
// ------------------------------------------------------------------------
