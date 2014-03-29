/*
 * blink.c -- LED matrix binary pattern blinker
 *
 * Copyright (C) 2008, jw@suse.de, distribute under GPL, use with mercy.
 *
 * total current consumption
 * - @ 8mhz, running row interrupt clk/8
 * All LEDs off, 		16.08 mA
 *
 * 1 Row  130ohm, 8 on:   35.50 mA
 * 1 Row  130ohm, 4 on:   25.80 mA
 * 1 Row  130ohm, 1 on:   18.50 mA
 *
 * 1 Row  390ohm, 8 on:   24.38 mA
 * 1 Row  390ohm, 4 on:   20.25 mA
 * 1 Row  390ohm, 1 on:   17.15 mA
 *
 * 2 Rows 390ohm, 8 on:   32.10 mA
 * 3 Rows 390ohm, 8 on:   38.70 mA
 * 4 Rows 390ohm, 8 on:   44.60 mA
 * 5 Rows 390ohm, 8 on:   49.50 mA
 * 6 Rows 390ohm, 8 on:   54.20 mA
 * 7 Rows 6x390+1x130, 8: 64.20 mA
 * 8 Rows 7x390+1x130, 8: 67.20 mA
 *
 *                         5.6V
 * 1 Rows 100ohm, 8 on:   39.10 mA
 * 2 Rows 100ohm, 8 on:   55.70 mA
 * 3 Rows 100ohm, 8 on:   67.20 mA
 * 4 Rows 100ohm, 8 on:   75.50 mA
 * 5 Rows 100ohm, 8 on:   81.10 mA
 * 6 Rows 100ohm, 8 on:   85.20 mA
 * 7 Rows 100ohm, 8 on:   90.30 mA
 * 8 Rows 100ohm, 8 on:  105.20 mA
 * columns 3,5,6 start flickering under full load at 5.6V
 * everything is smooth at 5.05V from USB.
 *
 */
// #include "config.h"
#include "cpu_mhz.h"

// Pollin D03881, Art.Nr 120 542
#define M_A1 PB0		// H
#define M_A2 PD4		// C
#define M_A3 PB5		// 8
#define M_A4 PD5		// E
#define M_A5 PC5		// 1
#define M_A6 PB2		// 7
#define M_A7 PC4		// 2
#define M_A8 PC1		// 5

#define M_K1 PB1		// D
#define M_K2 PC3		// 3
#define M_K3 PC2		// 4
#define M_K4 PD7		// G
#define M_K5 PC0		// 6
#define M_K6 PD6		// F
#define M_K7 PD3		// B
#define M_K8 PD2		// A



#include <util/delay.h>			// needs F_CPU from cpu_mhz.h
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t matrix[8];
volatile uint8_t byte_mirror = 0;
volatile uint8_t vertical = 0;

ISR(TIMER0_OVF_vect)
{
  static uint8_t col = 0;
  if (++col > 7) col = 0;

  // power down all cathodes, so that we do not smear while moving the anode.
  DDRB = DDRC = DDRD = 0;

  // power up one anode row, for fan out through the cathode resistors
  if      (col == 0) { DDRB |=  (1<<PB0); }
  else if (col == 1) { DDRD |=  (1<<PD4); }
  else if (col == 2) { DDRB |=  (1<<PB5); }
  else if (col == 3) { DDRD |=  (1<<PD5); }
  else if (col == 4) { DDRC |=  (1<<PC5); }
  else if (col == 5) { DDRB |=  (1<<PB2); }
  else if (col == 6) { DDRC |=  (1<<PC4); }
  else               { DDRC |=  (1<<PC1); }

  if (vertical)
    {
      uint8_t row = matrix[col];
      // power up the cathode bits for this row
      if (byte_mirror)
	{
	  if (row & 0x01) DDRB |= (1<<PB1); else DDRB &= ~(1<<PB1);
	  if (row & 0x02) DDRC |= (1<<PC3); else DDRC &= ~(1<<PC3);
	  if (row & 0x04) DDRC |= (1<<PC2); else DDRC &= ~(1<<PC2);
	  if (row & 0x08) DDRD |= (1<<PD7); else DDRD &= ~(1<<PD7);
	  if (row & 0x10) DDRC |= (1<<PC0); else DDRC &= ~(1<<PC0);
	  if (row & 0x20) DDRD |= (1<<PD6); else DDRD &= ~(1<<PD6);
	  if (row & 0x40) DDRD |= (1<<PD3); else DDRD &= ~(1<<PD3);
	  if (row & 0x80) DDRD |= (1<<PD2); else DDRD &= ~(1<<PD2);
	}
      else
	{
	  if (row & 0x80) DDRB |= (1<<PB1); else DDRB &= ~(1<<PB1);
	  if (row & 0x40) DDRC |= (1<<PC3); else DDRC &= ~(1<<PC3);
	  if (row & 0x20) DDRC |= (1<<PC2); else DDRC &= ~(1<<PC2);
	  if (row & 0x10) DDRD |= (1<<PD7); else DDRD &= ~(1<<PD7);
	  if (row & 0x08) DDRC |= (1<<PC0); else DDRC &= ~(1<<PC0);
	  if (row & 0x04) DDRD |= (1<<PD6); else DDRD &= ~(1<<PD6);
	  if (row & 0x02) DDRD |= (1<<PD3); else DDRD &= ~(1<<PD3);
	  if (row & 0x01) DDRD |= (1<<PD2); else DDRD &= ~(1<<PD2);
	}
    }
  else
    {
      uint8_t bit = byte_mirror ? (0x80>>col) : (1<<col);

      // power up the cathode bits for this row
#if 0
      if (byte_mirror)
	{
	  if (matrix[0] & bit) DDRB |= (1<<PB1); else DDRB &= ~(1<<PB1);
	  if (matrix[1] & bit) DDRC |= (1<<PC3); else DDRC &= ~(1<<PC3);
	  if (matrix[2] & bit) DDRC |= (1<<PC2); else DDRC &= ~(1<<PC2);
	  if (matrix[3] & bit) DDRD |= (1<<PD7); else DDRD &= ~(1<<PD7);
	  if (matrix[4] & bit) DDRC |= (1<<PC0); else DDRC &= ~(1<<PC0);
	  if (matrix[5] & bit) DDRD |= (1<<PD6); else DDRD &= ~(1<<PD6);
	  if (matrix[6] & bit) DDRD |= (1<<PD3); else DDRD &= ~(1<<PD3);
	  if (matrix[7] & bit) DDRD |= (1<<PD2); else DDRD &= ~(1<<PD2);
	}
      else
#endif
	{
	  if (matrix[7] & bit) DDRB |= (1<<PB1); else DDRB &= ~(1<<PB1);
	  if (matrix[6] & bit) DDRC |= (1<<PC3); else DDRC &= ~(1<<PC3);
	  if (matrix[5] & bit) DDRC |= (1<<PC2); else DDRC &= ~(1<<PC2);
	  if (matrix[4] & bit) DDRD |= (1<<PD7); else DDRD &= ~(1<<PD7);
	  if (matrix[3] & bit) DDRC |= (1<<PC0); else DDRC &= ~(1<<PC0);
	  if (matrix[2] & bit) DDRD |= (1<<PD6); else DDRD &= ~(1<<PD6);
	  if (matrix[1] & bit) DDRD |= (1<<PD3); else DDRD &= ~(1<<PD3);
	  if (matrix[0] & bit) DDRD |= (1<<PD2); else DDRD &= ~(1<<PD2);
	}
    }
}

int main()
{
  // prepare the anode drivers high, and the cathode drivers low.
  PORTB = (1<<PB0)|(1<<PB5)|(1<<PB2);
  PORTC = (1<<PC5)|(1<<PC4)|(1<<PC1);
  PORTD = (1<<PD4)|(1<<PD5);

  TCCR0 = (0<<CS02) | (1<<CS01) | (0<<CS00);	// 101:clk/1024, 100:clk/256, 011:clk/64, 010:clk/8
  TIMSK |= (1<<TOIE0);				// enable overflow interrupt
  sei();

  uint8_t i, j = 0, not = 0, reverse = 0;

  for (;;)
    {
#if 0
      matrix[7] = 0xff;
      _delay_ms(2000.0);
      matrix[6] = 0xff; 
      _delay_ms(2000.0);
      matrix[5] = 0xff;
      _delay_ms(2000.0);
      matrix[4] = 0xff;
      _delay_ms(2000.0);
      matrix[3] = 0xff;
      _delay_ms(2000.0);
      matrix[2] = 0xff;
      _delay_ms(2000.0);
      matrix[1] = 0xff;
      _delay_ms(2000.0);
      matrix[0] = 0xff;
      _delay_ms(2000.0);
      matrix[0] = matrix[1] = matrix[2] = matrix[3] = 
      matrix[4] = matrix[5] = matrix[6] = matrix[7] = 0;
#endif

#if 1
      if (reverse)
	{
	  for (i = 0; i < 7; i++) { matrix[i] = matrix[i+1]; }
	  matrix[7] = not ? (255-j) : j;
	}
      else
        {
	  for (i = 7; i > 0; i--) { matrix[i] = matrix[i-1]; }
	  matrix[0] = not ? (255-j) : j;
	}
      if (++j == 0) not = 1 - not;
      if (j == 0 && not) byte_mirror = 1 - byte_mirror;
      if (j == 0 && not && byte_mirror) vertical = 1 - vertical;
      if (j == 0 && not && byte_mirror && vertical) reverse = 1 - reverse;
      _delay_ms(40.0); 
#endif
    }
}
