#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
 
char uart_getchar(void);
int uart_putchar(char c);


static int uart0_putchar(char c, FILE *stream);
static int uart0_getchar(FILE *stream);
 
/* Buffersizes must be 2^n */
#define TBUFSIZE 64
#define RBUFSIZE 64

#define TMASK  (TBUFSIZE-1)
#define RMASK  (RBUFSIZE-1)

static volatile unsigned char tbuf[TBUFSIZE];
static volatile unsigned char rbuf[RBUFSIZE];

static volatile unsigned char t_in;
static volatile unsigned char t_out;

static volatile unsigned char r_in;
static volatile unsigned char r_out;

 
static FILE mystdin  = FDEV_SETUP_STREAM (NULL, uart0_getchar, _FDEV_SETUP_READ);
static FILE mystdout = FDEV_SETUP_STREAM (uart0_putchar, NULL, _FDEV_SETUP_WRITE);

// Init USART.  
// We use USART0, transmit pin on PD1.
void uart0_init (void)
{
    t_in = t_out = 0;
    r_in = r_out = 0;
    
    UCSR0B = 0x00; //disable while setting baud rate
    UCSR0C = 0x06;

    // Settings found from Atmega328p datasheet: "Examples of Baud Rate Setting"

#if (F_CPU == 8000000L) && (UART_BAUD_RATE == 57600)
    UCSR0A = 1<<U2X0; 
    UBRR0L = 16;    
    UBRR0H = 0x00; 

#elif (F_CPU == 16000000L) && (UART_BAUD_RATE == 57600)
    UCSR0A = 1<<U2X0;  
    UBRR0L = 34; 
    UBRR0H = 0x00; 
#elif (F_CPU == 8000000L) && (UART_BAUD_RATE == 115200)
    UCSR0A = 1<<U2X0;  
    UBRR0L = 8; 
    UBRR0H = 0x00; 
#elif (F_CPU == 16000000L) && (UART_BAUD_RATE == 115200)
    UCSR0A = 1<<U2X0;  
    UBRR0L = 16; 
    UBRR0H = 0x00; 
#else
#error("F_CPU and UART_BAUD_RATE must be defined");
#endif
    UCSR0B = (1<<RXCIE0) | (1<<TXEN0)|(1<<RXEN0);

    stdin  = &mystdin;
    stdout = &mystdout;
}


ISR( USART_RX_vect )
{
    /* Receive interrupt */
    char c;

    c = UDR0;  // Get received char
    rbuf[r_in & RMASK] = c;
    r_in++;
}

ISR( USART_UDRE_vect )
{
    /* Data register empty indicating next char can be transmitted */
    if(t_in != t_out) {
       UDR0 = tbuf[t_out & TMASK];
       t_out++;
    } else {
       UCSR0B &= ~(1<<UDRIE0);         /* disable UDRE interrupt */
    }

}

static char tbuflen(void) {
    return(t_in - t_out);
}

static int uart0_putchar(char c, FILE *stream) {
    /* Fills the transmit buffer, if it is full wait */
    while((TBUFSIZE - tbuflen()) <= 2);

   /* Add data to the transmit buffer */
   if (c == '\n') {
       tbuf[t_in & TMASK] = '\r';
       t_in++;
   }

   tbuf[t_in & TMASK] = c;
   t_in++;

   UCSR0B |= (1<<UDRIE0);            /* enable UDRE interrupt */

   return(0);
}

int uart_putchar(char c)
{
    return uart0_putchar(c, NULL);
}

static char rbuflen(void) {
    return(r_in - r_out);
}

int uart0_getchar(FILE *stream)
{
    unsigned char c;

    while(rbuflen() == 0);

    c = rbuf[r_out & RMASK];
    r_out++;

    return(c);
}

char uart_getchar(void)  // non-blocking version
{
    unsigned char c;

    if (rbuflen() == 0)
       return 0;

    c = rbuf[r_out & RMASK];
    r_out++;

    return(c);
}



