
//#include "P18cXXX.INC"

//	radix dec

//#define	port_out	PSTRCON, 2

#define START_GAP	38//350//d'350'		; 200
#define WRITE_GAP	20//330//d'330'		; 180

#define DATA_0		25//115//d'115'		; 180 160
#define DATA_1		55//335//d'335'		; 400 380

extern	unsigned char temp1, temp2, temp3, temp4, temp5, PSTRCON, INDF0, STATUS, PLUSW1, POSTINC0;
extern	unsigned char TXBuff[];
extern	unsigned char wr_buff[];

void em_next_bit(void);
void EMbyte(void);
void WriteRF(void);

//MAIN_CODE		CODE	
void DelayUs(unsigned int cnt)
{
//DelayUs 	macro time
//				movlw	high(0x10004-(time/2))
//				movwf	TMR3H,0 				
//				movlw	low(0x10004-(time/2))
//				movwf	TMR3L,0 				
//				bcf		PIR2,1,0 				
//			 	btfss 	PIR2,1,0 				
//				bra		$-2
//			endm
}
//TX_bit:
void TX_bit(void)
{
//				btfsc	STATUS,Z
//				bra		tx_0
//				nop
                                if(STATUS & 0x04)
                                {
//				DelayUs	DATA_1
                                DelayUs(DATA_1);
//				bra		TX_WrGap
//                                bcf		port_out
                                PSTRCON &= 0xFB;
//				DelayUs	WRITE_GAP
                                DelayUs(WRITE_GAP);
//				bsf		port_out
                                PSTRCON |= 0x04;
//				return
                                return;
                                }
//tx_0:
//				DelayUs	DATA_0
                                DelayUs(DATA_0);
//				bra		TX_WrGap
//TX_WrGap:
//				bcf		port_out
                                PSTRCON &= 0xFB;
//				DelayUs	WRITE_GAP
                                DelayUs(WRITE_GAP);
//				bsf		port_out
                                PSTRCON |= 0x04;
//				return
//                                return;
//TX_StGap:

//				bcf		port_out
//                                PSTRCON &= 0xFB;
//				DelayUs	START_GAP
//                                DelayUs(START_GAP);
//				bsf		port_out
//                                PSTRCON |= 0x04;
//				return
//                                  return;
}                                
//WriteRF:
void WriteRF(void)
{
//    			movlw   -1
//    			movff   PLUSW1,temp1
                                temp1 = PLUSW1;
//				lfsr	FSR0,TXBuff
//				movlw	d'4'
//				movwf	temp5
                                temp5 = 4;
//				rcall	TX_StGap
                                    {
                                    PSTRCON &= 0xFB;
                                    DelayUs(START_GAP);
                                    PSTRCON |= 0x04;
//                                    return;
                                    }
//??				bra		$+2			; ?
//??				bra		$+2
//				bcf		STATUS,Z
                                STATUS &= 0xFB;    
//				rcall	TX_bit
                                TX_bit();
//??				bra		$+2
//				nop
//				movf	temp1,w
//				andlw	0x80
                                if(temp1 & 0x80) STATUS &= 0xFB;
                                else STATUS |= 0x04;
//				rcall	TX_bit
                                TX_bit();
//??				bra		$+2
//??				bra		$+2
//				bsf		STATUS,Z
                                STATUS |= 0x04;
//				rcall	TX_bit
                                TX_bit();
//??				bra		$+2
//				nop
//t5wb_loop:
                                do{
//				movf	INDF0,w
//				andlw	0x80
                                  if(INDF0 & 0x80) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x40
                                  if(INDF0 & 0x40) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x20
                                  if(INDF0 & 0x20) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x10
                                  if(INDF0 & 0x10) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x08
				if(INDF0 & 0x08) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x04
				if(INDF0 & 0x04) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	INDF0,w
//				andlw	0x02
				if(INDF0 & 0x02) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	POSTINC0,w
//				andlw	0x01
                                  if(POSTINC0 & 0x02) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				decfsz	temp5,f
                                  temp5 --;
//				bra		t5wb_loop
                                  }while(temp5);
//				nop
//				movf	temp1,w
//				andlw	b'00000100'
				if(temp1 & 0x04) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	temp1,w
//				andlw	b'00000010'
				if(temp1 & 0x02) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
//				rcall	TX_bit
                                  TX_bit();
//				bra		$+2
//				nop
//				movf	temp1,w
//				andlw	b'00000001'
//				bra		TX_bit
                                  if(temp1 & 0x01) STATUS &= 0xFB;
                                  else STATUS |= 0x04;
                                  TX_bit();

}
//PackEM:
void PackEM(void)
{
//				lfsr	FSR0,TXBuff
//				movlw	b'11111111'
//				movwf	POSTINC0
                                TXBuff[0] = 0xFF;
//				movlw	b'10000000'
//				movwf	POSTINC0
                                TXBuff[1] = 0x80;
//				movlw	b'00000000'
//				movwf	POSTINC0
                                TXBuff[2] = 0;
//				movwf	POSTINC0
                                TXBuff[3] = 0;
//				movwf	POSTINC0
                                TXBuff[4] = 0;
//				movwf	POSTINC0
                                TXBuff[5] = 0;
//				movwf	POSTINC0
                                TXBuff[6] = 0;
//				movwf	POSTINC0
                                TXBuff[7] = 0;
//				lfsr	FSR0,TXBuff + 1
//				movlw	0x80
//				movwf	temp2
                                temp2 = 0x80;
//				clrf	temp3
                                temp3 = 0;
//				swapf	wr_buff+5,w
                                temp4 = wr_buff[5];
                                temp4 <<= 4;
                                temp4 += (wr_buff[5] >> 4);
//				rcall	EMbyte
                                EMbyte();
//				movf	wr_buff+5,w
                                temp4 = wr_buff[5];
//				rcall	EMbyte
                                EMbyte();
//				swapf	wr_buff+4,w
				temp4 = wr_buff[4];
                                temp4 <<= 4;
                                temp4 += (wr_buff[4] >> 4);
//				rcall	EMbyte
                                EMbyte();
//				movf	wr_buff+4,w
				temp4 = wr_buff[4];
//				rcall	EMbyte
                                EMbyte();
//				swapf	wr_buff+3,w
				temp4 = wr_buff[3];
                                temp4 <<= 4;
                                temp4 += (wr_buff[3] >> 4);
//				rcall	EMbyte
                                EMbyte();
//				movf	wr_buff+3,w
				temp4 = wr_buff[3];
//				rcall	EMbyte
                                EMbyte();
//				swapf	wr_buff+2,w
				temp4 = wr_buff[2];
                                temp4 <<= 4;
                                temp4 += (wr_buff[2] >> 4);
//				rcall	EMbyte
                                EMbyte();
//				movf	wr_buff+2,w
				temp4 = wr_buff[3];
//				rcall	EMbyte
                                EMbyte();
//				swapf	wr_buff+1,w
				temp4 = wr_buff[1];
                                temp4 <<= 4;
                                temp4 += (wr_buff[1] >> 4);
//				rcall	EMbyte
                                EMbyte();
//				movf	wr_buff+1,w
				temp4 = wr_buff[1];
//				rcall	EMbyte
                                EMbyte();
//				movf	temp3,w
                                temp4 = temp3;
//				rcall	EMbyte
                                EMbyte();
//				comf	temp2,w
                                temp2 = 0xFF - temp2;
//				andwf	INDF0,f
                                INDF0 &= temp2;
//				return
}
//EMbyte:
void EMbyte(void)
{
//				movwf	temp4
//				rcall	em_next_bit
                                em_next_bit();
//				clrf	temp1
                                temp1 = 0;
//				btfss	temp4,3
//				bra		em4p_skip1
                                if(temp4 & 0x08)
                                {
//				btg		temp1,0
                                  if(temp1 & 0x01) temp1 &= 0xFE;
                                  else temp1 |= 0x01;
//				btg		temp3,3
                                  if(temp3 & 0x08) temp3 &= 0xF7;
                                  else temp3 |= 0x08;
//				movf	temp2,w
//				iorwf	INDF0,f
                                  INDF0 |= temp2;  
                                }
//em4p_skip1:
//				rcall	em_next_bit
                                em_next_bit();
//				btfss	temp4,2
//				bra		em4p_skip2
                                if(temp4 & 0x04)
                                {
//				btg		temp1,0
                                  if(temp1 & 0x01) temp1 &= 0xFE;
                                  else temp1 |= 0x01;
//				btg		temp3,2
                                  if(temp3 & 0x04) temp3 &= 0xFB;
                                  else temp3 |= 0x04;
//				movf	temp2,w
//				iorwf	INDF0,f
                                  INDF0 |= temp2; 
                                }
//em4p_skip2:
//				rcall	em_next_bit
                                em_next_bit();
//				btfss	temp4,1
//				bra		em4p_skip3
                                if(temp4 & 0x02)
                                {
//				btg		temp1,0
                                  if(temp1 & 0x01) temp1 &= 0xFE;
                                  else temp1 |= 0x01;
//				btg		temp3,1
                                  if(temp3 & 0x02) temp3 &= 0xFD;
                                  else temp3 |= 0x02;
//				movf	temp2,w
//				iorwf	INDF0,f
                                  INDF0 |= temp2; 
                                }
//em4p_skip3:
//				rcall	em_next_bit
                                em_next_bit();
//				btfss	temp4,0
//				bra		em4p_skip4
                                if(temp4 & 0x01)
                                {
//				btg		temp1,0
                                  if(temp1 & 0x01) temp1 &= 0xFE;
                                  else temp1 |= 0x01;
//				btg		temp3,0
                                  if(temp3 & 0x01) temp3 &= 0xFE;
                                  else temp3 |= 0x01;
//				movf	temp2,w
//				iorwf	INDF0,f
                                  INDF0 |= temp2; 
                                }
//em4p_skip4:
//				rcall	em_next_bit
                                em_next_bit();
//				btfss	temp1,0
//				bra		em4p_skip5
                                if(temp1 & 0x01)
                                {
//				movf	temp2,w
//				iorwf	INDF0,f
                                  INDF0 |= temp2; 
                                }
//em4p_skip5:
//				return
}
//em_next_bit:
void em_next_bit(void)
{
//				rrncf	temp2,f
                                temp2 >>= 1;
//??				btfsc	temp2,7
//??				movf	POSTINC0,w
//				return
}

//				END

