/*
    Date: 17.10.2019
    Created by: Zontec
    All rights reserved


*/

/*
 * Module provides with serial working wunctions
 * */

#include "Serial.h"

#define MAX_NUMBER_SIZE         16 /* Max length of number can be displayed*/

UART_Handle uart;

/* Hex selector*/
char selector[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void initDefaultUart()
{
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;
    uart = UART_open(0, &uartParams);
}

void UART_write_string(char *str)
{
    UART_write(uart, str, strlen(str));
}

void UART_write_byte(char data)
{
    UART_write(uart, &data, 1);
}

void UART_write_number(int num, int cpec)
{
    char number[MAX_NUMBER_SIZE];
    int i = 0;

    if(cpec == 16)
        UART_write_string("0x");
    if(cpec == 2)
        UART_write_string("0b");

    if(!num)
    {
        UART_write_byte('0');
        return;
    }

    while(num)
    {
        number[i++] = selector[num % cpec];
        num /= cpec;
    }
    while(i--)
        UART_write_byte(number[i]);
}

/* Custom printf function using serial port*/
void _printf(char *format, ...)
{
    unsigned int i;
    char *s;

    va_list arg;
    va_start(arg, format);

    for(char *traverse = format; *traverse != '\0'; traverse++)
    {
        while(*traverse != '\0' && *traverse != '%')
        {
            UART_write_byte(*traverse);
            traverse++;
        }
        if(*traverse == '\0')
            break;
        traverse++;

        switch(*traverse)
        {
            case 'c' : i = va_arg(arg, int);     //Fetch char argument
                UART_write_byte(i);
                break;

            case 'd' : i = va_arg(arg,  int);         //Fetch Decimal/Integer argument
                if(i < 0)
                {
                    i = -i;
                    UART_write_byte('-');
                }
                UART_write_number(i, 10);
                break;

            case 's': s = va_arg(arg, char *);       //Fetch string
                UART_write_string(s);
                break;

            case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
                UART_write_number(i, 16);
                break;
            }
        }
        va_end(arg);
}



