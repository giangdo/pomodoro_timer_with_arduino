/*************************************************************************************
 
  Mark Bramwell, July 2010
 
  This program will test the LCD panel and the buttons.When you push the button on the shield，
  the screen will show the corresponding one.
  
  Connection: Plug the LCD Keypad to the UNO(or other controllers)
 
**************************************************************************************/
 
#include <LiquidCrystal.h>
 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel
 
// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
 
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
 
int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 
 
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result
 
    if (adc_key_in > 1000) return btnNONE; 
 
    // For V1.1 us this threshold
    /*
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;  
    */
   // For V1.0 comment the other threshold and use the one below:
   
     if (adc_key_in < 50)   return btnRIGHT;  
     if (adc_key_in < 195)  return btnUP; 
     if (adc_key_in < 380)  return btnDOWN; 
     if (adc_key_in < 555)  return btnLEFT; 
     if (adc_key_in < 790)  return btnSELECT;   
   
 
    return btnNONE;                // when all others fail, return this.
}

unsigned long prevTime = 0;
typedef enum status
{
  Wo = 0, //Working
  Sb = 1, //Short Break
  Lb = 2, //Long Break
  St = 3  //Stop
}eStatus;
eStatus status = Wo;
unsigned long curSecond = 0;
unsigned long curMin = 0;
unsigned long resMin = 0;
void setup(){
   lcd.begin(16, 2);               // start the library
   lcd.setCursor(0,0);             // set the LCD cursor   position 
   lcd.print("Wo<-  Sb^  Lb->");  // print a simple message on the LCD
}
  
typedef enum mode
{
	countDownM = 0,
	summaryM = 1,
   modifyM = 2
}eMode;
const char * const countDownStr = "Count Down";
const char * const summaryStr = "Summary";
const char * const modifyStr = "Modify";
typedef enum button
{
	selectB = 0,
	breakB  = 1,
	workB   = 2,
	break1  = 3,
	break2  = 4
}eButton;

typedef void (*buttonHdl) (eButton button,void *data);
typedef void (*modeHdl) (void* data);
typedef struct mode
{
	eMode cur; //this mode
	const char *name; //this mode's string name
	buttonHdl butHandler; //function pointer that point to button handler function   	
	modeHdl modeHdl; //Function pointer that point to normal task handler of this mode
	void *data;  //Internal data of this mode
}tMode;

//Interface between mode and main thread
eMode curMode;
char line1Buf[17]; //buffer that contain characters for first line in lcd
char line1Blink[17]; //buffer that contain blink feature of each character for first line in lcd
char line2Buf[17]; //buffer that contain character for second line in lcd
char line2Blink[17];  //buffer that contain blink feature of each character of second line

typedef enum counterType;
{
	workCounter = 0,
	sBreakCounter = 1,
	lBreakCounter = 2
}eCounterType

typedef struct countDown
{
	eMode nxt; //next mode
	unsigned long prevTime;
	eCounterType counterType;
	char prevCounterType[17];
}countDownT countDownData;

typedef struct summary
{
	eMode nxt; //next mode
}summaryT summaryData;

typedef struct modify
{
	eMode nxt; //next mode
}modifyT modifyData;

void countDownBHdl(eButton button,void *data);
void countDownHdl(void *data);
void summaryBHdl(eButton button,void *data);
void summaryHdl(void* data);
void modifyBHdl(eButton button,void *data);
// What is the mechanism to transfer data between mode
tMode mode[3] =
{
	{countDownM, summaryM, countDownStr, countDownBHdl, countDownHdl, &countDownData},
	{summaryM, modifyM, summaryStr, summaryBHdl, summaryHdl, &summaryData}, 
	{modifyM, countDownM,modifyStr, modififyBHdl, modifyHdl, &modifyData}
}
void loop(){  
	
	// read button
	// handle button function if there are button
	// handle for mode (generate data buffer
	// print character to lcd
   curSecond = (millis() - prevTime) / 1000;
   curMin = curSecond / 60;
   switch (status)
   {
    case Wo:
    {
      lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("Work ");
      if (curMin < 25)
        resMin = 25 - curMin;
      else
        status = St;
        
      break;
    }
    case Sb:
    {
      lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("SBreak ");
      if (curMin < 5)
        resMin = 5 - curMin;
      else
        status = St;
      break;
    }    
    case Lb:
    {
      lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("LBreak ");
      if (curMin < 15)
        resMin = 15 - curMin;
      else
        status = St;
      break;
    }    
    case St:
    {
      if ((millis() % 500) > 250)
      {
      lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("Please Stop Now");
      }
      else
      {
        lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("               ");
      }
      break;
    }    
   }

   if (status != St)
   {
      lcd.setCursor(7,1);             // move cursor to second line "1" and 9 spaces over             
      lcd.print(resMin);      
      lcd.print("  ");
   }
   
   lcd_key = read_LCD_buttons();   // read the buttons
 
   switch (lcd_key){               // depending on which button was pushed, we perform an action
 
       case btnRIGHT:{             //  push button "RIGHT" and show the word on the screen
          lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("               ");
            prevTime = millis();
            status = Lb; 
            break;
       }
       case btnLEFT:{             
          lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("               ");
             prevTime = millis(); 
             status = Wo;
             break;
       }    
       case btnUP:{             
          lcd.setCursor(0,1);             // move to the begining of the second line
      lcd.print("               ");
             prevTime = millis();
             status = Sb; 
             break;
       }
       case btnDOWN:{            
             break;
       }
       case btnSELECT:{             
             break;
       }
       case btnNONE:{             
             break;
       }
   }
}
