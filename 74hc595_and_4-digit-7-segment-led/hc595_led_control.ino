unsigned char LED_0F[] =
{ // 0    1     2    3     4       5    6     7     8     9     A    b     C      d     E     F    -
  0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0x8C, 0xBF, 0xC6, 0xA1, 0xc0, 0x86, 0xff, 0xbf
};
unsigned char LED[4]; //用于LED的4位显示缓存
int SCLK = 5;
int RCLK = 6;
int DIO = 7; //这里定义了那三个脚
void setup ()
{
  pinMode(SCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(DIO, OUTPUT); //让三个脚都是输出状态
}
void loop()
{

  LED[0] = 2;
  LED[1] = 2;
  LED[2] = 1;
  LED[3] = 0;


  for (int i = 0; i < 24 * 4; i++)
  {
    LED4_Display ();
  }


  LED[3] = 16;
    LED4_Display ();
  delay(1000);


  LED[0] = 9;
  LED[1] = 2;
  LED[2] = 1;
  LED[3] = 3;
  for (int i = 0; i < 24 * 4; i++)
  {
    LED4_Display ();
  }
  LED[3] = 16;
    LED4_Display ();
  delay(1000);
}
void delay_5(unsigned char us)
{
  delayMicroseconds(us * 50);
}
void LED4_Display (void)
{
  unsigned char *led_table;          // 查表指针
  unsigned char i;
  //显示第1位 - Digit 1
  led_table = LED_0F + LED[0];
  i = *led_table;
  LED_OUT(i);
  LED_OUT(0x01);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
  delay_5(5);
  //显示第2位 - Digit 2
  led_table = LED_0F + LED[1];
  i = *led_table;
  LED_OUT(i);
  LED_OUT(0x02);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
  delay_5(5);

  //显示第3位 - Digit 3
  led_table = LED_0F + LED[2];
  i = *led_table;
  LED_OUT(i);
  LED_OUT(0x04);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
  delay_5(5);
  //显示第4位 - Digit 4
  led_table = LED_0F + LED[3];
  i = *led_table;
  LED_OUT(i);
  LED_OUT(0x08);
  digitalWrite(RCLK, LOW);
  digitalWrite(RCLK, HIGH);
  delay_5(5);
}

void LED_OUT(unsigned char X)
{
  unsigned char i;
  for (i = 8; i >= 1; i--)
  {
    if (X & 0x80)
    {
      digitalWrite(DIO, HIGH);
    }
    else
    {
      digitalWrite(DIO, LOW);
    }
    X <<= 1;
    digitalWrite(SCLK, LOW);
    digitalWrite(SCLK, HIGH);
    delay_5(5);
  }
}



