//=============================================================================
// *** Nunchuk64 ***
// Copyright (c) Robert Grasböck, All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================
/// @file   paddle.c
/// @author Robert Grasböck (robert.grasboeck@gmail.com)
/// @date   December, 2017
/// @brief  analog paddle input part
//=============================================================================
#include <inttypes.h>
#include <avr/interrupt.h>

#include "ioconfig.h"
#include "enums.h"
#include "led.h"
#include "paddle.h"

void paddle_init(void) {
  // SID sensing port
  DDR_SENSE_A  &= ~_BV(BIT_SENSE_A); // SENSE is input
  PORT_SENSE_A &= ~_BV(BIT_SENSE_A); // pullup off, hi-biased by OC1B

  // SID POTX/POTY port
  PORT_PADDLE_A_X &= ~(_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y)); // = PORT_PADDLE_A_Y
  DDR_PADDLE_A_X  &= ~(_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y)); // = DDR_PADDLE_A_Y

  // SID sensing port
  DDR_SENSE_B  &= ~_BV(BIT_SENSE_B); // SENSE is input
  PORT_SENSE_B &= ~_BV(BIT_SENSE_B); // pullup off, hi-biased by OC1B

  // SID POTX/POTY port
  PORT_PADDLE_B_X &= ~(_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y)); // = PORT_PADDLE_B_Y
  DDR_PADDLE_B_X  &= ~(_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y)); // = DDR_PADDLE_B_Y

  // interrupt
  EIMSK &= ~_BV(INT0);                // disable INT0
  EICRA &= ~(_BV(ISC01) | _BV(ISC00));
  EICRA |= _BV(ISC01);                // ISC11:ISC10 == 10, @negedge

  // interrupt
  EIMSK &= ~_BV(INT1);                // disable INT1
  EICRA &= ~(_BV(ISC11) | _BV(ISC10));
  EICRA |= _BV(ISC11);                // ISC11:ISC10 == 10, @negedge
}

#define P1_MIN_TIMER     23
#define P1_MAX_TIMER     51
#define P1_RANGE         (P1_MAX_TIMER - P1_MIN_TIMER)

#define P2_MIN_TIMER     27
#define P2_MAX_TIMER     55
#define P2_RANGE         (P2_MAX_TIMER - P2_MIN_TIMER)

static volatile uint8_t ocr1a_load = P1_MIN_TIMER + (P1_RANGE / 2); ///< precalculated OCR1A value (A XPOT)
static volatile uint8_t ocr1b_load = P1_MIN_TIMER + (P1_RANGE / 2); ///< precalculated OCR1B value (A YPOT)
static volatile uint8_t ocr0a_load = P2_MIN_TIMER + (P2_RANGE / 2); ///< precalculated OCR0A value (B XPOT)
static volatile uint8_t ocr0b_load = P2_MIN_TIMER + (P2_RANGE / 2); ///< precalculated OCR0B value (B YPOT)

static volatile uint8_t a_enabled = 0xff;
static volatile uint8_t b_enabled = 0xff;

void paddle_start(Port port) {

  if (port == PORT_A) {

    if (a_enabled == 1)
      return;

    TCCR1B = 0;

    OCR1A = P1_MIN_TIMER + (P1_RANGE / 2);
    OCR1B = P1_MIN_TIMER + (P1_RANGE / 2);

    DDR_PADDLE_A_X  |= (_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y));   // enable POTX/POTY as outputs
    PORT_PADDLE_A_X |= (_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y));   // output "1" on both

    EIFR  |= _BV(INTF0);  // clear INT0 flag
    EIMSK |= _BV(INT0);   // enable INT0

    a_enabled = 1;

  } else {

    if (b_enabled == 1)
      return;

    TCCR0B = 0; // Port B

    OCR0A = P2_MIN_TIMER + (P2_RANGE / 2); // Port B
    OCR0B = P2_MIN_TIMER + (P2_RANGE / 2); // Port B

    DDR_PADDLE_B_X  |= (_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y));   // enable POTX/POTY as outputs
    PORT_PADDLE_B_X |= (_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y));   // output "1" on both

    EIFR  |= _BV(INTF1);  // clear INT1 flag
    EIMSK |= _BV(INT1);   // enable INT1

    b_enabled = 1;
  }
}

void paddle_stop(Port port) {
  if (port == PORT_A) {

    if (a_enabled == 0)
      return;

    DDR_PADDLE_A_X  &= ~(_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y));   // disable POTX/POTY as outputs
    PORT_PADDLE_A_X &= ~(_BV(BIT_PADDLE_A_X) | _BV(BIT_PADDLE_A_Y));

    EIMSK &= ~_BV(INT0);  // disable INT0

    a_enabled = 0;

  } else {

    if (b_enabled == 0)
      return;

    DDR_PADDLE_B_X  &= ~(_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y));   // disable POTX/POTY as outputs
    PORT_PADDLE_B_X &= ~(_BV(BIT_PADDLE_B_X) | _BV(BIT_PADDLE_B_Y));

    EIMSK &= ~_BV(INT1);  // disable INT1

    b_enabled = 0;
  }
}

void paddle_update(Paddle *port_a, Paddle *port_b, LED_State mode) {

  // ===================================
  //  CONTROL PORT A
  // ===================================

  // ocr1a_load  x
  // ocr1b_load  y

  // ocr0a_load  x
  // ocr0b_load  y
  uint16_t a_x = port_a->axis_x;
  uint16_t a_y = port_a->axis_y;
  uint16_t b_x = port_b->axis_x;
  uint16_t b_y = port_b->axis_y;

  if (mode == LED_BLINK3) {
    a_x = port_a->axis_x;
    a_y = port_b->axis_x;
    b_x = 0;
    b_y = 0;
  } 

  if (a_x > 1024)
    a_x = 1024;

  if (a_y > 1024)
    a_y = 1024;

  if (b_x > 1024)
    b_x = 1024;

  if (b_y > 1024)
    b_y = 1024;

  ocr1a_load = P1_RANGE - ((a_x * P1_RANGE) / 1024) + P1_MIN_TIMER;
  ocr1b_load = P1_RANGE - ((a_y * P1_RANGE) / 1024) + P1_MIN_TIMER;

  ocr0a_load = P2_RANGE - ((b_x * P2_RANGE) / 1024) + P2_MIN_TIMER;
  ocr0b_load = P2_RANGE - ((b_y * P2_RANGE) / 1024) + P2_MIN_TIMER;
/*
  if (ocr1a_load < P1_MIN_TIMER)
    ocr1a_load = P1_MIN_TIMER;
  else if (ocr1a_load > P1_MAX_TIMER)
    ocr1a_load = P1_MAX_TIMER;

  if (ocr1b_load < P1_MIN_TIMER)
    ocr1b_load = P1_MIN_TIMER;
  else if (ocr1b_load > P1_MAX_TIMER)
    ocr1b_load = P1_MAX_TIMER;

  if (ocr0a_load < P2_MIN_TIMER)
    ocr0a_load = P2_MIN_TIMER;
  else if (ocr0a_load > P2_MAX_TIMER)
    ocr0a_load = P2_MAX_TIMER;

  if (ocr0b_load < P2_MIN_TIMER)
    ocr0b_load = P2_MIN_TIMER;
  else if (ocr0b_load > P2_MAX_TIMER)
    ocr0b_load = P2_MAX_TIMER;
 */
}

/// SID measuring cycle detected.
///
/// 1. SID pulls POTX low\n
/// 2. SID waits 256 cycles us\n
/// 3. SID releases POTX\n
/// 4. 0 to 255 cycles until the cap is charged\n
///
/// This handler stops the Timer1, clears OC1A/OC1B outputs,
/// loads the timer with values precalculated in potmouse_movt()
/// and starts the timer.
///
/// OC1A/OC1B (YPOT/XPOT) lines will go up by hardware.
/// Normal SID cycle is 512us. Timer will overflow not before 65535us.
/// Next cycle will begin before that so there's no need to stop the timer.
/// Output compare match interrupts are thus not used.

ISR(INT0_vect) {
  // ===========================================================
  // SID started to measure the pots, uuu
  // disable INT0 until the measurement cycle is complete


  // ===========================================================
  // stop the timer
  TCCR1B = 0;

  // clear OC1A/OC1B:
  // 1. set output compare to clear OC1A/OC1B ("10" in table 37 on page 97)
  TCCR1A = _BV(COM1A1) | _BV(COM1B1);
  // 2. force output compare to make it happen
  TCCR1C |= _BV(FOC1A) | _BV(FOC1B);

  // Set OC1A/OC1B on Compare Match (Set output to high level)
  // WGM13:0 = 00, normal mode: count from BOTTOM to MAX
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0);

  // load the timer
  TCNT1 = 0;

  // init the output compare values
  OCR1A = ocr1a_load;
  OCR1B = ocr1b_load;

  // start timer with prescaler clk/8 (1 count = 1us)
  // TCCR1B |= _BV(CS11) | _BV(CS10);
  TCCR1B |= _BV(CS11);
}

ISR(INT1_vect) {

  // ===========================================================
  // SID started to measure the pots, uuu
  // disable INT1 until the measurement cycle is complete


  // ===========================================================
  // stop the timer
  TCCR0B = 0;

  // clear OC0A/OC0B:
  // 1. set output compare to clear OC0A/OC0B ("10" in table 37 on page 97)
  TCCR0A = _BV(COM0A1) | _BV(COM0B1);
  // 2. force output compare to make it happen
  TCCR0B |= _BV(FOC0A) | _BV(FOC0B);

  // Set OC0A/OC0B on Compare Match (Set output to high level)
  // WGM13:0 = 00, normal mode: count from BOTTOM to MAX
  TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(COM0B1) | _BV(COM0B0);

  // load the timer
  TCNT0 = 0;

  // init the output compare values
  OCR0A = ocr0a_load;
  OCR0B = ocr0b_load;

  // start timer with prescaler clk/8 (1 count = 1us)
  //TCCR0B |= _BV(CS01) | _BV(CS00);
  TCCR0B |= _BV(CS01);
}
