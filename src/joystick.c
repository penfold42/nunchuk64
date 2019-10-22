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
/// @file   joystick.c
/// @author Robert Grasböck (robert.grasboeck@gmail.com)
/// @date   December, 2017
/// @brief  digital joystick part
//=============================================================================
#include "ioconfig.h"

#include "joystick.h"

void joystick_init(void) {
  // NOTE not needed, default after reset

  /*
  // dataregister for joystick outputs
  BIT_CLEAR(PORT_JOY_A0,   BIT_JOY_A0);   // UP
  BIT_CLEAR(PORT_JOY_A1,   BIT_JOY_A1);   // DOWN
  BIT_CLEAR(PORT_JOY_A2,   BIT_JOY_A2);   // LEFT
  BIT_CLEAR(PORT_JOY_A3,   BIT_JOY_A3);   // RIGHT
  BIT_CLEAR(PORT_BUTTON_A, BIT_BUTTON_A);

  BIT_CLEAR(PORT_JOY_B0,   BIT_JOY_B0);   // UP
  BIT_CLEAR(PORT_JOY_B1,   BIT_JOY_B1);   // DOWN
  BIT_CLEAR(PORT_JOY_B2,   BIT_JOY_B2);   // LEFT
  BIT_CLEAR(PORT_JOY_B3,   BIT_JOY_B3);   // RIGHT
  BIT_CLEAR(PORT_BUTTON_B, BIT_BUTTON_B);

  // ddr for joystick outputs
  BIT_CLEAR(DDR_JOY_A0,   BIT_JOY_A0);
  BIT_CLEAR(DDR_JOY_A1,   BIT_JOY_A1);
  BIT_CLEAR(DDR_JOY_A2,   BIT_JOY_A2);
  BIT_CLEAR(DDR_JOY_A3,   BIT_JOY_A3);
  BIT_CLEAR(DDR_BUTTON_A, BIT_BUTTON_A);

  BIT_CLEAR(DDR_JOY_B0,   BIT_JOY_B0);
  BIT_CLEAR(DDR_JOY_B1,   BIT_JOY_B1);
  BIT_CLEAR(DDR_JOY_B2,   BIT_JOY_B2);
  BIT_CLEAR(DDR_JOY_B3,   BIT_JOY_B3);
  BIT_CLEAR(DDR_BUTTON_B, BIT_BUTTON_B);
  */
}

static volatile Joystick port_a_old = 0;
static volatile Joystick port_b_old = 0;

static volatile uint8_t  autofire_a = 0;
static volatile uint8_t  autofire_b = 0;

static volatile uint8_t  autofire2_a = 0;
static volatile uint8_t  autofire2_b = 0;

static volatile uint8_t  autofire3_a = 0;
static volatile uint8_t  autofire3_b = 0;

void joystick_update(Joystick in_port_a, uint8_t in_ext_a,
                     Joystick in_port_b, uint8_t in_ext_b,
                     LED_State mode, uint8_t switched) {

  Joystick port_a, port_b;
  uint8_t ext_a, ext_b;

  if (switched) {
    port_a = in_port_b;
    port_b = in_port_a;
    ext_a = in_ext_b;
    ext_b = in_ext_a;
  } else {
    port_a = in_port_a;
    port_b = in_port_b;
    ext_a = in_ext_a;
    ext_b = in_ext_b;
  }

  if (ext_a == 0) {
    autofire2_a = autofire3_a = 0;
  }

  if (ext_b == 0) {
    autofire2_b = autofire3_b = 0;
  }

  // look if same change?
  if (port_a == port_a_old && port_b == port_b_old) {
    // nothing changed
    return;
  }

  port_a_old = port_a;
  port_b_old = port_b;

  // ===================================
  // C64 paddle mode
  // ===================================
  if (mode == LED_BLINK3) {
    if (switched) {
      // PADDLE1 BUTTON is left
      if (port_b & BUTTON) {
	BIT_SET(DDR_JOY_B2, BIT_JOY_B2);
      } else {
	BIT_CLEAR(DDR_JOY_B2, BIT_JOY_B2);
      }

      // PADDLE2 BUTTON is right
      if (port_a & BUTTON) {
	BIT_SET(DDR_JOY_B3, BIT_JOY_B3);
      } else {
	BIT_CLEAR(DDR_JOY_B3, BIT_JOY_B3);
      }
    } else {
      // PADDLE1 BUTTON is left
      if (port_a & BUTTON) {
	BIT_SET(DDR_JOY_A2, BIT_JOY_A2);
      } else {
	BIT_CLEAR(DDR_JOY_A2, BIT_JOY_A2);
      }

      // PADDLE2 BUTTON is right
      if (port_b & BUTTON) {
	BIT_SET(DDR_JOY_A3, BIT_JOY_A3);
      } else {
	BIT_CLEAR(DDR_JOY_A3, BIT_JOY_A3);
      }
    }
  } else {
    // ===================================
    //  CONTROL PORT A
    // ===================================

    // UP
    if (port_a & UP) {
      BIT_SET(DDR_JOY_A0, BIT_JOY_A0);
    } else {
      BIT_CLEAR(DDR_JOY_A0, BIT_JOY_A0);
    }

    // DOWN
    if (port_a & DOWN) {
      BIT_SET(DDR_JOY_A1, BIT_JOY_A1);
    } else {
      BIT_CLEAR(DDR_JOY_A1, BIT_JOY_A1);
    }

    // LEFT
    if (port_a & LEFT) {
      BIT_SET(DDR_JOY_A2, BIT_JOY_A2);
    } else {
      BIT_CLEAR(DDR_JOY_A2, BIT_JOY_A2);
    }

    // RIGHT
    if (port_a & RIGHT) {
      BIT_SET(DDR_JOY_A3, BIT_JOY_A3);
    } else {
      BIT_CLEAR(DDR_JOY_A3, BIT_JOY_A3);
    }

    // BUTTON (small hack to simulate SPACE on both contollers)
    if (port_a & BUTTON || port_a & SPACE || port_b & SPACE) {
      BIT_SET(DDR_BUTTON_A, BIT_BUTTON_A);
    } else {

      // AUTOFIRE BUTTON
      if (port_a & AUTOFIRE) {
        autofire_a = 1;

        // neither AUTOFIRE nor FIRE BUTTON
      } else {
        autofire_a = 0;
        BIT_CLEAR(DDR_BUTTON_A, BIT_BUTTON_A);
      }
    }

    if (ext_a) {
      // BUTTON2
      if (port_a & BUTTON2) {
        BIT_SET(DDR_BUTTON2_A, BIT_BUTTON2_A);
        BIT_SET(PORT_BUTTON2_A, BIT_BUTTON2_A);
      } else {

        // AUTOFIRE2 BUTTON
        if (port_a & AUTOFIRE2) {
          autofire2_a = 1;

          // neither AUTOFIRE2 nor FIRE BUTTON2
        } else {
          autofire2_a = 0;
          BIT_CLEAR(DDR_BUTTON2_A, BIT_BUTTON2_A);
          BIT_CLEAR(PORT_BUTTON2_A, BIT_BUTTON2_A);
        }
      }

      // BUTTON3
      if (port_a & BUTTON3) {
        BIT_SET(DDR_BUTTON3_A, BIT_BUTTON3_A);
        BIT_SET(PORT_BUTTON3_A, BIT_BUTTON3_A);
      } else {

        // AUTOFIRE4 BUTTON
        if (port_a & AUTOFIRE3) {
          autofire3_a = 1;

          // neither AUTOFIRE3 nor FIRE BUTTON3
        } else {
          autofire3_a = 0;
          BIT_CLEAR(DDR_BUTTON3_A, BIT_BUTTON3_A);
          BIT_CLEAR(PORT_BUTTON3_A, BIT_BUTTON3_A);
        }
      }
    }

    // ===================================
    //  CONTROL PORT B
    // ===================================

    // UP
    if (port_b & UP) {
      BIT_SET(DDR_JOY_B0, BIT_JOY_B0);
    } else {
      BIT_CLEAR(DDR_JOY_B0, BIT_JOY_B0);
    }

    // DOWN
    if (port_b & DOWN) {
      BIT_SET(DDR_JOY_B1, BIT_JOY_B1);
    } else {
      BIT_CLEAR(DDR_JOY_B1, BIT_JOY_B1);
    }

    // LEFT
    if (port_b & LEFT) {
      BIT_SET(DDR_JOY_B2, BIT_JOY_B2);
    } else {
      BIT_CLEAR(DDR_JOY_B2, BIT_JOY_B2);
    }

    // RIGHT
    if (port_b & RIGHT) {
      BIT_SET(DDR_JOY_B3, BIT_JOY_B3);
    } else {
      BIT_CLEAR(DDR_JOY_B3, BIT_JOY_B3);
    }

    // BUTTON
    if (port_b & BUTTON) {
      BIT_SET(DDR_BUTTON_B, BIT_BUTTON_B);
    } else {

      // AUTOFIRE BUTTON
      if (port_b & AUTOFIRE) {
        autofire_b = 1;

        // neither AUTOFIRE nor FIRE BUTTON
      } else {
        autofire_b = 0;
        BIT_CLEAR(DDR_BUTTON_B, BIT_BUTTON_B);
      }
    }

    if (ext_b) {
      // BUTTON2
      if (port_b & BUTTON2) {
        BIT_SET(DDR_BUTTON2_B, BIT_BUTTON2_B);
        BIT_SET(PORT_BUTTON2_B, BIT_BUTTON2_B);
      } else {

        // AUTOFIRE2 BUTTON
        if (port_b & AUTOFIRE2) {
          autofire2_b = 1;

          // neither AUTOFIRE2 nor FIRE BUTTON2
        } else {
          autofire2_b = 0;
          BIT_CLEAR(DDR_BUTTON2_B, BIT_BUTTON2_B);
          BIT_CLEAR(PORT_BUTTON2_B, BIT_BUTTON2_B);
        }
      }

      // BUTTON3
      if (port_b & BUTTON3) {
        BIT_SET(DDR_BUTTON3_B, BIT_BUTTON3_B);
        BIT_SET(PORT_BUTTON3_B, BIT_BUTTON3_B);
      } else {

        // AUTOFIRE3 BUTTON
        if (port_b & AUTOFIRE3) {
          autofire3_b = 1;

          // neither AUTOFIRE3 nor FIRE BUTTON3
        } else {
          autofire3_b = 0;
          BIT_CLEAR(DDR_BUTTON3_B, BIT_BUTTON3_B);
          BIT_CLEAR(PORT_BUTTON3_B, BIT_BUTTON3_B);
        }
      }
    }
  }
}

void joystick_poll(void) {

  // ===================================
  //  CONTROL PORT A
  // ===================================

  if (autofire_a == 1) {
    // toggle FIRE A
    if (bit_is_set(DDR_BUTTON_A, BIT_BUTTON_A)) {
      BIT_CLEAR(DDR_BUTTON_A, BIT_BUTTON_A);
    } else {
      BIT_SET(DDR_BUTTON_A, BIT_BUTTON_A);
    }
  }

  if (autofire2_a == 1) {
    // toggle FIRE2 A
    if (bit_is_set(DDR_BUTTON2_A, BIT_BUTTON2_A)) {
      BIT_CLEAR(DDR_BUTTON2_A, BIT_BUTTON2_A);
      BIT_CLEAR(PORT_BUTTON2_A, BIT_BUTTON2_A);
    } else {
      BIT_SET(DDR_BUTTON2_A, BIT_BUTTON2_A);
      BIT_SET(PORT_BUTTON2_A, BIT_BUTTON2_A);
    }
  }

  if (autofire3_a == 1) {
    // toggle FIRE3 A
    if (bit_is_set(DDR_BUTTON3_A, BIT_BUTTON3_A)) {
      BIT_CLEAR(DDR_BUTTON3_A, BIT_BUTTON3_A);
      BIT_CLEAR(PORT_BUTTON3_A, BIT_BUTTON3_A);
    } else {
      BIT_SET(DDR_BUTTON3_A, BIT_BUTTON3_A);
      BIT_SET(PORT_BUTTON3_A, BIT_BUTTON3_A);
    }
  }

  // ===================================
  //  CONTROL PORT B
  // ===================================

  if (autofire_b == 1) {
    // toggle FIRE B
    if (bit_is_set(DDR_BUTTON_B, BIT_BUTTON_B)) {
      BIT_CLEAR(DDR_BUTTON_B, BIT_BUTTON_B);
    } else {
      BIT_SET(DDR_BUTTON_B, BIT_BUTTON_B);
    }
  }

  if (autofire2_b == 1) {
    // toggle FIRE2 B
    if (bit_is_set(DDR_BUTTON2_B, BIT_BUTTON2_B)) {
      BIT_CLEAR(DDR_BUTTON2_B, BIT_BUTTON2_B);
      BIT_CLEAR(PORT_BUTTON2_B, BIT_BUTTON2_B);
    } else {
      BIT_SET(DDR_BUTTON2_B, BIT_BUTTON2_B);
      BIT_SET(PORT_BUTTON2_B, BIT_BUTTON2_B);
    }
  }

  if (autofire3_b == 1) {
    // toggle FIRE3 B
    if (bit_is_set(DDR_BUTTON3_B, BIT_BUTTON3_B)) {
      BIT_CLEAR(DDR_BUTTON3_B, BIT_BUTTON3_B);
      BIT_CLEAR(PORT_BUTTON3_B, BIT_BUTTON3_B);
    } else {
      BIT_SET(DDR_BUTTON3_B, BIT_BUTTON3_B);
      BIT_SET(PORT_BUTTON3_B, BIT_BUTTON3_B);
    }
  }
}
