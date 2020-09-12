#include "nrf_delay.h"
#include "delay.h"
#include "nrf_gpio.h"
#include "utils.h"
//	radix dec

//#define	p_in	PORTA,3
//#define	p_out	LATC,4

//        		EXTERN 	tm_buff, wr_buff
extern unsigned char tm_buff[8];
extern unsigned char wr_buff[8];
//        		EXTERN 	TempL, TempH, DelayL, DelayH
extern unsigned int temp_7_8, Temp_L_H, Delay_L_H;
//        		EXTERN 	temp1, temp2, temp3, temp4, temp5, temp6
//       		EXTERN 	temp7, temp8, temp9, temp10, temp11
static unsigned char temp1, temp2, temp3, temp4, temp5, /*temp6,*/ temp7, temp8, temp9, temp10, temp11;
static unsigned char WREG, POSTINC0, PLUSW1, /*INDF0,*/ FSR0L;

extern unsigned long RF_CNT;
extern unsigned char TmReset(void);
unsigned char DSReset(void);
unsigned char TM2004Write(void);
unsigned char Detect_IZ(void);
void IZ_WF(unsigned char flag);
unsigned char IZ_Write(void);
unsigned char IZ_RF(void);
unsigned char IZ_W_F(void);
unsigned char RW_Write(void);
unsigned char RW_WF(unsigned char flag);
void receive_bit(void);
void delay1(void);
void delay2(unsigned int cnt);
unsigned char byte_rcv(void);
unsigned char byte_rcv(void);
unsigned char CalcCRCTM(void);
void byte_snd(unsigned char a);
void delay(unsigned int cnt);
void send_bit(void);
void send_one(void);
void send_zero(void);
void power(void);
void DelayDS(void);

//Detect_TM:
unsigned char Detect_TM(void)
{

//                rcall	DSReset
                DSReset();
//		bz		ds_err
                if(!DSReset()) return 1;
//                movlw   0x3C
//                rcall   byte_snd
//                movlw   0x00
//                rcall   byte_snd
//                movlw   0x00
//                rcall   byte_snd
//                movlw   0x55
//                rcall   byte_snd
                byte_snd(0x3C);
                byte_snd(0x00);
                byte_snd(0x00);
                byte_snd(0x55);
//                rcall   byte_rcv
//		movwf	temp1
                temp1 = byte_rcv();
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                movlw   d'200'
//                rcall	delay2
                delay2(200);
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//				movf	temp1,w
//				sublw	0xBE                
//				btfss	STATUS,Z
//				retlw	0x02
//				retlw	0x00
                if(temp1 == 0xBE) return 0;
                return 2;

}
//TM2004Write:
unsigned char TM2004Write(void)
{

//                rcall	DSReset                
//		bz		ds_err
                if(!DSReset()) return 1;
//                movlw   0x3C
//                rcall	byte_snd
//                movlw   0x00
//                rcall	byte_snd
//                movlw   0x00
//                rcall	byte_snd
                byte_snd(0x3C);
                byte_snd(0x00);
                byte_snd(0x00);
//                movlw	d'8'
//                movwf	temp3
                temp3 = 8;
//                lfsr	FSR0,wr_buff
//w_loop2:
                do{
//                movf	INDF0,w
//                rcall	byte_snd
                byte_snd(wr_buff[8 - temp3]);  
//                rcall	byte_rcv
                byte_rcv();
//                movlw	82h
//                rcall	delay2
                delay2(0x82);
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                movlw	01h
//                rcall	delay2
                delay2(0x01);
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                movlw 	15h
//                rcall	delay
                delay(0x15);
//                rcall   byte_rcv
                wr_buff[8 - temp3] = byte_rcv();
//                incf    FSR0L,f
//                decfsz  temp3,f
                  temp3 --;
//                bra     w_loop2
                }while(temp3);
//		retlw	0x00
                return 0;
                }
//Detect_IZ:
unsigned char Detect_IZ(void)
{

//				bcf		temp10,0
                                temp10 &= 0xFE;
//				rcall	IZ_W_F
//				sublw	0x01
//				btfsc	STATUS,Z
//				retlw	0x01
                                if(IZ_W_F()) return 1;
//				rcall	IZ_RF
                                IZ_RF();
//				btfsc	WREG,0
//				retlw	0x01
                                if(WREG & 0x01) return 1;
//				btfsc	WREG,7
//				retlw	0x02
                                if(WREG & 0x80) return 2;
//				bsf		temp10,0
                                temp10 |= 0x01;
//				rcall	IZ_W_F                              
//				sublw	0x01
//				btfsc	STATUS,Z
//				retlw	0x01
                                if(!IZ_W_F()) return 1;
//				rcall	IZ_RF
                                IZ_RF();
//				btfsc	WREG,0
//				retlw	0x01
                                if(WREG & 0x01) return 1;
//				btfss	WREG,7
//				retlw	0x02
                                if(WREG & 0x80) return 2;
//				retlw	0x00
                                return 0;
}
//IZ_Write:
unsigned char IZ_Write(void)
{
//                lfsr    FSR0, wr_buff
//                call    DSReset
//				bz		ds_err
                                if(!DSReset()) return 1;
//                movlw   0xD5
//                call    byte_snd
                                byte_snd(0xD5);
//				movlw	0x08
//				movwf	temp7
                                do{
//izs_loop1:
//				movlw	0x08
//				movwf	temp8
                                temp8 = 8;
//				movf	POSTINC0,w
//				movwf	temp9
                                temp9 = POSTINC0;
                                do{
//izs_loop2:
//				btfss	temp9,0	
//                              call	send_one			
//				btfsc	temp9,0			
//                              call	send_zero
                                if(temp9 & 0x01) send_one();	
                                else send_zero();
//				rcall	power
                                power();
//				rrcf	temp9,f	
                                temp9 >>= 1;
//				decfsz	temp8,f
                                  temp8 --;
//				bra		izs_loop2
                                }while(temp8);
//				decfsz	temp7,f
                                temp7 --;
//				bra		izs_loop1
                                }while(temp7);
                                return 0;
//				retlw	0x00
//ds_err:
//				retlw	0x01
}
//IZ_RF:
unsigned char IZ_RF(void)
{
//                call    DSReset
//				bz		ds_err
                if(!DSReset()) return 1;
//                movlw   0xB5
//                call    byte_snd
                byte_snd(0xB5);
//                call    byte_rcv
                byte_rcv();
//				rrncf	WREG,w
                WREG >>= 1;
//??				andlw	0x80
                return 0;
}
//IZ_WF:
void IZ_WF(unsigned char flag)
{
//	    	    movlw   -1
//    	    	movf    PLUSW1,w
//				andlw	0x01
//				movwf	temp10
                temp10 = (PLUSW1 & 0x01);
               IZ_W_F();
}
//IZ_W_F:
unsigned char IZ_W_F(void)
{
//				clrf	temp11
                                temp11 = 0;
//                call    DSReset
//				bz		ds_err
                                if(!DSReset()) return 1;
//		        movlw   0xC1
//				btfss	temp11,0
//		        movlw   0xD1
//                call    byte_snd
                                if(temp11 & 0x01) byte_snd(0xC1);
                                else byte_snd(0xD1);
//				btfss	temp10,0			
//				call	send_one
//				btfsc	temp10,0			
//				call	send_zero
                                if(temp10 & 0x01) send_zero();
                                else send_one();
//				rcall	power
                                power();
//				retlw	0x00
                                  return 0;
}
//RW_Write:
unsigned char RW_Write(void)
{
//                lfsr    FSR0, wr_buff
//                call    DSReset
//				bz		ds_err
                                if(!DSReset()) return 1;
//                movlw   0xD5
//                call    byte_snd
                                byte_snd(0xD5);
//				movlw	0x08
//				movwf	temp7
                                temp7 = 8;
//rws_loop1:
                                do{
//				movlw	0x08
//				movwf	temp8
                                  temp8 = 8;
//				movf	POSTINC0,w
//				movwf	temp9
                                  temp9 = POSTINC0;
//				comf	temp9,f
                                temp9 = 0xFF - temp9;  
//rws_loop2:
                                  do{
//				btfss	temp9,0			
//				call	send_one
//				btfsc	temp9,0			
//				call	send_zero
                                    if(temp9 & 0x01) send_zero();
                                    else send_one();
//				rcall	power
                                  power();
//				rrcf	temp9,f	
                                  temp9 >>= 1;
//				decfsz	temp8,f
                                  temp8 --;
//				bra		rws_loop2
                                  }while(temp8);
//				decfsz	temp7,f
                                temp7 --;
				//bra		rws_loop1
                                }while(temp7);
//				retlw	0x00
                                return 0;
}
//RW_WF:
unsigned char RW_WF(unsigned char flag)
{
//	    	    movlw   -1
//    	    	movf    PLUSW1,w
//				movwf	temp10
                temp10 = (PLUSW1 & 0x01);
//                call    DSReset
//				bz		ds_err
                if(!DSReset()) return 1;
//                movlw   0x1D
//                call    byte_snd
                byte_snd(0x1D);
//				btfsc	temp10,0			
//				call	send_one              
//				btfss	temp10,0			
//				call	send_zero
                if(temp10 & 0x01) send_one();
//				call	power
                power();
//				retlw	0x00
                return 0;
}
//power:
void power(void)
{
//				rcall	DelayDS
//				rcall	DelayDS
                DelayDS();
                DelayDS();
//		return;
}

//DelayDS:
void DelayDS(void)
{
//                movlw   d'255'
//                call    delay2
//                movlw   d'255'
//                call    delay2
//                movlw   d'255'
//                call    delay2
//                movlw   d'255'
//                call    delay2
//                movlw   d'255'
//                call    delay2
  
                  delay2(255);
                  delay2(255);
                  delay2(255);
                  delay2(255);
                  delay2(255);
  
//		return;
}
//TmReset:
/*
unsigned char TmReset(void)
{

}

//res_loop1:
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                movlw   d'240'
//                rcall	delay2
//                delay2(240);
                RF_CNT = 0;
                while(RF_CNT < 100);
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                movlw   d'150'
//                rcall	delay2
//                delay2(150);
                RF_CNT = 0;
                while(RF_CNT < 60);
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                movlw   d'15'
//                rcall	delay2
                //delay2(15);
                RF_CNT = 0;
                while(RF_CNT < 10);
//                btfsc   p_in
//                bra     _no_data
                  nrf_gpio_pin_clear(14);
                if(!nrf_gpio_pin_read(4))
                {
//                movlw   d'60'
//                rcall	delay2
                  //delay2(60);
                  RF_CNT = 0;
                while(RF_CNT < 20);
//				btfss   p_in
//				bra     _no_data
                  nrf_gpio_pin_set(14);
                if(nrf_gpio_pin_read(4))
                {
//                movlw   d'75'
//                rcall	delay2
//                  delay2(75);
//                retlw   0x00
                  return 0;
                }
                }
//_no_data:
//                movlw   d'75'
//                rcall	delay2
                  delay2(75);
//		  retlw	0x01
                  return 1;
}    */                              
//DSReset:
unsigned char DSReset(void)
{
//				movlw	d'100'
//				movwf	temp3
                                temp3 = 100;
//dr_loop:
                                do{
//                              rcall	TmReset
//                                iorlw	0x00
//				bz		dr_exit
                                if(!TmReset()) return 0;
//				decfsz	temp3,f
                                temp3 --;
//				bra		dr_loop
                                }while(temp3);
//				bsf		STATUS,Z
//				retlw	0x01
                                return 1;
//dr_exit:
//				bcf		STATUS,Z
//				retlw	0x00
}
//Tm_Read:
/*
void Tm_Read(void)
{
//                movlw   0x33
//                rcall   byte_snd
                byte_snd(0x33);
//                lfsr    FSR0,tm_buff
//                movlw   d'8'
//                movwf   temp3
                
                temp3 = 8;  
//tr:
                do{
//                rcall   byte_rcv                  
//                movwf   INDF0
                tm_buff[8 - temp3] = byte_rcv();
//                incf    FSR0L,f
//                decfsz  temp3,f
//                bra     tr
                }while(temp3);
//                return

}
*/
//CheckCRC:
unsigned char CheckCRC(void)
{
//				rcall   CalcCRCTM
//				xorwf   tm_buff+7,w
//				bnz     crc_err
//				retlw   0x00
//crc_err:
//				retlw   0x01
                                if(CalcCRCTM() ^ tm_buff[7]) return 1;
                                else return 0;
}
//CalcCRCTM:
unsigned char CalcCRCTM(void)
{
//                lfsr    FSR0,tm_buff
//                clrf    temp4
                temp4 = 0;
//                movlw   d'7'
//                movwf   temp1
                temp1 = 7;
//calc_crc1:
                do{
//                movlw   0x08
//                movwf   temp2
                  temp2 = 8;
//                movf    INDF0,w
//                movwf   temp3
                  temp3 = tm_buff[0];//INDF0;
//calc_crc2:
                  do{
//                xorwf   temp4,w
//                movwf   temp5
                  temp5 = temp4 ^ temp3;
//                rrcf    temp5,w
//                movf    temp4,w
//                btfsc   STATUS,C
//                xorlw   b'00011000'
//                movwf   temp5
                  if(temp5 & 0x01) temp5 = temp4 ^ 0x18;
                  else temp5 = temp4;
//                rrcf    temp5,w                  
//                movwf   temp4
                  temp4 = temp5 >>= 1;
//                bcf     STATUS,C
//                rrcf    temp3,f
                  temp3 >>= 1;
//                movf    temp3,w
//                decfsz  temp2,f
                  temp2 --;
//                bra     calc_crc2
                  }while(temp2);
//                incf    FSR0L,f
                  FSR0L ++;
//                decfsz  temp1,f
                  temp1 --;
//                bra     calc_crc1
                }while(temp1);
//                movf    temp4,w
                return temp4;
//                return
}
//send_bit:
void send_bit(void)
{
//				btfss	WREG,0
//				bra		send_zero
  if(WREG & 0x01) send_one();
  else send_zero();
}
//send_one:
void send_one(void)
{
//                nop
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                nop
//                rcall   delay1
                delay1();
//                nop
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                nop
//                movlw   d'13'
//                rcall   delay2
                delay2(13);
//??				bra		send_oz_exit
}
//send_zero:
void send_zero(void)
{
//                nop
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                nop
//                movlw   d'13'
//                rcall   delay2
                delay2(13);
//                nop
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                nop
//                rcall   delay1
                delay1();
}
//??				bra		$+2
//send_oz_exit:
//??				bra		$+2
//??				bra		$+2
//??				bra		$+2
//??				bra		$+2
//??				bra		$+2
//??				bra		$+2
//				return


//byte_snd:
void byte_snd(unsigned char a)     
{
//                movwf   temp1
                temp1 = a;
//                movlw   d'8'
//                movwf   temp2
                temp2 = 8;
//next_tr_b:
                do{
//                rrcf    temp1,f
//                btfss   STATUS,C
//                bra     _set_in
                if(temp1 & 0x01) 
                {
//                nop
//                bsf     p_out//PORTC,4
//                nop
                  nrf_gpio_pin_set(1);
//                rcall   delay1
                  delay1();
//                nop
// 	            bcf     p_out
//                nop
                  nrf_gpio_pin_clear(3);
//                movlw   d'13'
//                rcall   delay2
                  delay2(13);
//                bra     _skip_set_in
                }
                else
                {
//_set_in:
//                nop
//                bsf     p_out
                  nrf_gpio_pin_set(1);
//                nop
//                movlw   d'13'
//                rcall   delay2
                  delay2(13);
//                nop
                  
//                bcf     p_out
//                nop
                  nrf_gpio_pin_clear(3);
//                rcall   delay1
//                rcall   delay1
//                rcall   delay1
                  delay1();
                  delay1();
                  delay1();
                }
//_skip_set_in:
//                decfsz  temp2,f
//                bra     next_tr_b
                temp2 --;
                }while(temp2);
//                return
}
//receive_bit:
void receive_bit(void)
{
  unsigned char tmp;
//                movlw   d'1'
//                movwf   temp1
                temp1 = 1;
//                clrf    temp2
                temp2 = 0;
//				rcall	next_rc_b
//                next_rc_b();
                do{
//                nop
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                nop
//                rcall   delay1
                delay1();
//                nop
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                nop
//                rcall   delay1
                delay1();
//                bcf     STATUS,C
//                btfsc   p_in
//                bsf     STATUS,C
//                rrcf    temp2,f
                temp2 >>= 1;
                if(nrf_gpio_pin_read(4)) temp2 |= 0x80;
//                movlw   d'9'
//                rcall   delay2
                delay2(9);
//                decfsz  temp1,f
                temp1 -= 1;
//                bra     next_rc_b
                }while(temp1);
//				rlncf	temp2,w
                tmp = temp2;
                temp2 <<= 1;
                if(tmp & 0x80) temp2 |= 0x01;
//				return		
}
//byte_rcv:
unsigned char byte_rcv(void)
{
//                movlw   d'8'
//                movwf   temp1
                temp1 = 8;
//                clrf    temp2
                temp2 = 0;
//next_rc_b:
                do{
//                nop
//                bsf     p_out
                nrf_gpio_pin_set(1);
//                nop
//                rcall   delay1
                delay1();
//                nop
//                bcf     p_out
                nrf_gpio_pin_clear(1);
//                nop
//                rcall   delay1
                delay1();
//                bcf     STATUS,C
//                btfsc   p_in
//                bsf     STATUS,C
//                rrcf    temp2,f
                temp2 >>= 1;
                if(nrf_gpio_pin_read(4)) temp2 |= 0x80;
//                movlw   d'9'
//                rcall   delay2
                delay2(9);
//                decfsz  temp1,f
                temp1 -= 1;
//                bra     next_rc_b
                }while(temp1);
//                movf    temp2,w
                return temp2;
//                return
}

//delay1:
void delay1(void)
{
//				call	delay12
//				call	delay12
//				goto	delay12
//delay12:
//				return	
}

//delay2:
void delay2(unsigned int cnt)
{
  unsigned int i;
  for(i = 0; i < cnt; i ++) nrf_delay_us(4);
//  Delay10TCYx(cnt);
    
//                movwf   DelayL
//l4dl:  			call	x4delay
//                decfsz  DelayL,f
//                bra     l4dl
//                return

//x4delay:
//                nop
//				goto	x4delay1
//x4delay1:
//				return
}

//delay:
void delay(unsigned int cnt)
{
  unsigned int i;
  for(i = 0; i < cnt; i ++) delay2(253);
//				movwf   TempH
//delay_loop1:
//				movlw	d'20'
//				movwf	TempL
//delay_loop2:
//				movlw	d'253'
//				rcall   delay2
//				decfsz  TempL,f
//				bra     delay_loop2
//				decfsz  TempH,f
//				bra     delay_loop1
//				return
}
