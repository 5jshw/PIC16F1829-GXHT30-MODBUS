
// CONFIG1
#pragma config FOSC = INTOSC	// Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF		// Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF		// Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON		// MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF			// Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF		// Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF		// Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF	// Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON		// Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON		// Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF		// Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF		// PLL Enable (4x PLL enabled)
#pragma config STVREN = ON		// Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO		// Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config DEBUG = OFF		// In-Circuit Debugger Mode (In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins)
#pragma config LVP = OFF		// Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 8000000

void main_init(void);	// 主程序初始化
void bit_Value(void);	// 数值拆分函数
void EEPROM_Write(char const *eAddr, char *eDate);	// EEPROM写入函数
void EEPROM_Read(char const *eAddr, char *eDate);	// EEPROM读取函数

#define SDA		PORTBbits.RB6	// I2C数据
#define T_SDA	TRISBbits.TRISB6

#define SCL		PORTBbits.RB5	// I2C时钟
#define T_SCL	TRISBbits.TRISB5

#define DI		PORTCbits.RC4	// 485发送
#define T_DI	TRISCbits.TRISC4

#define RO		PORTCbits.RC5	// 485接收
#define T_RO	TRISCbits.TRISC5
