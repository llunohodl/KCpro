

void Delay1KTCYx(unsigned int delay)
{
unsigned int i;
  for(; delay; delay --)
    for(i = 0; i < 8000; i ++);

}

void Delay10KTCYx(unsigned int delay)
{
unsigned long i;
  for(; delay; delay --)
    for(i = 0; i < 80000; i ++);

}
/*
void Delay1TCYx(unsigned int delay)
{
unsigned int i;
  for(; delay; delay --)
    for(i = 0; i < 32; i ++);

}
*/
void Delay10TCYx(unsigned int delay)
{
unsigned int i;
  for(; delay; delay --)
    for(i = 0; i < 80; i ++);

}

void Delay100TCYx(unsigned int delay)
{
unsigned int i;
  for(; delay; delay --)
    for(i = 0; i < 800; i ++);

}
