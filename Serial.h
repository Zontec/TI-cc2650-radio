
#ifndef SERIAL_H_
#define SERIAL_H_
#include <ti/drivers/UART.h>
#include <stdarg.h>

#define PRINTF_EN           (true)

//Base uart for Launchpad board
void initDefaultUart();

void UART_write_string(char *str);

void UART_write_byte(char data);

void UART_write_number(int num, int cpec);

void _printf(char *format, ...);

#endif
