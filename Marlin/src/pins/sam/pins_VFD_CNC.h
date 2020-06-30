/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

 /**
  * Arduino DUE based CNC router.
  */

#define BOARD_INFO_NAME "VFD_CNC"

#if !defined(__SAM3X8E__) && !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
#error "Oops! Select 'Arduino Due or Mega' in 'Tools > Board.'"
#endif

  /*
    A CNC router based on the DUE can be as simple as this.
    - En/Dir/Step X   Digital pin 22,24,26
    - En/Dir/Step Y   Digital pin 30,32,34
    - En/Dir/Step Z   Digital pin 38,40,42
    - X limiters      Digital pin 31 (X+),33 (X-)
    - Y limiters      Digital pin 37 (Y+),39 (Y-)
    - Z limiters      Digital pin 41 (Z+),43 (Z-)
    - VFD RS485       TX2 (16) / RX2 (17) and Digital pin 25 for RTS

    Wiring RESET to a button to GND is optional, but recommended for routers!
  */

  //
  // Servos
  //
#define SERVO0_PIN                            61  // Analog pin 7, Digital pin 61

//
// Limit Switches
//
#define X_MIN_PIN                             31
#define X_MAX_PIN                             33
#define Y_MIN_PIN                             37
#define Y_MAX_PIN                             39
#define Z_MIN_PIN                             41
#define Z_MAX_PIN                             43

#define Z_MIN_PROBE_PIN                       62  // Analog pin 8, Digital pin 62

//
// Steppers
//
#define X_STEP_PIN                             22
#define X_DIR_PIN                              24
#define X_ENABLE_PIN                           26

#define Y_STEP_PIN                             30
#define Y_DIR_PIN                              32
#define Y_ENABLE_PIN                           34

#define Z_STEP_PIN                             38
#define Z_DIR_PIN                              40
#define Z_ENABLE_PIN                           42

//
// RS-485 Modbus (spindle)
//

#define VFD_MODBUS_RX_PIN                      17  // RS-485 RX pin
#define VFD_MODBUS_TX_PIN                      16  // RS-485 TX pin
#define VFD_MODBUS_RTS_PIN                     25  // RS-485 RTS pin
// #define VFD_MODBUS_RTS_PIN                  23  // RS-485 RTS pin #2 when not using a single pin for both RX and TX RTS

//
// Heaters / Fans
//
#define HEATER_0_PIN                           55  // "Hold":   Analog pin 1, Digital pin 55
#define HEATER_BED_PIN                         57  // "CoolEn": Analog pin 3, Digital pin 57
#define FAN_PIN                                54  // "Abort":  Analog pin 0, Digital pin 54
#undef E0_AUTO_FAN_PIN                         
#define E0_AUTO_FAN_PIN                        56  // "Resume": Analog pin 2, Digital pin 56

//                                             
// Misc. Functions                             
//                                             
#define SDSS                                   52
