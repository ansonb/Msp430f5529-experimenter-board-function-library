#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include "HAL_Board.h"
#include <stdint.h>
#include "driverlib.h"
#include "grlib.h"
#include "Dogs102x64_UC1701.h"
#include "stdio.h"
#include "math.h"

#include "HAL_Buttons.h"
#include "HAL_Board.h"
#include "HAL_Dogs102x6.h"
#include "HAL_Wheel.h"

uint16_t timeoutCounter;
tContext g_sContext;
tRectangle g_sRect;

extern void stop_watchdog();

extern void adc_init();

extern int adc_read();

extern void usart_init();

extern char usart_receive_char();

extern void usart_transmit_char(char ch);

extern void usart_transmit_string(char* str);

extern void usart_transmit_int_as_string(int i);

extern short charReceived;

extern void usart_init_async();

extern short usart_receive_char_async(char* ch);

extern void delay_us(int us);

extern void delay_ms(int ms);

extern void delay_s(int s);

extern void Clock_init(void);

void plot();

void lcd_init();

void print_int(int column,int row,int n);

void print_string(int column,int row,char *s);

void print_char(int column,int row,const char s);

void highlight_and_print_string(int column,int row,char *s);

char eeprom_read_byte(unsigned int address);

void eeprom_write_bytes(char* bytes, unsigned int num_bytes);

//Initialise switches s1 and s2 on board
void switch_init();
