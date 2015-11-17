/*************************************************************************************
Giang Do (Đỗ Trường Giang), November 2015
 
Đây là chương trình Pomodoro Timer dùng hai bo mạch lcd-button shield và Ardunio U3

Chương trình gồm có 3 mode chính:
	+ Countdown(đếm lùi): đây là mode chính của chương trình, cho phép thiết lập thời
		gian đếm lùi theo 3 kiểu và phát tín hiệu cảnh báo khi thời gian lùi về 0.
			- Working : Làm việc
			- Long Break : Nghỉ dài
			- Short Break : Nghỉ ngắn
	+ Summary(Tổng hợp): đây là mode dùng để hiển thị kết quả số lẩn thực hiện thành công của
		từng kiểu đếm lùi. từ khi đồng hồ pomodoro này được khởi động chạy.

	+ Modify(Điều chỉnh): đây là mode dùng để điểu chỉnh khoảng thời gian cho mỗi kiểu đếm lùi.
		Giá trị mặc định là:
		- Working: 25 phút
		- Long Break: 15 phút
		- Short Break: 5 phút 

Cách sử dụng:
	+ Nhấn Select để chuyển mode: Countdown -> Summary -> Modify -> Countdown
	+ Trong mode Countdown:
		- Để bắt đầu đếm lùi Nhấn nút Work, hoặc LBreak, hoặc SBreak để bắt đầu đếm lùi
		   theo kiểu Làm Việc, Nghỉ dài, nghỉ ngắn.
		- Nhấn nút Fail để xác nhận chu kỳ đang thực hiện cũng như vừa thực hiện xong ko thành công.

	+ Trong mode Summary: xem số lần làm việc/nghỉ ngắn/nghỉ dài thành công
		- Nhấn nút Fail để thiết lập lại tất cả giá trị các bộ đếm về 0

	+ Trong mode Modify: 
		- Nhấn nút Work, hoặc LBreak, hoặc SBreakWork chỉnh giá trị cho thời gian Làm việc,
		  Nghỉ dài, Nghỉ ngắn

Thế nào là một chu kỳ thành công ?
	+ Một chu kì chỉ được xác định là thành công chỉ khi đồng hồ đếm lùi đã về 0
	  và người dùng ko nhấn nút Fail sau đó.

Thế nào là một chu kỳ ko thành công ?
	+ Trong lúc đang đếm lùi: Nhấn bất kỳ một nút nào đó
	+ Khi đã kết thúc: Nhấn nút Fail.
			
* Định hướng lập trình (dành cho các bạn mới làm quen với lập trình nhúng)
	+ Cần phải giải quyết bài toán MCU làm nhiều công việc gần như cùng lúc (lập trình song song):
		- MCU phải điều khiển chip LCD driver để hiển thị ký tự trên LCD
		- MCU phải đọc giá trị ADC để biết nút nhấn nào được nhấn
		- MCU phải tăng giá trị timer để làm bộ đếm lùi
		- MCU phải tính toán để quyết định hiển thị cái gì ra màn hình.

		-> Hướng giải quyết:
		 	. Các công việc tất nhiên sẽ được thực hiện 1 cách tuần tự. (vì ko có nhiều core trong MCU)
			. Các công việc phải được chia rất nhỏ (thời gian thực hiện rất ngắn, tính bằng micro giấy)
		 	. Ko thể để một công việc chiếm quá nhiều thời gian xử lý khiến cho tất cả các chức năng
			  khác ko thể thực thi.
			  Ví dụ: Chắc chắn là khi MCU điều khiển LCD, MCU sẽ ko thể nhận biết được nút nhấn.
			  			Vậy nếu ta cách ta lập trình khiến cho MCU tốn nhiều thời gian cho điều khiển LCD,
						MCU sẽ hầu như ko nhận được tín hiệu nhấn nút được do lúc ta nhấn nút
						thì MCU đang bận điều khiển LCD rồi.

	+ Cần phải giải quyết bài toán máy trạng thái:
		- Chỉ cần nhấn một nút Select thì có thể chuyển qua lần lượt 3 mode, tức là chuyển trạng
		  thái chỉ thông qua một nút nhấn chứ ko phải là mỗi nút nhấn một trạng thái.

		- Ngoài ra ở mỗi mode, nút nhấn cũng có chức năng khác nhau.

		-> Hướng giải quyết:
			. Dùng bảng để thể hiện máy trạng thái.
			. Dùng con trỏ hàm để thể hiện chức năng của mỗi nút nhấn.

**************************************************************************************/
 
/* Khai báo sử dụng thư viện điều khiển LCD */
#include <LiquidCrystal.h>
 
// Kỹ thuật sử dụng macro để tránh sử dụng những con số vô nghĩa trong lập trình
// XXX please check this again
#define LCD_D0 8  //GPIO8 của MCU nối với chân LCD D0
#define LCD_D1 9  //GPIO9 của MCU nối với chân LCD D1
#define LCD_D2 4  //GPIO4 của MCU nối với chân LCD D2
#define LCD_D3 5  //GPIO5 của MCU nối với chân LCD D3
#define LCD_D4 6  //GPIO6 của MCU nối với chân LCD D4
#define LCD_D5 7  //GPIO7 của MCU nối với chân LCD D5
#define COLUM_0 0
#define COLUM_1 1
#define NUMBER_OF_COLUM 16
#define LINE_0 0
#define LINE_1 1
#define NUMBER_OF_LINE 2
// Khai báo một object của lớp LiquidCrystal, đồng thời thông qua constructor của lớp
// này xác định các chân GPIO trên MCU dùng để điều khiển LCD 
// Thư viện LiquidCrystal viết bằng C++ ở arduino-1.6.6/libraries/LiquidCrystal/src/LiquidCrystal.h 
LiquidCrystal lcd(LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5);

// Kỹ thuật sử dụng enum để tránh sử dụng những con số vô nghĩa trong lập trình 
typedef enum button
{
	selectB = 0, // Select Button      <=> nút Select ở board lcd shield
	failB   = 1, // Fail Button        <=> nút Left   ở board lcd shield
	workB   = 2, // Work Button        <=> nút Up     ở board lcd shield
	sBreakB = 3, // Short Break Button <=> nút Down   ở board lcd shield
	lBreakB = 4, // Long Break Button  <=> nút Right  ở board lcd shield
   noneB	  = 5  // Không có nút nào được nhấn
}E_Button; //Enum Button

#define adcPIN 0 //Chần GPIO 0 của MCU được nối với bộ nút nhấn của board LCD shield

// Hàm này dùng để xác định nút đã được nhấn trên board lcd shield
E_Button read_buttons(){
	// MCU đọc giá trị của hiệu điện thế từ chân ADC (Analog Digital Converter)
	// hàm analogRead() được khai báo trong arduino-1.6.6/hardware/arduino/avr/cores/arduino/wiring_analog.c
    int adc = analogRead(adcPIN); 

	// Thủ thuật hay: Bằng cách nối các nút nhấn vào một chân ADC của MCU như trong board
	// LCD shield này ta có thể xác định được trạng thái của nút nhấn thông qua giá trị
	// hiệu điện thế của một chân MCU. -> Tiết kiệm được chân MCU vì cách thông thường là
	// mỗi nút nhấn sẽ được kiểm soát bởi một chân MCU.
		 
 	 // Khi adc là rất lớn ,gần như vô cùng -> ko có nút nào được nhấn
    if (adc > 1000) return noneB; 
 
	 // Nếu dùng board LCD shield version 1.0 thì dùng bảng sau xóa bảng còn lại
    if (adc < 50)  return lBreakB; //Khi nhấn nút lBreak  giá trị chính xác trả về là 0
    if (adc < 195) return workB;   //Khi nhấn nút workB   giá trị chính xác trả về là 144 
    if (adc < 380) return sBreakB; //Khi nhấn nút sBreakB giá trị chính xác trả về là 329 
    if (adc < 555) return failB;   //Khi nhấn nút failB   giá trị chính xác trả về là 504 
    if (adc < 790) return selectB; //Khi nhấn nút selectB giá trị chính xác trả về là 741 
 
	 // Nếu dùng board LCD shield version 1.1 thì dùng bảng sau xóa bảng còn lại
    // if (adc_key_in < 50)   return lBreakB;  
    // if (adc_key_in < 250)  return workB; 
    // if (adc_key_in < 450)  return sBreakB; 
    // if (adc_key_in < 650)  return failB; 
    // if (adc_key_in < 850)  return selectB;

    return noneB;
}

unsigned long prevTime = 0;
typedef enum status
{
  Wo = 0,  //Working
  Sb = 1,  //Short Break
  Lb = 2,  //Long Break
  St = 3   //Stop
}E_Status; // Enum Status

unsigned long curSecond = 0;
unsigned long curMin = 0;
unsigned long resMin = 0;
  
typedef enum mode
{
	countDownM = 0, // Countdown Mode
	summaryM   = 1, // Summary Mode
   modifyM    = 2, // Modify Mode
   noneM      = 3  // None Mode, this is use as polar
}E_Mode; // Enum mode

typedef void (*buttonHdl) (eButton button,void *data);
typedef void (*modeHdl) (void* data);
typedef struct mode
{
	E_Mode mode; //Mode
	buttonHdl buttonHandler; //function pointer that point to button handler function   	
	modeHdl taskHandler; //Function pointer that point to normal task handler of this mode
}T_Mode;

//Interface between mode and main thread
typedef struct lcdBuffer
{
	char line1Buf[17]; //buffer that contain characters for first line in lcd
	char line1Blink[17]; //buffer that contain blink feature of each character for first line in lcd
	char line1PreState[17]; //buffer that contain blink feature of each character for first line in lcd
	char line2Buf[17]; //buffer that contain character for second line in lcd
	char line2Blink[17];  //buffer that contain blink feature of each character of second line
	char line2PreState[17]; //buffer that contain blink feature of each character for first line in lcd
}T_LcdBuffer g_LcdBuffer;

typedef enum counterType;
{
	workCounter = 0,
	sBreakCounter = 1,
	lBreakCounter = 2
}E_CounterType;

typedef struct countDown 
{
	unsigned long prevTime;
	eCounterType counterType;
	char prevCounterType[17];
}T_CountDown;

typedef struct summary
{
	unsigned char workSuccess;   // Number of time that we finish work period
	unsigned char sBreakSuccess; // Number of time that we finish short break period
	unsigned char lBreakSuccess; // Number of time that we finish long break period
}T_Summary;

typedef struct modify
{
	eMode nxt; //next mode
	unsigned char workPeriod;
	unsigned char sBreakPeriod;
	unsigned char lBreakPeriod; 
}T_Modify;

typedef struct dataBlock
{
	T_CountDown countDownData;
	T_Summary   summaryData;
	T_Modify    modifyData;
   E_Mode      curMode;
}T_DataBlock g_DataBlock;

void countDownButtonHdl(E_Button button)
void countDownTaskHdl();
void summaryButtonHdl(E_Button button)
void summaryTaskHdl();
void modifyButtonHdl(E_Button button)
void modifyTaskHdl();

// What is the mechanism to transfer data between mode
T_Mode g_Mode[] = // global variable mode
{
	{countDownM , countDownButtonHdl , countDownTaskHdl},
	{summaryM   , summaryButtonHdl   , summaryTaskHdl  },
	{modifyM    , modififyButtonHdl  , modifyTaskHdl   }
}

void setup()
{
	// Khởi động LCD dùng hàm void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
	// và dùng kỹ thuật default argument trong C++ để không cần cung cấp thông tin về dotsize
	// mà lấy giá trị mặc định LCD_5x8DOTS.
	// LCD_5x8DOTS5x8 có ý nghĩa là lcd dùng ô chữ chật 5x8 = 40 điểm để hiển thị 1 ký tự
	lcd.begin(NUMBER_OF_COLUM, NUMBER_OF_LINE);


	lcd.setCursor(COLUM_0,LINE_0);  // di chuyển con trỏ của LCD đến dòng đầu tiên, cột đầu tiên

	// Dùng hàm size_t Print::print(const char str[]) ở arduino-1.6.6/hardware/arduino/avr/cores/arduino/Print.cpp
	// để in dòng chữ "Wellcome"
	// Kỹ thuật inheritance trong C++ được sử dụng, đối tượng của lớp LiquidCrystal có thể gọi hàm của lớp Print
	// vì lớp LiquidCrystal thừa kế theo kiểu public lớp Print
	lcd.print("Welcome");    

	// In số 0 rồi 1 rồi 2 ... 
	for (int i = 0; i < 3, i++)
	{
		lcd.setCursor(COLUM_0,LINE_1);
		lcd.print(i);  //XXX need to comment here
		delay(1000); 	//dừng 1000ms, hàm này ở arduino-1.6.6/hardware/arduino/avr/cores/arduino/wiring.c
	}

	// In "Pomodoro Now!"
	char string[12];
	string[0]  = 'P';
	string[1]  = 'o';
	string[2]  = 'm';
	string[3]  = 'o';
	string[4]  = 'd';
	string[5]  = 'o';
	string[6]  = 'r';
	string[7]  = 'o';
	string[8]  = ' ';
	string[9]  = 'N';
	string[10] = 'o';
	string[11] = 'w';
	string[12] = '!';
	for (int j = 0; j < sizeof(string); j++)
	{
		lcd.setCursor(j,LINE_1); // XXX need to comment here
		lcd.print(string[j]);  
	}
	// XXX need to comment about polimophism technique
	// XXX need to comment about overwrite function technique in print function
}

void loop(){  
	// Đây là vòng lặp lớn nhất của chương trình, sau khi khởi động xong, MCU sẽ chỉ thực thi
	// trong vòng lặp này mà thôi.

	while (1)
	{
		// Đọc tín hiệu ADC để biết nút nào đã được nhấn
		eButton button = read_buttons(); 

		// XXX
		unsigned int modeIndex;
		for (modeIndex = 0; modeIndex < (sizeof(g_Mode) / sizeof(T_Mode)); i++)
		{
			if (g_Mode[modeIndex].mode == g_DataBlock.curMode)
			{
				g_Mode[modeIndex].buttonHandler(button);
				g_Mode[modeIndex].taskHandler();
				break;
			}
		}
		if (modeIndex == (sizeof(g_Mode) / sizeof(T_Mode)))
		{
			// Should never come into this case!
		}

		// XXX

	}

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
