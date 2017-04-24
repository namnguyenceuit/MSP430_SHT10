#include "library/sht.h"

void SHT_Init()
{
	P1SEL &= ~(SCK+SDA); 		//I/O function is selected
	P1DIR |= SCK;				//Output selected
	P1DIR &= ~SDA;				//Data is input
}
/*(Data Sheet)
To initiate a transmission, a Transmission Start sequence has to be issued.
It consists of a lowering of the DATA line while SCK is high, followed by a low pulse on SCK
and raising DATA again while SCK is still high
*/

/*
Truyen xung Start:
- Chan DATA = 1
- SCK co xung tu thap len cao
- Sau do MCU keo DATA xuong thap
- SCK co xung tiep theo, VDK giu Data o thap, Luc nay SHT biet la VDK muon giao tiep voi no
- Dua chan DATA len 1, chuan bi qua trinh truyen du lieu xuong SHT
*/
void SHT_Tranmission_Start()
{
	P1DIR |= SDA;			//Chan P1.4(DATA) duoc chon lam chan output
	SDA_H;					//P1OUT |= SDA

	SCK_L;					//P1OUT &= ~SDA
	SCK_H;

	SDA_L;

	SCK_L;
	SCK_H;

	SDA_H;

	SCK_L;

	P1DIR &= ~SDA;
}
/*Gửi lệnh xuống SHT
	+Gửi 0: cho chân DATA xuống 0, sau đó kích xung SCK từ Thấp lên Cao.
	Khi đó SHT11 sẽ đọc tín hiệu tại chân DATA và nhận giá trị
	+Gửi 1: cho chân DATA lên 1, sau đó kích xung SCK từ thấp lên cao.
	VĐK gửi lệnh 8 bít xuống SHT11
		· 3 bit đầu là 0
		· 5 bit sau xác định yêu cầu của VĐK với SHT11
Kiểm tra lỗi:
Sau khi gửi đủ 8 bit lệnh xuống SHT11, thì chúng ta kiểm tra lỗi
	+Cho chân DATA lên 1, xau đó chuyển chế độ là chân INPUT
	+Kích 1 xung từ thấp lên cao tại SCK
	+Đọc lại chân DATA
		· Nếu =0: Gửi lệnh OK
		· Nếu =1 : Gửi lệnh có lỗi.
Nếu gửi lệnh ok, ta chờ 1 thời gian để SHT11 đo nhiệt độ và gửi lại data cho VDK.
Lúc này chân DATA vẫn là chân INPUT. SHTT11 sẽ kéo chân DATA lên 1 trong quá trình đo chờ kết quả.
 */
char SHT_Write_Byte(unsigned char value)
{
	unsigned char i, error = 0;
	P1DIR |= SDA; 		//Chan P1.4(DATA) duoc chon lam chan output
	for(i = 0x80; i > 0; i /= 2)	//shift bit for masking
	{
		if(i & value)
			SDA_H;            //masking value with i, send bit 1
		else
			SDA_L;			//send bit 0
		SCK_H;            //clk for gate bus(Figure 17 datasheet)
		delay_us(5);		//pulswith approx. 5 us
		SCK_L;
	}
	SDA_H;                   /* release DATA-line
								SHT indicates the proper reception of a command by
								pulling the DATA pin low (ACK bit) after the falling edge of
								the 8th SCK clock. The DATA line is released (and goes
								high) after the falling edge of the 9th SCK clock */
	P1DIR &= ~SDA;           //Change SDA to be input 0:input 1:ouput
	SCK_H;                   //clk #9 for ack
	error = P1IN;           //check ack (DATA will be pulled down by SHT11)
	error &= SDA;
	P1DIR |= SDA;           //Change SDA to be output 0:input 1:ouput
	SCK_L;
	if(error)
		return 1;                     //error=1 in case of no acknowledge
	return 0;
}

/*
- Quá trình đọc kết quả:
o Quá trình đọc dữ liệu bắt đầu khi chân DATA được SHT11 kéo xuống thấp.
Khi đó SHT thông báo với VĐK xử lý xong ( đo xong nhiệt độ, hoặc độ ẩm)
o Dữ liệu kết quả độ ẩm hay nhiệt độ SHT11 gửi tới VĐK đều dạng 16 bit.
Cứ sau 8 bit được gửi từ SHT11, VĐK lại truyền xuống 1 bit ACK = 0 tới SHT11.
Khi nhận được bit này, SHT11 sẽ truyền byte tiếp theo lên.
o Ngoài 16 bit dữ liệu ra, SHT11 còn gửi lại 1 byte CheckSum. Ta có thể sử dụng hoặc không
o Nếu không sử dụng thì sau khi nhận được 8 bit byte thấp ta cho tín hiệu ACK =1.
Khi đó SHT11 sẽ chuyển sang chế độ Sleep. Và chuẩn bị cho lệnh tiếp theo.
Đọc byte cao dữ liệu(MSB):
· Chân DATA VĐK là đầu vào
· Khi kích xung SCK từ thấp lên cao, sau đó đọc dữ liệu từ chân DATA
· Bit 0: DATA = 0;
· Bit 1 : DATA =1;
· Khi đọc được 8 bit. Chuyển chân DATA của VĐK thành chân Output
· VĐK kéo chân DATA = 0;
· Có xung kích SCK từ thấp lên cao. ( truyền bit ACK).
Lặp lại quá trình trên để đọc byte thấp.
Byte CheckSum có thể đọc hoặc không.
Nếu đọc thì chú ý bit ACK =1. Để SHT11 chuyển vào trạng thái Sleep. Chuẩn bị quá trình làm việc tiếp theo.
 */

char SHT_Read_Byte(unsigned char ack)
{
	// reads a byte form the sensor bus and gives an acknowledge in case of "ack=1"
	unsigned char i, val = 0;
	P1DIR |= SDA;           //Change SDA to be output 0:input 1:ouput
	SDA_H;                           //release DATA-line
	P1DIR &= ~SDA;                     //Change SDA to be input 0:input 1:ouput
	for(i = 0x80; i > 0; i /= 2)             //shift bit for masking
	{
		SCK_H;                     //clk for SENSI-BUS
		if(P1IN & SDA)
		  val = (val | i);               //read bit
		  SCK_L;
	}
	P1DIR |= SDA;                      //Change SDA to be output 0:input 1:ouput
	if (ack)                        //in case of "ack==1" pull down DATA-Line
		SDA_L;
	else
		SDA_H;
	SCK_H;                           //clk #9 for ack
	delay_us(5);				///pulswith approx. 5 us
	SCK_L;
	SDA_H;                           //release DATA-line
	P1DIR &= ~SDA;                     //Change SDA to be output 0:input 1:ouput
	return val;
}

//Datasheet 3.4
void SHT_Connection_Reset()
{
	unsigned char ClkCnt;		//Clk count
	P1DIR |= SDA;              	//Change SDA to be input 0:input 1:ouput
	SDA_H;
	SCK_L;						//Initial state
	for(ClkCnt = 0; ClkCnt < 9; ClkCnt++)      //9 SCK cycles
	{
		SCK_H;
		SCK_L;
	}
	SHT_Tranmission_Start();                      //transmission start
}

char SHT_Soft_Reset()
{
	unsigned char error = 0;
	SHT_Connection_Reset();           //reset communication
	error += SHT_Write_Byte(RESET);   //send RESET-command to sensor
	return error;                     //error=1 in case of no response form the sensor
}

unsigned char SHT_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
	unsigned error = 0;
	unsigned int i;

	SHT_Tranmission_Start();          //transmission start
	switch (mode)
	{                                 //send command to sensor
		case TEMPERATURE:
			error += SHT_Write_Byte(MEASURE_TEMP);
		break;
		case HUMIDITY:
			error += SHT_Write_Byte(MEASURE_HUMI);
		break;
	}
	P1DIR &= ~SDA;             		//Change SDA to be input 0:input 1:ouput
	for (i = 0; i < 65535; i++)
		if((P1IN & SDA) == 0)
			break; 					//wait until sensor has finished the measurement
	if(P1IN & SDA)
		error += 1;            		//or timeout (~2 sec.) is reached
	*(p_value) = SHT_Read_Byte(ACK);       //read the first byte (MSB) 8bit cao
	*(p_value + 1) = SHT_Read_Byte(ACK);   //read the second byte (LSB) 8bit thap
	*p_checksum = SHT_Read_Byte(NO_ACK);    //read checksum
	return error;
}

float SHT_Calculate(unsigned int *p_temperature, unsigned int *p_humidity, float *result_temp, float *result_humid)
{
	//12bit do am / 14bit nhiet do
	//do am
	const float C1 = 	-2.0468;
	const float C2 = 	0.0367;
	const float C3 = 	-0.0000015955;
	const float T1 = 	0.01;
	const float T2 =	0.00008;

	//nhiet do
	const float D1 =	-39.7;				//3.5V [℃]
	const float D2 =	0.01;				//14bit [℃]

	unsigned int SORH = 	*p_humidity;		//Humidity 12 Bit
	unsigned int SOT  = 	*p_temperature;		//Temperature 14 Bit
	float rh_Linear;                     	//Humidity linear
	float rh_True;                  			//Temperature compensation of Humidity Signal
	float temperature_C;                        // temperature_C : Temperature [℃]

	temperature_C = D1 + D2*SOT;                //calc. temperature from ticks to [℃]
	rh_Linear = C1 + C2*SORH + C3*SORH*SORH;    //calc. humidity from ticks to [%RH]
	rh_True = (temperature_C - 25)*(T1 + T2*SORH) + rh_Linear;   //calc. temperature compensated humidity [%RH]
	if(rh_True > 100)
		rh_True=100;         //cut if the value is outside of range
	if(rh_True < 0.1)
		rh_True=0.1;         //the physical possible range

	*result_temp = temperature_C;           //return temperature [℃]
	*result_humid = rh_True;            	//return humidity[%RH]

	return 0;
}



