//#include "P18cXXX.INC"
#include "nrf_gpio.h"
#include "nrfx_timer.h"
#include "nrf_drv_timer.h"
//	radix dec


//#define	p_in		PORTC,RC6
#define	bit_b		0x40//b'01000000'
#define MINVAL      32 * 50
#define MAXVAL      32 * 70
#define	BYTESMAX	18

extern const nrf_drv_timer_t TIMER1;

//MAIN_ACS		UDATA_ACS
static unsigned char PIR1, PIR2, STATUS, WREG, POSTINC0, FSR0, INDF0, /*PLUSW1, PSTRCON,*/ FSR0L;
unsigned char data_rx[10];//			RES		d'10'
unsigned char tm_buff[8];//			RES		d'8'
unsigned char wr_buff[8];//			RES		d'8'
static unsigned char
temp1,//			RES		1
temp2,//			RES		1
temp3,//			RES		1
temp4,//			RES		1
temp5,//			RES		1
temp6,//			RES		1
temp11
;
unsigned int temp_9_10, temp_7_8, Temp_L_H, Delay_L_H;
/*        		GLOBAL 	tm_buff, wr_buff
        		GLOBAL 	TempL, TempH, DelayL, DelayH
        		GLOBAL 	temp1, temp2, temp3, temp4, temp5, temp6
        		GLOBAL 	temp7, temp8, temp9, temp10, temp11
*/

extern unsigned char RF_IN_U, RF_IN_D;


unsigned char id_array[BYTESMAX];//		RES		BYTESMAX + 1
			
unsigned char RxSync(void);
void wait_period(void);
unsigned char get_period(void);
unsigned char ReciveM(unsigned char c);
unsigned int ReciveRFID(void);
unsigned int Decode_EM4100(void);
void not_buff(void);
void scroll(void);
unsigned char rrncfWREG(unsigned char a, unsigned char cnt);
void HPC(void);

//MAIN_CODE		CODE 

unsigned char ScanRFID(void)
{
//				GLOBAL	ScanRFID

//				movlw	d'5'
//				movwf	temp1
  temp1 = 5;
//				movlw	d'8'
//				movwf	temp4
  temp4 = 8;
//				lfsr	FSR0,id_array
                                FSR0 = id_array[0];
//				rcall	RxSync
                                if(RxSync()) return 1;
//				bz		sr_error
//                                if(STATUS & 0x04) return 1;
//				bra		sr_inp
                                wait_period();
                                temp4 --;
                                do{
                                  do{
//sr_loop1:
//				rcall	get_period
                                get_period();  
//				bz		sr_error
                                if(STATUS & 0x04) return 1;
//sr_inp:
//				rcall	wait_period
                                wait_period();
//				decfsz	temp4,f 
                                temp4 --;
//				bra		sr_loop1
                                 }while(temp4);                                
//				bsf		temp4,3
                                temp4 |= 0x08;
//				decfsz	temp1,f
                                temp1 --;
//				bra		sr_loop1
                                  }while(temp1);                               
//				retlw	0x00
                                return 0;  
//sr_error:
//				retlw	0x01


}
//ReciveRFID:
unsigned int ReciveRFID(void) 
{
//				GLOBAL	ReciveRFID

//				movlw	BYTESMAX
//				call	ReciveM
                                ReciveM(BYTESMAX);
//				btfsc	STATUS,Z
//				retlw	0x01
                                if(STATUS & 0x04) return 1;
//				call	Decode_EM4100
                                Decode_EM4100();
//				iorlw	0x00
//				bz		rr_found
                                if(STATUS & 0x04) return 0;
//				call	not_buff
                                not_buff();
//				call	Decode_EM4100
                                Decode_EM4100();
//				iorlw	0x00
//				bz		rr_found
                                if(STATUS & 0x04) return 0;
//				retlw	0x01
                                return 1;
//rr_found:
//				retlw	0x00
}
//ReciveM:
unsigned char ReciveM(unsigned char c)
{
unsigned char tmp;
//				movwf	temp1
                                temp1 = c;
//				movlw	d'8'
//				movwf	temp4
                                temp4 = 8;
//				lfsr	FSR0, id_array
                                FSR0 = id_array[0];
//				rcall	RxSync
                                RxSync();
//				bz		rm_error
//                              bra		rm_inp
                                if(STATUS & 0x04) return 1;
				else goto rm_inp;
                                do{
rm_loop1:
//				rcall	get_period
                                get_period();  
//				bz		rm_error
                                if(STATUS & 0x04) return 1;
rm_inp:
//				rcall	wait_period
                                wait_period();
//				bcf		STATUS,C
                                STATUS &= 0xFE;
//				btfsc	p_in
//				bsf		STATUS,C
                                if(nrf_gpio_pin_read(11)) STATUS |= 0x01;
//				rlcf	INDF0,f
                                tmp = INDF0;
                                INDF0 <<= 1;
                                if(tmp & 0x80) INDF0 |= 0x01;

//				decfsz	temp4,f
//				bra		rm_loop1
                                if(temp4) goto rm_loop1;
//??				movf	POSTINC0,w
//				bsf		temp4,3
                                temp4 |= 0x08;
//				decfsz	temp1,f
//				bra		rm_loop1
                                }while(temp1);
//				bcf		STATUS,Z
                                STATUS &= 0xFB;
//				retlw	0x00
                                return 0;
//rm_error:
//				bsf		STATUS,Z
//				retlw	0x01
}
//RxSync:
unsigned char RxSync(void)
{
unsigned int tmp;
//				movlw	d'130'
//				movwf	temp2
                                temp2 = 130;
//				clrf	TMR1H
//				clrf	TMR1L
                                nrfx_timer_clear(&TIMER1);
//				clrf	temp3
                                temp3 = 0;  
//				btfsc	p_in//RF_IN                                
//				decf	temp3,f
                                if(nrf_gpio_pin_read(11)) temp3 --;
//				clrf	TMR1H
//				clrf	TMR1L
                                nrfx_timer_clear(&TIMER1);
//				bcf		PIR1, TMR1IF
                                PIR1 &= 0xFE;
//				call	get_period
//				call	get_period
//				call	get_period
//                                get_period();
//                                get_period();
//                                get_period();
                                if(get_period()) return 1;
//				bz		rs_error
//                                if(STATUS & 0x04) return;
//rs_loop:
                                do{
//				movff	temp9, temp5
//				movff	temp10, temp6
                                temp5 = temp_9_10;
                                temp6 = (temp_9_10 >> 8);
//				dcfsnz	temp2,f                               
//				bra		rs_error
                                temp2 --;
                                if(!temp2)
                                {
                                  STATUS |= 0x04;
                                  return 1;
                                }
//				call	get_period
                                if(get_period()) return 1;
//				bz		rs_error
  //                              if(STATUS & 0x04) return 1;
//				movff	temp9, temp7
//				movff	temp10, temp8
                                temp_7_8 = temp_9_10;
//				bcf		STATUS,C
                                STATUS &= 0xFE;
//				rrcf	temp8,f
//				rrcf	temp7,f
                                tmp = temp_7_8;
                                temp_7_8 >>= 1;                                
//				movf	temp9,w
//				subwf	temp5,w
//				movwf	TempL
//				movf	temp10,w
//				subwfb	temp6,w
//                              movwf	TempH
                                tmp = temp6;
                                tmp <<= 8;
                                tmp += temp5;
                                tmp -= temp_9_10;
                                Temp_L_H = tmp;
				
//				bcf		temp11,0
                                temp11 &= 0xFE;
//				btfss	TempH,7
//				bra		rs_plus
                                if(!(Temp_L_H & 0x8000))
                                { 
//				bsf		temp11,0
                                temp11 |= 0x01;
//				movf	TempL,w	
//				sublw	0x00
//				movwf	TempL
//				movlw	0x00
//				subfwb	TempH,f
                                Temp_L_H = 0 - Temp_L_H;
                                }
//rs_plus:
//				movf	TempL,w
//				subwf	temp7,w
//				movf	TempH,w
//				subwfb	temp8,w
//				bc		rs_loop
                                if(temp_7_8 >= Temp_L_H) temp_7_8 -= Temp_L_H;
                                }while(temp_7_8 >= Temp_L_H);
//				btfsc	temp11,0
//				bra		rs_skip1
                                if(temp11 & 0x01)
                                {
//				movff	temp9, temp5
//				movff	temp10, temp6
                                temp5 = temp_9_10;
                                temp6 = (temp_9_10 >> 8);  
//				call	get_period
                                get_period();
//				bz		rs_error
                                if(STATUS & 0x04) return 1;
//				movf	temp5,w
//				addwf	temp9,f
//				movf	temp6,w
//				addwfc	temp10,f
                                
                                tmp = temp6;
                                tmp <<= 8;
                                tmp += temp5;
                                tmp += temp_9_10;
                                temp5 = tmp;
                                temp6 = (tmp >> 8);
                                }
//rs_skip1:
//				bcf		STATUS,C
//				rrcf	temp10,f
//				rrcf	temp9,f
                                temp_9_10 >>= 1;
//				movff	temp10, DelayH
//				movff	temp9, DelayL
                                Delay_L_H = temp_9_10;
/*                                  
;				movf	TMR1L,w
;				movlw	d'23'
;				btfsc	STATUS,Z
;				btg		PLUSW0,0
*/
//				bcf		STATUS,C
//				rrcf	temp10,f
//				rrcf	temp9,f
                                temp_9_10 >>= 1;
//				bcf		STATUS,C
//				rrcf	temp10,f
//				rrcf	temp9,f
                                temp_9_10 >>= 1;
//				movf	temp9,w
//				addwf	DelayL,f
//				movf	temp10,w
//				addwfc	DelayH,f
                                Delay_L_H += temp_9_10;
//				movf	DelayL,w
//				sublw	0x00
//				movwf	DelayL
//				movlw	0x00
//				subfwb	DelayH,f
                                Delay_L_H = 0 - Delay_L_H;
//				bcf		STATUS,Z
                                STATUS &= 0x04;
				return 0;
//rs_error:
//				bsf		STATUS,Z
//				return
}                                  
unsigned char get_period(void)//get_period:
{
  
//				movf	PORTC,w
//				andlw	bit_b 
//				movwf	temp3
//gp_loop1:
//				btfsc	PIR1, TMR1IF
//				bra		gp_error

//				movf	temp3,w
//				xorwf	PORTC,w
//				andlw	bit_b
//				btfsc	STATUS,Z
//				bra		gp_loop1 
//??				movff	TMR1L, temp9
//??				movff	TMR1H, temp10
//				clrf	TMR1H
//				clrf	TMR1L
                                nrfx_timer_clear(&TIMER1);
                                return 0;
//				bcf		PIR1, TMR1IF
//                                PIR1 &= 0xFE;
/*;				movlw	low(MINVAL)
;				subwf	temp9,w
;				movlw	high(MINVAL)
;				subwfb	temp10,w
;				bnc		gp_error
*/
//				movlw	low(MAXVAL)
//				subwf	temp9,w
//				movlw	high(MAXVAL)
//				subwfb	temp10,w


//				bc		gp_error
//gp_ok:
//				bcf		STATUS,Z
/*                                STATUS &= 0xFB;     
				return;
*/
//gp_error:
//				bsf		STATUS,Z
//				return
}

//wait_period:
void wait_period(void)
{
//??				movff	DelayH, TMR3H
//??				movff	DelayL, TMR3L
//				bcf		PIR2, TMR3IF
                                PIR2 &= 0xFD;
  
//wa_loop1:
                                while(!(PIR2 & 0x02));
//				btfss	PIR2, TMR3IF
//				bra		wa_loop1
				return;
}

//Decode_EM4100:
unsigned int Decode_EM4100(void)
{
unsigned char n = 0, em64_found;
//				movlw	BYTESMAX
//				movwf	temp1
                                temp1 = BYTESMAX;
//em64_search_loop:
//				movlw	d'8'
//				movwf	temp4
                                do{
                                temp4 = 8;
//em64_search_loop1:
                                  do{
//				call	scroll
                                scroll();    
//rm_next9:
//				movff	id_array+0x00,WREG
//				andlw	b'00000011'
//				sublw	b'00000001'
                                id_array[0] += 0x03;
                                id_array[0] -= 0x01;
//				bnz		em64_next
                                if(!id_array[0]) 
                                {
//				movff	id_array+1,WREG
//				sublw	b'11111111'
                                  id_array[1] = 0xFF - id_array[n];
//				bz		em64_found
                                  em64_found = 0;
                                if(!id_array[1]) 
                                {
                                  em64_found = 1;
                                  break;
                                }
                                }
                                
em64_next:
//				decfsz	temp4,f
//				bra		em64_search_loop1
                                  temp4 --;
                                  }while(temp4);
                                  
                                if(em64_found) break;
//				decfsz	temp1,f
//				bra		em64_search_loop
                                temp1 --;
                                }while(temp1);
//				bra		em64_error
                                if(!em64_found) return 1;
//em64_found:
//				clrf	temp3
                                temp3 = 0;
//??				movlw	d'10'
//??				movlb	temp5
//??				movwf	temp5
//??                                temp5 &= 0x0F;
//				lfsr	FSR0,data_rx
//em_64_loop1:
                                do{
//				movff	id_array+2,WREG
//				call	HPC
                                HPC();
//				bnz		em64_next
                                if(STATUS & 0x04) goto em64_next;
//				movlb	id_array+2
//				btfsc	id_array+2,7,1
//				btg		temp3,7
                                if(id_array[2] & 0x80)
                                {
                                  if(temp3 & 0x80) temp3 &= 0x7F;
                                  else temp3 |= 0x80;
                                }
//				btfsc	id_array+2,6,1
//				btg		temp3,6
                                if(id_array[2] & 0x40)
                                {
                                  if(temp3 & 0x40) temp3 &= 0xBF;
                                  else temp3 |= 0x40;
                                }
//				btfsc	id_array+2,5,1
//				btg		temp3,5
                                if(id_array[2] & 0x20)
                                {
                                  if(temp3 & 0x20) temp3 &= 0xDF;
                                  else temp3 |= 0x20;
                                }
//				btfsc	id_array+2,4,1
//				btg		temp3,4
                                if(id_array[2] & 0x10)
                                {
                                  if(temp3 & 0x10) temp3 &= 0xEF;
                                  else temp3 |= 0x10;
                                }
//				movf	id_array+2,w,1
//				rrncf	WREG,w
//				rrncf	WREG,w
//				rrncf	WREG,w
//				rrncf	WREG,w
//				andlw	b'00001111'
//				movwf	POSTINC0
                                POSTINC0 = rrncfWREG(id_array[2], 4);
                                POSTINC0 &= 0x0F;
//				call	scroll
//				call	scroll
//				call	scroll
//				call	scroll
//				call	scroll
//??				movlb	temp5
//				decfsz	temp5,f
//				bra		em_64_loop1
                                }while(temp5);
//				movff	id_array+2,WREG
                                WREG = id_array[2];
//				xorwf	temp3,w
                                WREG ^= temp3;
//				andlw	b'11110000'
//				bnz		em64_next
                                if(WREG) goto em64_next;
//??				movlb	data_rx+0
//??				swapf	data_rx+0,w
//??				movlb	data_rx+1
//??				iorwf	data_rx+1,w
//??				movwf	tm_buff+5
//??				movlb	data_rx+2
//??				swapf	data_rx+2,w
//??				movlb	data_rx+3
//??				iorwf	data_rx+3,w
//??				movwf	tm_buff+4
//??				movlb	data_rx+4
//??				swapf	data_rx+4,w
//??				movlb	data_rx+5
//??				iorwf	data_rx+5,w
//??				movwf	tm_buff+3
//??				movlb	data_rx+6
//??				swapf	data_rx+6,w
//??				movlb	data_rx+7
//??				iorwf	data_rx+7,w
//??				movwf	tm_buff+2
//??				movlb	data_rx+8
//??				swapf	data_rx+8,w
//??				movlb	data_rx+9
//??				iorwf	data_rx+9,w
//??				movwf	tm_buff+1
//				retlw	0x00
                                return 0;
//em64_error:
//				retlw	0x01
}

//HPC:
void HPC(void)
{ 
//				movlb	temp2
//				movwf	temp6
                                temp6 = temp2;
//??                              temp6 &= 0x0F;
//				clrf	temp2
                                temp2 = 0;
//				btfsc	temp6,7
//				incf	temp2,f
                                if(temp6 & 0x80) temp2 ++;
//				btfsc	temp6,6
//				incf	temp2,f
                                if(temp6 & 0x40) temp2 ++;
//				btfsc	temp6,5
//				incf	temp2,f
                                if(temp6 & 0x20) temp2 ++;
//				btfsc	temp6,4
//				incf	temp2,f
                                if(temp6 & 0x10) temp2 ++;
//??				movf	temp6,w
//??				rrcf	WREG,w
//??				rrcf	WREG,w
//??				rrcf	WREG,w
//??				xorwf	temp2,w
//??				andlw	0x01
                                temp2 &= 0x01;
				return;
}

//scroll:
void scroll(void)
{
//				movff	FSR0L,saveFSR0L
//				movff	FSR0H,saveFSR0H
//				lfsr	FSR0,id_array + (BYTESMAX - 1)
//				movlb	id_array
//				rlcf	id_array,w,1
//				movlw	BYTESMAX
//rl_loop:
                                do{
//				rlcf	POSTDEC0,f
//				decfsz	WREG,w
//				bra		rl_loop
                                WREG --;
                                }while(WREG);
//				movff	saveFSR0L,FSR0L
//				movff	saveFSR0H,FSR0H
				return;
}
//not_buff:
void not_buff(void) 
{
//				lfsr	FSR0,id_array
//				movlw	BYTESMAX
                                WREG = 18;// - BYTESMAX;
//cm_loop:
                                do{
//				comf	POSTINC0,f
//				decfsz	WREG,w
//				bra		cm_loop
                                WREG --;
                                }while(WREG);
				return;

}
unsigned char rrncfWREG(unsigned char a, unsigned char cnt)
{
 unsigned char i, tmp;
    cnt &= 0x7;
    for(i = 0; i < cnt; i ++)
    {
      tmp = a;
      a >>= 1;
      if(tmp & 0x01) a |= 0x80;
    }
  return a;
}



