#include <stdio.h>
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include "tmr.h"
 

// function prototypes
void uart0_init(void);    // uart0.c
char uart_getchar(void);  
int uart_putchar(char c);

int main (void)
{
    uint32_t now, led_ms; // ms
    unsigned char c;

    DDRB  |= 1 << 5;
    uart0_init();
    tmr_init();

    sei();

    printf("hello world\n");
    led_ms = tmr_count_ms();
    while (1) {
        now = tmr_count_ms();  

        if ((now - led_ms) >= 1000) {
            led_ms = now;
            PORTB ^= 1 << 5;
            printf("%ld ms\n", now);
        }

        c = uart_getchar();
        if (c) {
            uart_putchar(c);
        }
    }

    return 0;
}
 
