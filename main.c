#include "library/sht.h"
#include "library/UART.h"

/*
 * main.c
 */
void main()
{
	value humi_val, temp_val;
	unsigned char error, checksum;
	unsigned int temphigh, templow, humihigh, humilow;

	WDTCTL = WDTPW+WDTHOLD; //Stop watchdog timer

	//DCO 1MHZ
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	SHT_Init();
	SHT_Connection_Reset();

	//Test via UART
	UART_Init();

	while(1)
	{
		error = 0;
		error += SHT_Measure((unsigned char*) &humi_val.i, &checksum, HUMIDITY);   //measure humidity
		error += SHT_Measure((unsigned char*) &temp_val.i, &checksum, TEMPERATURE); //measure temperature
		if(error != 0)
		  SHT_Connection_Reset();//in case of an error: connection reset
		else
		{
		  //12bit humidity MSB=0x0000，XXXX(4bits valid) LSB=0xXXXX XXXX(valid)
		  humihigh = ((humi_val.i & 0x0f) << 8);
		  humilow = ((humi_val.i & 0xff00) >> 8);
		  humi_val.i = humihigh + humilow;		//Humidity

		  //14bit temperature MSB=0x00XX XXXX( 6bit valid) LSB=0XXXX XXXXvalid)
		  temphigh = ((temp_val.i & 0x3f) << 8);
		  templow = ((temp_val.i & 0xff00) >> 8);
		  temp_val.i = temphigh + templow;		//Temperature

		  SHT_Calculate(&temp_val.i, &humi_val.i, &temp_val.f, &humi_val.f);         //calculate humidity, temperature

		  //UART Test
		  UART_Write_String("Temperature: ");
		  UART_Write_Float(temp_val.f, 2);
		  UART_Write_Char(' ');
		  UART_Write_String("Humidity: ");
		  UART_Write_Float(humi_val.f, 2);
		  UART_Write_Char(10);
		}

		__delay_cycles(1000000*10);	//wait approx. 10s to avoid heating up SHT
	}
}
