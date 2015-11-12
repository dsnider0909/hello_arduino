
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "tmr.h"

#ifndef F_CPU
#error("F_CPU not defined");
#endif

static volatile uint32_t count_ms;

void tmr_init(void)
{
 // Setup 1ms counter

 // prescale - divide down sys clock by 64 
 // output compare for 1kHz 
 // set timer to clear upon compare match

 TCNT0 = 0 ;
 ASSR  = 0 ;          // AS0 = 0 - use internal clk for timers

#if ((F_CPU < 16384000L) && (F_CPU > 128000L))
 OCR0A  = ((F_CPU / 1000) / 64) - 1;  // divide clock source down to 1mS
#else
#error "F_CPU out of range"
#endif
                      
 TCCR0A = 0x02;       //  COM0A1 COM0A0 COM0B1 COM0B0  -  -  WGM01 WGM00
                      //       0      0     0      0   0  0      1     0 
                      // 
                      // COM0A[1.0] = 00 - pin OC0A disconnected
                      // COM0B[1.0] = 00 - pin OC0B disconnected
                      // WGM0[2.0]  = 010 - Clear Timer on Match 
                      
 TCCR0B = 0x0B;       // FOC0A  F0C0B -  -   WGM02 CS02 CS01 CS00
                      //  0   0    0    0    1    0   0    1    1
                      //  FOC0A - 0 no forced ouput compare
                      //  FOC0B - 0
                      //  WGM[2.0] = 010 - Clear timer compare
                      //  CS[2.0]  = 011 - clock source: clk/64 from prescaler
                  
 TIMSK0 |= _BV(OCIE0A);  // enable output compare interrupt
 TIFR0  =  _BV(OCF0A) ;  // clear OCF0 interrupt 
}                                                       


ISR(TIMER0_COMPA_vect)
{
  count_ms++;
  DDRC  |= 0x01;      // debug waveform - 500hz signal 
  PORTC ^= 0x01;
}

uint32_t tmr_count_ms(void)
{
   uint32_t tmp;
   
   // Since count_ms can partially update during a read
   //  interrupts must be disabled.
   cli();
   tmp = count_ms;
   sei();
   
   return tmp;
}

void tmr_wait_ms(uint16_t ms) 
{
   uint32_t start, now;

   start = tmr_count_ms();
   do { 
     now = tmr_count_ms() - start; 
   } while ( now < ms);
}


