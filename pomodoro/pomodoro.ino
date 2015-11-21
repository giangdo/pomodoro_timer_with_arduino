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
typedef enum e_button
{
	selectB = 0, // Select Button      <=> nút Select ở board lcd shield
	stopB   = 1, // Stop Button        <=> nút Left   ở board lcd shield
	workB   = 2, // Work Button        <=> nút Up     ở board lcd shield
	sBreakB = 3, // Short Break Button <=> nút Down   ở board lcd shield
	lBreakB = 4, // Long Break Button  <=> nút Right  ở board lcd shield
	noneB	  = 5  // Không có nút nào được nhấn
}E_Button; //Enum Button

#define adcPIN 0 //Chần GPIO 0 của MCU được nối với bộ nút nhấn của board LCD shield

typedef enum e_state
{
	workState   = 0,  //Working state
	sBreakState = 1,  //Short Break state
	lBreakState = 2,  //Long Break state
	stopState   = 3,  //Stop state
	notifyState = 4   // Notify state
}E_State; // Enum State

typedef enum e_bool
{
	False = 0,
	True  = 1
}E_Bool;

static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;
static E_Button lastButton = noneB;
// Hàm này dùng để xác định nút đã được nhấn trên board lcd shield
static E_Bool returned = False;
E_Button read_buttons_2()
{
	E_Button tmpButton = noneB;
	E_Button button;
	// MCU đọc giá trị của hiệu điện thế từ chân ADC (Analog Digital Converter)
	// hàm analogRead() được khai báo trong arduino-1.6.6/hardware/arduino/avr/cores/arduino/wiring_analog.c
	int adc = analogRead(adcPIN);

	// Thủ thuật hay: Bằng cách nối các nút nhấn vào một chân ADC của MCU như trong board
	// LCD shield này ta có thể xác định được trạng thái của nút nhấn thông qua giá trị
	// hiệu điện thế của một chân MCU. -> Tiết kiệm được chân MCU vì cách thông thường là
	// mỗi nút nhấn sẽ được kiểm soát bởi một chân MCU.

	// Nếu dùng board LCD shield version 1.0 thì dùng bảng sau xóa bảng còn lại
	if (adc < 50)
		tmpButton = lBreakB; //Khi nhấn nút lBreak  giá trị chính xác trả về là 0
	else if (adc < 195)
		tmpButton = workB;   //Khi nhấn nút workB   giá trị chính xác trả về là 144
	else if (adc < 380)
		tmpButton = sBreakB; //Khi nhấn nút sBreakB giá trị chính xác trả về là 329
	else if (adc < 555)
		tmpButton = stopB;   //Khi nhấn nút stopB   giá trị chính xác trả về là 504
	else if (adc < 790)
		tmpButton = selectB; //Khi nhấn nút selectB giá trị chính xác trả về là 741
	else 
		tmpButton = noneB; 

	if (tmpButton != lastButton)
	{
		lastDebounceTime = millis();
		returned = False;
	}

	if ((millis() - lastDebounceTime) > debounceDelay)
   {
	   if (returned == False)
		{
			button = tmpButton;
			returned = True;
		}
		else
		{
			button = noneB;
		}
	}

	lastButton = tmpButton;

	return button;
}

E_Button read_buttons_3()
{
	E_Button tmpButton = noneB;
	E_Button button;
	// MCU đọc giá trị của hiệu điện thế từ chân ADC (Analog Digital Converter)
	// hàm analogRead() được khai báo trong arduino-1.6.6/hardware/arduino/avr/cores/arduino/wiring_analog.c
	int adc = analogRead(adcPIN);

	// Thủ thuật hay: Bằng cách nối các nút nhấn vào một chân ADC của MCU như trong board
	// LCD shield này ta có thể xác định được trạng thái của nút nhấn thông qua giá trị
	// hiệu điện thế của một chân MCU. -> Tiết kiệm được chân MCU vì cách thông thường là
	// mỗi nút nhấn sẽ được kiểm soát bởi một chân MCU.

	// Nếu dùng board LCD shield version 1.0 thì dùng bảng sau xóa bảng còn lại
	if (adc < 50)
		tmpButton = lBreakB; //Khi nhấn nút lBreak  giá trị chính xác trả về là 0
	else if (adc < 195)
		tmpButton = workB;   //Khi nhấn nút workB   giá trị chính xác trả về là 144
	else if (adc < 380)
		tmpButton = sBreakB; //Khi nhấn nút sBreakB giá trị chính xác trả về là 329
	else if (adc < 555)
		tmpButton = stopB;   //Khi nhấn nút stopB   giá trị chính xác trả về là 504
	else if (adc < 790)
		tmpButton = selectB; //Khi nhấn nút selectB giá trị chính xác trả về là 741
	else 
		tmpButton = noneB; 

	if (tmpButton != lastButton)
	{
		lastDebounceTime = millis();
	}

   if ((millis() - lastDebounceTime) > debounceDelay)
   {
	   button = tmpButton;
	}

	lastButton = tmpButton;

	return button;
}

E_Button read_buttons()
{
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
	if (adc < 555) return stopB;   //Khi nhấn nút stopB   giá trị chính xác trả về là 504
	if (adc < 790) return selectB; //Khi nhấn nút selectB giá trị chính xác trả về là 741

	// Nếu dùng board LCD shield version 1.1 thì dùng bảng sau xóa bảng còn lại
	// if (adc_key_in < 50)   return lBreakB;
	// if (adc_key_in < 250)  return workB;
	// if (adc_key_in < 450)  return sBreakB;
	// if (adc_key_in < 650)  return failB;
	// if (adc_key_in < 850)  return selectB;

	return noneB;
}

typedef struct t_countDown
{
	E_State state;
	E_State prevSucState[17];
	unsigned long mileStone; // Thời điểm được lấy lám mốc để tính khoảng thời gian đã trôi qua
	unsigned long remain; // Thời gian còn lại tính theo đơn miligiây
}T_CountDown;

const unsigned long MINUTE_TO_SECONDS = 60;
const unsigned long SECOND_TO_MILISECONDS = 1000;
const unsigned long MINUTE_TO_MILISECOND = MINUTE_TO_SECONDS * SECOND_TO_MILISECONDS;
const unsigned long MAX_PERIOD_MINUTE = 60;

typedef struct t_count
{
	unsigned long period;  // Thời gian đếm lùi tính theo đơn vị miligiấy
	unsigned char success; // Tồng số lần thành công
	unsigned char fail;	  // Tổng số lần thất bại
	char const * progressStr; //Chuỗi sẽ in ra khi đang trong quá trình đếm
	char const * doneStr;     //Chuỗi sẽ in ra khi đếm xong
	char const * str;         //Tên của loại đếm xuống
}T_Count;

typedef enum e_mode
{
	countDownM = 0, // Countdown Mode
	summaryM   = 1, // Summary Mode
	modifyM    = 2, // Modify Mode
	noneM      = 3  // None Mode, this is use as polar
}E_Mode; // Enum mode

typedef void (*buttonHdl) (E_Button button);
typedef void (*modeHdl) ();

typedef struct t_mode
{
	E_Mode mode; //Mode
	buttonHdl buttonHandler; //function pointer that point to button handler function
	modeHdl taskHandler; //Function pointer that point to normal task handler of this mode
}T_Mode;

//Interface between mode and main thread
typedef struct t_lcdBuffer
{
	char origin[NUMBER_OF_LINE][NUMBER_OF_COLUM];
	E_Bool blink[NUMBER_OF_LINE][NUMBER_OF_COLUM];  //XXX should change to bit technique here
	char sendOut[NUMBER_OF_LINE][NUMBER_OF_COLUM];
}T_LcdBuffer;
#define LCD_CONTROL_FREQ 50 // 20 Milisecond
#define BLINK_FREQUENCY 500 // 500 Milisecond
#define BLINK_DUTY_CYCLE 250 // 250 milisecond

typedef struct t_dataBlock
{
	T_CountDown cntDown;
	T_Count     work;
	T_Count     sBreak;
	T_Count     lBreak;
	E_Mode      curMode;
	T_LcdBuffer lcdBuf;
}T_DataBlock;

T_DataBlock g_Data;

void lcd_buffer_clean();
void lcd_buffer_insert(int line, int colum, const char *str, E_Bool isBlink);
void lcd_flush_out();

void lcd_buffer_clean()
{
	memset(g_Data.lcdBuf.origin, 0, sizeof(g_Data.lcdBuf.origin));
	memset(g_Data.lcdBuf.blink, 0, sizeof(g_Data.lcdBuf.blink));
}

void lcd_buffer_insert(int line, int colum, const char *str, E_Bool isBlink)
{
	if ((line < NUMBER_OF_LINE) &&
		 (colum < NUMBER_OF_COLUM) &&
		 ((NUMBER_OF_COLUM - colum) >= strlen(str)))
	{
		memcpy(&(g_Data.lcdBuf.origin[line][colum]), str, strlen(str));
		E_Bool *pBlink = &g_Data.lcdBuf.blink[line][colum];
		for (unsigned char i = 0; i < strlen(str); i++)
		{
			*(pBlink + i) = isBlink;
		}
	}
}

void lcd_flush_out()
{
	char* p_origin = g_Data.lcdBuf.origin[0];
	E_Bool* p_blink = g_Data.lcdBuf.blink[0];
	char* p_sendOut = g_Data.lcdBuf.sendOut[0];

	if ((millis() % LCD_CONTROL_FREQ) == 0)
	{
		// Tạo hiệu ứng nhấp nháy
		if ((millis() % BLINK_FREQUENCY) < BLINK_DUTY_CYCLE)  
		{
			// Trong khoảng 250ms đầu tiên của 500ms thì giữ nguyên
			memcpy(p_sendOut, p_origin, sizeof(g_Data.lcdBuf.origin));
		}
		else
		{
			// Trong 250ms sau của 500ms thì hiển thị khoảng trắng
			// cho những vị trí được đánh dấu là blink
			for (unsigned char i = 0; i < sizeof(g_Data.lcdBuf.origin); i++)
			{
				if (*(p_blink + i) == True)
				{
					*(p_sendOut + i) = ' ';
				}
				else
				{
					*(p_sendOut + i) = *(p_origin + i);
				}
			}
		}

		// bo sung khoang trang de tao thanh chuoi lien tuc
		for (unsigned char j = 0; j < sizeof(g_Data.lcdBuf.sendOut); j++)
		{
			if (*(p_sendOut + j) == 0)
			{
				*(p_sendOut + j) = ' ';
			}
		}

		char sendOut_line0[NUMBER_OF_COLUM + 1];
		char sendOut_line1[NUMBER_OF_COLUM + 1];
		memcpy(sendOut_line0, g_Data.lcdBuf.sendOut[LINE_0],NUMBER_OF_COLUM);
		memcpy(sendOut_line1, g_Data.lcdBuf.sendOut[LINE_1],NUMBER_OF_COLUM);

		sendOut_line0[NUMBER_OF_COLUM] = 0; // add NULL
		sendOut_line1[NUMBER_OF_COLUM] = 0; // add NULL

		lcd.setCursor(COLUM_0,LINE_0);
		lcd.print(sendOut_line0);
		lcd.setCursor(COLUM_0,LINE_1);
		lcd.print(sendOut_line1);
	}
}

void countDownButtonHdl(E_Button button);
void countDownTaskHdl();
void summaryButtonHdl(E_Button button);
void summaryTaskHdl();
void modifyButtonHdl(E_Button button);
void modifyTaskHdl();

T_Mode g_Mode[] =
{
	{countDownM , countDownButtonHdl , countDownTaskHdl},
	{summaryM   , summaryButtonHdl   , summaryTaskHdl  },
	{modifyM    , modifyButtonHdl    , modifyTaskHdl   }
};

void insert_state(E_State state)
{
	//Thành viên mới nhất nằm ở đầu mảng, Thành viên cũ nhất nằm ở cuối mảng
	E_State prev = state;
	E_State cur;
	for (unsigned char i = 0; i < sizeof(g_Data.cntDown.prevSucState)/ sizeof(g_Data.cntDown.prevSucState[0]); i++)
	{
		// Dịch phải
		cur = g_Data.cntDown.prevSucState[i];  // Lưu giá trị hiện tại
		g_Data.cntDown.prevSucState[i] = prev; // Cập nhật giá trị mới
		prev = cur;                             // Chuẩn bị cho lần sau
	}
}

const char* recommended_next_state()
{
	// XXX nên lưu thêm thông số thời điểm mà chu kỳ thành công để dễ dàng hơn trong
	// việc đưa ra lởi khuyên
	const char *str;
	if ((g_Data.cntDown.prevSucState[0] == sBreakState) ||
		 (g_Data.cntDown.prevSucState[0] == lBreakState))
	{
		return g_Data.work.str; //Trước đó nghỉ thì bây giờ nên làm
	}
	else
	{
		if ((g_Data.cntDown.prevSucState[1] == sBreakState) &&
			 (g_Data.cntDown.prevSucState[2] == workState) &&
			 (g_Data.cntDown.prevSucState[3] == sBreakState) &&
			 (g_Data.cntDown.prevSucState[4] == workState) &&
			 (g_Data.cntDown.prevSucState[5] == sBreakState))
		{
			return g_Data.lBreak.str;
		}
		else
		{
			return g_Data.sBreak.str;
		}
	}
}

void countDownButtonHdl(E_Button button)
{
	if (button == noneB)
	{
		// ko có nút nhấn, ko làm gì cả
	}
	else
	{
		// Xác định chu kỳ hiện tại có thất bại hay ko?
		T_Count *pT_Count = NULL;
		if (g_Data.cntDown.state == workState)
		{
			pT_Count = &g_Data.work;
		}
		else if (g_Data.cntDown.state == sBreakState)
		{
			pT_Count = &g_Data.sBreak;
		}
		else if (g_Data.cntDown.state == lBreakState)
		{
			pT_Count = &g_Data.lBreak;
		}
		if (pT_Count != NULL)
		{
			if (g_Data.cntDown.remain != 0)
			{
				pT_Count->fail++;
			}
		}

		// Xử lý công việc theo chức năng của nút nhấn
		if (button == selectB)
		{
			g_Data.curMode = summaryM;
			g_Data.cntDown.state = stopState; // Xác định trạng thái khi quay lại mode countdown này
			g_Data.cntDown.remain = 0;
		}
		else
		{
			g_Data.cntDown.mileStone = millis();
			if (button == stopB)
			{
				g_Data.cntDown.state = stopState;
				g_Data.cntDown.remain = 0;
			}
			else if (button == workB)
			{
				g_Data.cntDown.state = workState;
				g_Data.cntDown.remain = g_Data.work.period;
			}
			else if (button == sBreakB)
			{
				g_Data.cntDown.state = sBreakState;
				g_Data.cntDown.remain = g_Data.sBreak.period;
			}
			else if (button == lBreakB)
			{
				g_Data.cntDown.state = lBreakState;
				g_Data.cntDown.remain = g_Data.lBreak.period;
			}
		}
	}
}

void countDownTaskHdl()
{
	// Tìm pointer trỏ đến state đếm hiện tại
	T_Count *pT_Count = NULL;
	if (g_Data.cntDown.state == workState)
	{
		pT_Count = &g_Data.work;
	}
	else if (g_Data.cntDown.state == sBreakState)
	{
		pT_Count = &g_Data.sBreak;
	}
	else if (g_Data.cntDown.state == lBreakState)
	{
		pT_Count = &g_Data.lBreak;
	}

	if (pT_Count != NULL) // Kiểm tra xem có đang ở state đếm nào ko ?
	{
		// Hàm millis() trả về giá trị tăng dần của timer, vì kiểu trả về là unsigned long -> giá trị
		// trả về lớn nhất của hàm là 2^64 micro giấy -> dùng hàm này ta có thể đếm từ bây giờ đến khoảng năm
		// trăm ngàn thiên niên kỉ nữa!
		// Hàm millis() sẽ trả về cùng giá trị nếu ta gọi liên tục hàm này trong khoảng thời gian nhỏ hơn 1 mili giây

		if (g_Data.cntDown.remain != 0)
		{
			// Số miligiây đã trải qua từ lúc bấm nút đếm
			unsigned long msPassed = millis() - g_Data.cntDown.mileStone;
			if (msPassed <= pT_Count->period)
			{
				g_Data.cntDown.remain = pT_Count->period - msPassed;
				unsigned int remainMin =
					(g_Data.cntDown.remain / SECOND_TO_MILISECONDS) / MINUTE_TO_SECONDS;
				unsigned int remainSec =
					(g_Data.cntDown.remain / SECOND_TO_MILISECONDS) % MINUTE_TO_SECONDS;

				lcd_buffer_clean();
				char string[17];
				snprintf(string, sizeof(string), "%s", pT_Count->progressStr);
				lcd_buffer_insert(LINE_0, 0, string, False);
				snprintf(string, sizeof(string), "%d:%d", remainMin, remainSec);
				lcd_buffer_insert(LINE_1, 0, string, False);
			}

			if (g_Data.cntDown.remain == 0)
			{
				// Ngay tại thời điểm thời gian về bằng 0, ta phải đếm lên số lần thành công ngay lập tức
				// và xử lý sao cho đoạn lệnh này chỉ được chạy một lần duy nhất trong suốt chu kỳ
				pT_Count->success++;

				// Cập nhật dữ liệu cho bảng chứa các trạng thái trước đó
				insert_state(g_Data.cntDown.state);
			}
		}
		else
		{
			char string[17];
			snprintf(string, sizeof(string), "%s, let %s!", pT_Count->doneStr, recommended_next_state());
			lcd_buffer_clean();
			lcd_buffer_insert(LINE_0, 0, string, True);
		}
	}
	else // Ko có pointer => đang ở Stop State
	{
		lcd_buffer_clean();
		lcd_buffer_insert(LINE_0, 0, "   Pomodoro", False);
		lcd_buffer_insert(LINE_1, 0, "Press Wo/Sb/Lb!" , False);
	}
}

void summaryButtonHdl(E_Button button)
{
	T_Count *pT_Count = NULL;
	switch (button)
	{
		case selectB:
			{
				g_Data.curMode = modifyM;
				break;
			}
		case workB:
			{
				pT_Count = &g_Data.work;
				break;
			}
		case sBreakB:
			{
				pT_Count = &g_Data.sBreak;
				break;
			}
		case lBreakB:
			{
				pT_Count = &g_Data.lBreak;
				break;
			}
		case stopB:
		default:
			{
				break;
			}
	}
	if (pT_Count != NULL)
	{
		pT_Count->success = 0;
		pT_Count->fail = 0;
	}
}

void summaryTaskHdl()
{
	char string[6];
	memset(string, 0, sizeof(string));

	lcd_buffer_clean(); //Xoá tất bộ đệm lcd

	lcd_buffer_insert(LINE_0, 1, "Wo", False);
	snprintf(string, sizeof(string), "%d/%d", g_Data.work.success,
				g_Data.work.fail + g_Data.work.success);
	lcd_buffer_insert(LINE_1, 0, string, False);

	lcd_buffer_insert(LINE_0, 7, "Sb", False);
	snprintf(string, sizeof(string), "%d/%d", g_Data.sBreak.success,
				g_Data.sBreak.fail + g_Data.sBreak.success);
	lcd_buffer_insert(LINE_1, 6, string, False);

	lcd_buffer_insert(LINE_0, 13, "Lb", False);
	snprintf(string, sizeof(string), "%d/%d", g_Data.lBreak.success,
				g_Data.lBreak.fail + g_Data.lBreak.success);
	lcd_buffer_insert(LINE_1, 12, string, False);
}

void modifyButtonHdl(E_Button button)
{
	T_Count *pT_Count = NULL;
	switch (button)
	{
		case selectB:
			{
				g_Data.curMode = countDownM;
				break;
			}
		case workB:
			{
				pT_Count = &g_Data.work;
				break;
			}
		case sBreakB:
			{
				pT_Count = &g_Data.sBreak;
				break;
			}
		case lBreakB:
			{
				pT_Count = &g_Data.lBreak;
				break;
			}
		case stopB:
		default:
			{
				break;
			}
	}
	if (pT_Count != NULL)
	{
		if (pT_Count->period > MINUTE_TO_MILISECOND)
		{
			pT_Count->period = pT_Count->period - MINUTE_TO_MILISECOND;
		}
		else
		{
			pT_Count->period = MAX_PERIOD_MINUTE * MINUTE_TO_MILISECOND;
		}
	}
}

void modifyTaskHdl()
{
	char string[3];
	memset(string, 0, sizeof(string));

	lcd_buffer_clean(); //Xoá tất bộ đệm lcd

	lcd_buffer_insert(LINE_0, 0, "Change|", False);
	lcd_buffer_insert(LINE_1, 0, "Period|", False);

	lcd_buffer_insert(LINE_0, 8, "Wo", False);
	snprintf(string, sizeof(string), "%lu", g_Data.work.period/MINUTE_TO_MILISECOND);
	lcd_buffer_insert(LINE_1, 8, string, False);

	lcd_buffer_insert(LINE_0, 11, "Sb", False);
	snprintf(string, sizeof(string), "%lu", g_Data.sBreak.period/MINUTE_TO_MILISECOND);
	lcd_buffer_insert(LINE_1, 11, string, False);

	lcd_buffer_insert(LINE_0, 14, "Lb", False);
	snprintf(string, sizeof(string), "%lu", g_Data.lBreak.period/MINUTE_TO_MILISECOND);
	lcd_buffer_insert(LINE_1, 14, string, False);
}

//----------------------------------------------------------------------------------------
// Đây là hàm khởi động của chương trình, sau hàm này chương trình sẽ nhảy vào thực thi
// hàm loop chính
//----------------------------------------------------------------------------------------
void setup()
{
	// xac dinh mode cua chan adc:
   pinMode(adcPIN, INPUT);

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
	for (int i = 0; i < 3; i++)
	{
		lcd.setCursor(COLUM_0,LINE_1);
		lcd.print(i); //Dùng hàm size_t Print::print(int n, int base)
		              //ở arduino-1.6.6/hardware/arduino/avr/cores/arduino/Print.cpp
		delay(100); //dừng 1000ms, hàm này ở arduino-1.6.6/hardware/arduino/avr/cores/arduino/wiring.c
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
		lcd.print(string[j]); // Dùng hàm size_t Print::print(char c)
	}
	delay(100);
	// Ở đây ta thấy cùng cách viết là lcd.print nhưng hàm thực sự được gọi trong runtime
	// là khác nhau. Kỹ thuật overside function được sử dụng để chọn hàm phù hợp nhất trong
	// đống hàm có trùng tên

	// Xác định Mode khởi động của chương trình
	g_Data.curMode = countDownM;
	g_Data.cntDown.state = stopState;
	g_Data.work.progressStr   = "Working";
	g_Data.work.doneStr       = "Worked";
	g_Data.work.str           = "Wo";
	g_Data.sBreak.progressStr = "SBreaking";
	g_Data.sBreak.doneStr     = "SBroken";
	g_Data.sBreak.str         = "Sb";
	g_Data.lBreak.progressStr = "LBreaking";
	g_Data.lBreak.doneStr     = "LBroken";
	g_Data.lBreak.str         = "Lb";

	T_Count *pCount = &g_Data.work;
	pCount->period = 25 * SECOND_TO_MILISECONDS;
	pCount->success = 0;
	pCount->fail = 0;

	pCount = &g_Data.sBreak;
	pCount->period = 5 * SECOND_TO_MILISECONDS;
	pCount->success = 0;
	pCount->fail = 0;

	pCount = &g_Data.lBreak;
	pCount->period = 15 * SECOND_TO_MILISECONDS;
	pCount->success = 0;
	pCount->fail = 0;
}

//----------------------------------------------------------------------------------------
// Đây là vòng lặp lớn nhất của chương trình, sau khi khởi động xong, MCU sẽ chỉ thực thi
// trong vòng lặp này mà thôi.
//----------------------------------------------------------------------------------------
void loop()
{
	// Đọc tín hiệu ADC để biết nút nào đã được nhấn
	//E_Button button = read_buttons();
	E_Button button = read_buttons_2();

	// Xác định mode hiện tại và thực hiện công việc trong mode đó
	unsigned int modeIndex;
	for (modeIndex = 0; modeIndex < (sizeof(g_Mode) / sizeof(T_Mode)); modeIndex++)
	{
		if (g_Mode[modeIndex].mode == g_Data.curMode)
		{
			g_Mode[modeIndex].buttonHandler(button); // xử lý công việc dựa theo nút đã nhấn
			g_Mode[modeIndex].taskHandler();         // xử lý công việc bình thường trong mode
			break;
		}
	}
	if (modeIndex == (sizeof(g_Mode) / sizeof(T_Mode)))
	{
		// Nếu không tìm thấy mode hiện tại trong bảng -> lỗi của chương trình
		// Chương trình chạy tốt sẽ ko bao giờ nhảy vào đoạn này
		lcd.setCursor(0,LINE_1);
		lcd.print("I'm broken!!!");
	}

	//In ký tự trong buffer ra màn hình (gửi lcd buffer từ MCU đến LCD driver chip
	lcd_flush_out();
}
