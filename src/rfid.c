//#include "p18cxxx.h"
#include "utils.h"
#include "delay.h"
#include "rfid.h"
#include "rfid_tx.h"
#include "lcd.h"
#include "nrfx_pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

// ------------------------------------------------------------------------
//#pragma udata 	MAIN_RAM
//#pragma romdata MAIN_DATA
//#pragma code 	MAIN_CODE
// ------------------------------------------------------------------------

void SendOne(void);
void SendZero(void);
void FieldOn(void);
void FieldOff(void);

extern unsigned int TC;
unsigned char TXBuff[8];
extern RFIDflags FLAG;
extern unsigned long RF_CNT;
extern unsigned char str[];
extern nrf_drv_pwm_config_t config0;
extern nrf_pwm_values_individual_t m_pwm_seq_values;

extern rfid_t RFID;
extern nrf_drv_pwm_t const m_pwm0;
extern void pwm_handler(nrf_drv_pwm_evt_type_t event_type);
extern nrf_pwm_sequence_t const    m_pwm_seq;

#define START_GAP	38//350//d'350'		; 200
#define WRITE_GAP	20//330//d'330'		; 180

#define DATA_0		25//115//d'115'		; 180 160
#define DATA_1		55//335//d'335'		; 400 380


void RFRestart(void)
{
  FieldOff();
  RF_CNT = 0;
 while(RF_CNT < 25);
 FieldOn();

}

unsigned char RFIDWriteEM4100(void)
{
       
  TXBuff[4] = RFID.serialNumber.b[0];
  TXBuff[3] = RFID.serialNumber.b[1];
  TXBuff[2] = RFID.serialNumber.b[2];
  TXBuff[1] = RFID.serialNumber.b[3];
  TXBuff[0] = RFID.serialNumber.b[4];

  WriteCardID(TXBuff);
  

	RFRestart();

        if(!ReadRFID())
                  return(1);
	if ((TXBuff[0] == RFID.serialNumber.b[4]) && (TXBuff[1] == RFID.serialNumber.b[3]) && (TXBuff[2] == RFID.serialNumber.b[2]) && (TXBuff[3] == RFID.serialNumber.b[1]) && (TXBuff[4] == RFID.serialNumber.b[0])) return(0);
	return(1);	
}


unsigned char WriteRFID(unsigned char time)
{
	do {
		if (RFIDWriteEM4100() == 0) 
                  return(0);
	} while (--time);
	return(2);
}

unsigned char ReadRFID(void)
{
        RFID.serialNumber.b[4] = 0;
        if(FLAG == RFID_NO) return(NO_KEY);
        FLAG = RFID_read;
        RF_CNT = 0;
        while(FLAG != Read_OK && FLAG != RFID_NO && RF_CNT < 10000);
        if(FLAG == Read_OK)
        {

            Beep();
	return(EM4100);
        }

        return(NO_KEY);
}


void RFIDFree(unsigned char time)
{
        RF_CNT = 0;
        while(RF_CNT < 4000 * time);
  
}
//----------------------------------------------------------------------------------------------------
//включить поле
//----------------------------------------------------------------------------------------------------
void FieldOn(void)
{
  m_pwm_seq_values.channel_0 = 32;
}
//----------------------------------------------------------------------------------------------------
//отключить поле
//----------------------------------------------------------------------------------------------------
void FieldOff(void)
{
  m_pwm_seq_values.channel_0 = 64;
}

//----------------------------------------------------------------------------------------------------
//пауза Start Gap
//----------------------------------------------------------------------------------------------------
void StartGap(void)
{
 FieldOff();
 RF_CNT = 0;
 while(RF_CNT < 38);
 FieldOn();
}
 
//----------------------------------------------------------------------------------------------------
//отправить единицу
//----------------------------------------------------------------------------------------------------
void SendOne(void)
{
  RF_CNT = 0;
 while(RF_CNT < DATA_1);
// nrf_delay_us(420);
 FieldOff();
 RF_CNT = 0;
 while(RF_CNT < WRITE_GAP);
// nrf_delay_us(300);
 FieldOn();        
}
//----------------------------------------------------------------------------------------------------
//отправить ноль
//----------------------------------------------------------------------------------------------------
void SendZero(void)
{
  RF_CNT = 0;
 while(RF_CNT < DATA_0);
// nrf_delay_us(150);
 FieldOff();
 RF_CNT = 0;
 while(RF_CNT < WRITE_GAP);
// nrf_delay_us(300);
 FieldOn();        
}
//----------------------------------------------------------------------------------------------------
//отправить код lock bit
//----------------------------------------------------------------------------------------------------
void OpCode_LockBit(void)
{
 SendOne();
 SendZero();
 SendZero();
}
//----------------------------------------------------------------------------------------------------
//отправить сброс
//----------------------------------------------------------------------------------------------------
void WriteReset(void)
{
 SendZero();
 SendZero();
}
//----------------------------------------------------------------------------------------------------
//выбрать адрес
//----------------------------------------------------------------------------------------------------
void AddressSelect(unsigned char data)
{
 if (data&(1<<2)) SendOne(); 
              else SendZero();
 if (data&(1<<1)) SendOne();
              else SendZero();
 if (data&(1<<0)) SendOne();
              else SendZero();
}
//----------------------------------------------------------------------------------------------------
//записать страницу
//----------------------------------------------------------------------------------------------------
void WritePage(unsigned char *data)
{
 int i;
 int j;
 StartGap();
 OpCode_LockBit();
 for(i=0;i<4;i++) 
 {
  for(j=7;j>=0;j--) 
  {
   if (data[i]&(1<<j)) SendOne();
                   else SendZero();
  }
 } 
 AddressSelect(1);
 RF_CNT = 0;
 while(RF_CNT < 12500);
// nrf_delay_ms(100);
 StartGap();
 OpCode_LockBit();
 for(i=4;i<8;i++) 
 {
  for(j=7;j>=0;j--) 
  {
   if (data[i]&(1<<j)) SendOne();
                   else SendZero();
  }
 }
 AddressSelect(2);
 //nrf_delay_ms(100);
 RF_CNT = 0;
 while(RF_CNT < 12500);
}
//----------------------------------------------------------------------------------------------------
//записать конфигурацию
//----------------------------------------------------------------------------------------------------
void WriteConfig(unsigned char *data)
{
 int i;
 int j;
 StartGap();
 OpCode_LockBit();
 for (i=0;i<4;i++) 
 {
  for (j=7;j>=0;j--) 
  {
   if (data[i]&(1<<j)) SendOne(); 
                   else SendZero(); 
  }
 }
 AddressSelect(0);
 //nrf_delay_ms(100);
 RF_CNT = 0;
 while(RF_CNT < 12500);
}
//----------------------------------------------------------------------------------------------------
//вычислить CRC
//----------------------------------------------------------------------------------------------------
unsigned char CalculateCRC(unsigned char *buffer)
{
 int i;
 unsigned char crc=0;
 for(i=0;i<5;i++)
 {
  crc=crc^(buffer[i]&0x0F)^((buffer[i]>>4)&0x0F); 
 }
 return(crc); 
} 
//----------------------------------------------------------------------------------------------------
//подготовить буфер передачи
//----------------------------------------------------------------------------------------------------
void PrepareSendBuffer(unsigned char *source_data,unsigned char *send_data)
{
 unsigned char source_bits=0;
 unsigned char send_bits=9;
 send_data[0]=0xFF;//первые 8 единиц
 send_data[1]=0x80;//в старшем бите ещё одна единица
 send_data[2]=0;
 send_data[3]=0;
 send_data[4]=0;
 send_data[5]=0;
 send_data[6]=0;
 send_data[7]=0;
 do  
 {
  char j=0;
  unsigned char send_crc=0;
  do 
  {
   unsigned char bit=0;
   if (source_data[source_bits/8]&(1<<(7-(source_bits%8)))) bit=1;
   send_crc=send_crc^bit;
   if (bit==1) send_data[send_bits/8]|=(1<<(7-(send_bits%8)));
   source_bits++;
   send_bits++;
   j++;
  } while(j<4);
  if (send_crc==1) send_data[send_bits/8]|=(1<<(7-send_bits%8));
  send_bits++;
 } while(source_bits<40);
 send_data[7]|=(CalculateCRC(source_data)<<1);
}

//----------------------------------------------------------------------------------------------------
//записать ID карты
//----------------------------------------------------------------------------------------------------
void WriteCardID(unsigned char id[5])
{
 FieldOn();//включаем электромагнитное поле
 //nrf_delay_ms(500);
 RF_CNT = 0;
 while(RF_CNT < 62);
 //запись карты
 unsigned char RFID_Send_Buffer[8];//буффер записи RFID
 unsigned char RFID_Config[4];//буфер конфигурации RFID
 //задаём конфигурацию карты 
 RFID_Config[0]=0x10;
 RFID_Config[1]=0x14;
 RFID_Config[2]=0x80;
 RFID_Config[3]=0x40;
 //задаём номер карты 
 //RFIDBuffer[0] = tm_buff[1];
 //RFIDBuffer[1] = tm_buff[2];
 //RFIDBuffer[2] = tm_buff[3]
 //RFIDBuffer[3] = tm_buff[4];
 //RFIDBuffer[4] = tm_buff[5]; 
 //перекодируем данные и подсчитаем контрольную сумму
 PrepareSendBuffer(id, RFID_Send_Buffer);
 WritePage(RFID_Send_Buffer);  
 WriteConfig(RFID_Config);  
 StartGap();
 WriteReset();
 //передаём ответ
 //SendAnswer((unsigned char*)"OK WRITE ID",11);*/ 
 //_delay_ms(500);
 //FieldOff();//отключаем электромагнитное поле 
}

