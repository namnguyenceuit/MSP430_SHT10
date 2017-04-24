/*
 * sht.h
 *
 *  Created on: Apr 19, 2017
 *      Author: Leisjerk
 */

#include <msp430.h>
//#include <intrinsics.h>

#ifndef LIBRARY_SHT_H_
#define LIBRARY_SHT_H_

#define CPU_F			(double)1000000
#define delay_us(x)		__delay_cycles((long)(CPU_F * (double)x/1000000.0))
#define delay_ms(x)     __delay_cycles((long)(CPU_F * (double)x/1000.0))

#define uint 			unsigned int
#define uchar 			unsigned char
#define ulong 			unsigned long

//datasheet sht1x command define
#define STATUS_REG_W 	0x06   //000 00110 		Write status register
#define STATUS_REG_R 	0x07   //000 00111 		Read status register
#define MEASURE_TEMP 	0x03   //000 00011 		Measure temperature
#define MEASURE_HUMI 	0x05   //000 00101 		Measure Relative Humidity
#define RESET      		0x1e   //000 11110 		Soft reset

//sht mode define
#define SHT_12_8_BIT   	0x01
#define SHT_14_12_BIT   0x00

//ack define
#define NO_ACK       	0
#define ACK         	1

//case
#define TEMPERATURE   	1
#define HUMIDITY      	2

//define SCK, SDA output
#define SCK         	BIT5	//P1.5 Clock
#define SDA         	BIT4	//P1.4 Data
#define SCK_H       	P1OUT |= SCK
#define SCK_L       	P1OUT &= ~SCK
#define SDA_H       	P1OUT |= SDA
#define SDA_L       	P1OUT &= ~SDA

void SHT_Init();

void SHT_Tranmission_Start();

char SHT_Write_Byte(unsigned char value);

char SHT_Read_Byte(unsigned char ack);

void SHT_Connection_Reset();

char SHT_Soft_Reset();

unsigned char SHT_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);

float SHT_Calculate(unsigned int *p_temperature, unsigned int *p_humidity, float *result_temp, float *result_humid);

typedef union
{
	unsigned int i;
	float f;
}value;

#endif /* LIBRARY_SHT_H_ */
