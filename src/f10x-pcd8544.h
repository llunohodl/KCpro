

void lcd8544_init(void);  // �������������� �������

void lcd8544_refresh(void); // ���������� ������ �� ������

void lcd8544_putpix(unsigned char x, unsigned char y, unsigned char mode); // ����� �������

void lcd8544_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode); // ����� �����

void lcd8544_rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode); // �������������

void lcd8544_putchar(unsigned char px, unsigned char py, unsigned char ch, unsigned char mode); //  ����� �������

void lcd8544_dec(unsigned int numb, unsigned char dcount, unsigned char x, unsigned char y, unsigned char mode); // ����� ����������� �����

void lcd8544_putstr(unsigned char x, unsigned char y, const unsigned char str[], unsigned char mode); // ����� ������


