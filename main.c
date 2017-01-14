/*
 * TMP102.c
 *
 * Created: 12/28/2016 2:20:14 PM
 * Author : fairenough
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL)) - 1))



void USART_init()
{
	UBRR0H = (unsigned char)BAUD_PRESCALE >> 8;
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

unsigned char USART_getc()
{
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void USART_putc(unsigned char ch)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = ch;
}

void USART_puts (unsigned char* string)
{
	while(*string > 0)
	{
		USART_putc(*string++);
	}
}

/**********************************************************/
#define TMP102_ADDRESS_W				0x92
#define TMP102_ADDRESS_R				0x93
#define TMP102_TEMP_REGISTER			0x00
#define TMP102_CONFIG_REGISTER			0x01
#define TMP102_CONFIG_DATA_BYTE_1		0x60            //Normal mode operation
#define TMP102_CONFIG_DATA_BYTE_2		0xA0            //Normal mode operation

unsigned char tempHighByte;
unsigned char tempLowByte;
float temp;

void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_send (unsigned char data);
unsigned char i2c_readAck();
unsigned char i2c_readNoAck();


void i2c_init()
{
	TWSR = 0x00;            //set prescaler bits to zero
	TWBR = 72;                // BR = (F_CPU) / (16 + 2*TWBR * 1) = 100kHz
	TWCR |= 1 << TWEN;        //enable TWI module
}

void i2c_start()
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 <<TWINT)));
}

void i2c_stop()
{
	TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	while (!(TWCR & (1 << TWSTO)));
}
void i2c_send (unsigned char data)
{
	TWDR = data;
	TWCR |= (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

unsigned char i2c_readAck()
{
	TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

unsigned char i2c_readNoAck()
{
	TWCR |= (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}


int main(void)
{
	i2c_init();
	USART_init();
	char buffer[32];
	char buffer2[15];
	
	while (1)
	{
		//USART_putc(USART_getc());
		
		itoa(52, buffer2, 10);
		USART_puts(buffer2);
		
		i2c_start();
		i2c_send(TMP102_ADDRESS_W);
		i2c_send(TMP102_TEMP_REGISTER);
		
		i2c_start();
		i2c_send(TMP102_ADDRESS_R);
		tempHighByte = i2c_readNoAck();
		tempLowByte = i2c_readAck();
		i2c_stop();
		
		
		
		/*
		i2c_start();
		i2c_send(TMP102_ADDRESS_W);
		i2c_send(TMP102_CONFIG_REGISTER);
		i2c_send(TMP102_CONFIG_DATA_BYTE_1);
		i2c_send(TMP102_CONFIG_DATA_BYTE_2);
		i2c_stop();
		
		i2c_start();
		i2c_send(TMP102_ADDRESS_R);
		tempHighByte = i2c_read_ack();
		tempLowByte = i2c_read_noAck();
		i2c_stop();
		*/
		temp = 0.0625 * ( (tempHighByte << 4) | (tempLowByte >>4) );
		itoa((int)temp, buffer, 10);
		USART_puts(buffer);
		USART_puts("TEMP:");
	}
}
