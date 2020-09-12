#include "nrf_gpio.h"
#include "f10x-pcd8544.h"
#include "font6x8.h"	  // шрифт



// Управление линией LCD_CS
#define LCD_CS1   nrf_gpio_pin_set(20);
#define LCD_CS0   nrf_gpio_pin_clear(20);
// Управление линией LCD_RST
#define LCD_RST1  nrf_gpio_pin_set(10);
#define LCD_RST0  nrf_gpio_pin_clear(10);
// Управление линией LCD_DC
#define LCD_DC1   nrf_gpio_pin_set(19);
#define LCD_DC0   nrf_gpio_pin_clear(19);
// Управление линией LCD_SCK
#define LCD_SCK1   nrf_gpio_pin_set(13);
#define LCD_SCK0   nrf_gpio_pin_clear(13);
// Управление линией LCD_MOSI
#define LCD_MOSI1   nrf_gpio_pin_set(15);
#define LCD_MOSI0   nrf_gpio_pin_clear(15);

unsigned char lcd8544_buff[85*6]; // буфер дисплея
unsigned char lcd_buff[85*6];

// отправка данных\команд на дисплей
void lcd8544_senddata(unsigned char data) {
unsigned char i;
	for(i=0;i<8;i++) {
		if (data & 0x80) LCD_MOSI1 
                else LCD_MOSI0;
		data = data<<1;
          //      CPUdelay(100);
		LCD_SCK0;
           //     CPUdelay(100);
		LCD_SCK1;
	}
}

// обновление дисплея содержимым буфера
void lcd8544_refresh(void) {
	LCD_CS0;            // СS=0 - начали сеанс работы с дисплеем
       // CPUdelay(100);
	LCD_DC0;            // передача комманд
      //  CPUdelay(100);
	lcd8544_senddata(0x40); // установка курсора в позицию Y=0; X=0
	lcd8544_senddata(0x80);

	LCD_DC1;            // передача данных

	unsigned char y, x;
        unsigned int i;
        for(i=0;i<504;i++)lcd_buff[i] = lcd8544_buff[503 - i];
        for(i=0;i<504;i++)
          {
            x = 1;
            if( lcd_buff[i] & 0x01 ) x |= 0x80;
            if( lcd_buff[i] & 0x02 ) x |= 0x40;
            if( lcd_buff[i] & 0x04 ) x |= 0x20;
            if( lcd_buff[i] & 0x08 ) x |= 0x10;
            if( lcd_buff[i] & 0x10 ) x |= 0x08;
            if( lcd_buff[i] & 0x20 ) x |= 0x04;
            if( lcd_buff[i] & 0x40 ) x |= 0x02;
            lcd_buff[i] = x;
          }

        lcd8544_buff[504] = 0xff;
	for (y=0;y<6;y++) 
        for (x=0;x<84;x++) lcd8544_senddata(lcd_buff[y*84+x]);
        LCD_CS1;
}


// Инициализация
void lcd8544_init(void) {
unsigned int i;
          for( i = 0; i < 504; i ++ ) lcd8544_buff[i] = 0;
	  LCD_SCK0;
	  // сброс дисплея
	  LCD_CS0;            // CS=0  - начали сеанс работы с дисплеем
	  LCD_RST0;           // RST=0 - сброс дисплея
	  LCD_RST1;           // RST=1

	  // последовательность инициализации дисплея
	  LCD_DC0;            // передача команды

	  lcd8544_senddata(0x21);      // переход в расширенный режим
	  lcd8544_senddata(0xC1);

	  lcd8544_senddata(0x06);		// температурный коэффициент, от 4 до 7

	  lcd8544_senddata(0x13);		// Bias 0b0001 0xxx - работает как контрастность

	  lcd8544_senddata(0x20); 	// переход в обычный режим
	  lcd8544_senddata(0x0D);	    // 0b1100 - normal mode
			                        // 0b1101 - invert mode
			                        // 0b1001 - полностью засвеченный экран
			                        // 0b1000 - чистый экран

	  LCD_DC1;            // передача команды

}

// вывод пиксела
void lcd8544_putpix(unsigned char x, unsigned char y, unsigned char mode) {
	if ((x>84) || (y>47)) return;

	unsigned int adr=(y>>3)*84+x;
	unsigned char data=(1<<(y&0x07));

	if (mode) lcd8544_buff[adr]|=data;
   else lcd8544_buff[adr]&=~data;
}



// процедура рисования линии
void lcd8544_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode) {
signed char   dx, dy, sx, sy;
unsigned char  x,  y, mdx, mdy, l;


  dx=x2-x1; dy=y2-y1;

  if (dx>=0) { mdx=dx; sx=1; } else { mdx=x1-x2; sx=-1; }
  if (dy>=0) { mdy=dy; sy=1; } else { mdy=y1-y2; sy=-1; }

  x=x1; y=y1;

  if (mdx>=mdy) {
     l=mdx;
     while (l>0) {
         if (dy>0) { y=y1+mdy*(x-x1)/mdx; }
            else { y=y1-mdy*(x-x1)/mdx; }
         lcd8544_putpix(x,y,mode);
         x=x+sx;
         l--;
     }
  } else {
     l=mdy;
     while (l>0) {
        if (dy>0) { x=x1+((mdx*(y-y1))/mdy); }
          else { x=x1+((mdx*(y1-y))/mdy); }
        lcd8544_putpix(x,y,mode);
        y=y+sy;
        l--;
     }
  }
  lcd8544_putpix(x2, y2, mode);

}



// рисование прямоугольника (не заполненного)
void lcd8544_rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode) {
	lcd8544_line(x1,y1, x2,y1, mode);
	lcd8544_line(x1,y2, x2,y2, mode);
	lcd8544_line(x1,y1, x1,y2, mode);
	lcd8544_line(x2,y1, x2,y2, mode);
}

// вывод символа на экран по координатам
void lcd8544_putchar(unsigned char px, unsigned char py, unsigned char ch, unsigned char mode) {
	const unsigned char *fontpointer;


	if (ch<127) {	// знакогенератор латиницы
		fontpointer=NewFontLAT; ch=ch-32; }
		else	{	// знакогенератор русских букв
		fontpointer=NewFontRUS; ch=ch-192;}

    unsigned char lcd_YP=7- (py & 0x07);    // битовая позиция символа в байте
    unsigned char lcd_YC=(py & 0xF8)>>3; 	// байтовая позиция символа на экране
    unsigned char x;
	for (x=0; x<6; x++) {

		unsigned char temp=*(fontpointer+ch*6+x);

		if (mode!=0) {
			temp=~temp;
			if (py>0) lcd8544_putpix(px, py-1, 1);	// если печать в режиме инверсии - сверху отчертим линию
		}

		temp&=0x7F;

		lcd8544_buff[lcd_YC*84+px]=lcd8544_buff[lcd_YC*84+px] | (temp<<(7-lcd_YP)); 	// печать верхней части символа

	    if (lcd_YP<7) lcd8544_buff[(lcd_YC+1)*84+px]=lcd8544_buff[(lcd_YC+1)*84+px] | (temp>>(lcd_YP+1)); 	// печать нижней части символа

		px++;
		if (px>83) return;
	}
}


// печать десятичного числа
void lcd8544_dec(unsigned int numb, unsigned char dcount, unsigned char x, unsigned char y, unsigned char mode) {
	unsigned int divid=10000;
	unsigned char i;

	for (i=5; i!=0; i--) {

		unsigned char res=numb/divid;

		if (i<=dcount) {
			lcd8544_putchar(x, y, res+'0', mode);
			x=x+6;
		}

		numb%=divid;
		divid/=10;
	}
}


// линия левее символа для печати в инверсе
void lcd8544_leftchline(unsigned char x, unsigned char y) {
	if (x>0) lcd8544_line(x-1, y-1, x-1, y+6, 1);
}


// вывод строки
void lcd8544_putstr(unsigned char x, unsigned char y, const unsigned char str[], unsigned char mode) {
	if (mode) lcd8544_leftchline(x, y);
	while (*str!=0) {
		lcd8544_putchar(x, y, *str, mode);
		x=x+6;
		str++;
	}
}
