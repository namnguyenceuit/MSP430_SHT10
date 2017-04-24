#ifndef UART_H_
#define UART_H_

void UART_Init();
void UART_Write_Char(unsigned char data);
void UART_Write_String(char* string);
void UART_Write_Int(unsigned long n);
void UART_Write_Float(float x,unsigned char coma);

//Init UART
void UART_Init()
{
	//Setup RXD, TXD (P1SEL & P2SEL Table 16, Page 43 MSP430G2553 Datasheet)
	//Positive PxSEL for every PORT
	P1SEL = BIT1 + BIT2;	//P1.1 = RXD, P1.2 = TXD
	P1SEL2 = BIT1 + BIT2;	//P1.1 = RXD, P1.2 = TXD

	UCA0CTL1 |= UCSWRST; //USCI Software reset, place UCA0 in reset state to be configure

	//Configure
	UCA0CTL1 |= UCSSEL_2; //SMCLK

	//clockConfigure 1MHZ selected
	//select baudrate 9600, datasheet MSP430x2xx Family, table 15.5
	//UCBRx = 6, UCBRSx = 0, UCBRFx = 8, UCOS16 = 1
	//UCBRx include two 8bit registers: UCAxBR0(low) & UCAxBR1(high)
	UCA0BR0 = 6; //6dec in 8bit-range bin
	UCA0BR1 = 0;

	//Modulation control respectively with the table
	UCA0MCTL = UCBRS_0 + UCBRF_8 + UCOS16;

	UCA0CTL1 &= ~UCSWRST; //Take UCA0 out of reset state

	IE2 |= UCA0RXIE;	//Enable interrupt when receive(Rx)
	_BIS_SR(GIE);	//Global enable interrupt
}

void UART_Write_Char(unsigned char data)
{
	while (!(IFG2 & UCA0TXIFG)); // Doi gui xong ky tu truoc
	UCA0TXBUF = data; // Moi cho phep gui ky tu tiep theo
}

void UART_Write_String(char* string)
{
	while(*string) // Het chuoi ky tu thi thoat
		{
		while (!(IFG2&UCA0TXIFG)); // Doi gui xong ky tu truoc
		UCA0TXBUF= *string; // Moi cho phep gui ky tu tiep theo
		string ++; // Lay ky tu tiep theo
		}
	//UART_Write_Char(0);
}

void UART_Write_Int(unsigned long n)
{
     unsigned char buffer[16]; //(16) unsigned long ccs
     unsigned char i,j;

     if(n == 0) {
    	 UART_Write_Char('0');
          return;
     }

     for (i = 15; i > 0 && n > 0; i--) {
          buffer[i] = (n % 10) + '0'; // to turn n%10 to %c
          n /= 10;
     }

     for(j = i+1; j <= 15; j++) {
    	 UART_Write_Char(buffer[j]);
     }
}

void UART_Write_Float(float number, unsigned char coma)
{
	unsigned long temp;

	if(number < 0)
	{
		UART_Write_Char('-');			//In so am
		number *= -1;
	}
	temp = (unsigned long) number;		// Chuyan thanh so nguyen.

	UART_Write_Int(temp);

	number = ((float)number - temp);

	if(coma != 0)
		UART_Write_Char('.');	// In ra dau "."
	while(coma > 0)
	{
		number *= 10;
		coma--;
	}
	temp = (unsigned long)(number + 0.5);	//Lam tron
	UART_Write_Int(temp);
}

#endif	/* UART_H_ */
