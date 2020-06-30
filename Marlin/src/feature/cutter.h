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
 * feature/spindle_laser.h
 * Support for Laser Power or Spindle Power & Direction
 *
 * To make things easy, CNC machines can support multiple lasers and spindles.
 * A common configuration is where the laser is mounted next to the spindle.
 * But other examples also exist, like front/back spindles mounted on a PCB router.
 * Even worse, spindles are controlled in a large variety of ways, with examples
 * like digital pins (optionally with relays or optocouplers), RS485 and PWM.
 *
 * To support all these different configurations, this proxy class was added
 * that basically forwards the actual commands to the spindle_base class in
 * the spindles folder. The spindle_base class in the spindles folder has
 * several derived classes, each with a different spindle or laser implementation.
 *
 * For simplicity, I currently implemented only one instance of a cutter. However,
 * it is relatively easy to change this implementation to support multiple cutter
 * instances.
 *
 * This class has all the code required for the LCDs, the planner and the
 * g-codes. Each physical spindle configured in the configuration_adv.h file is
 * converted to an instance here, which is instantiated only once. During the runtime,
 * tool change commands can be used to pick the right spindle/laser.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(CUTTER_FEATURE)

#include "cutter_types.h"

#if ENABLED(LASER_POWER_INLINE)
  #include "../module/planner.h"
#endif

#if HAS_CUTTER_TYPE(PWM_LASER) || HAS_CUTTER_TYPE(PWM_SPINDLE)
  #include "cutters/pwm_laser.h"

  extern SpindleLaser cutter;
#else

#include "cutters/cutter_base.h"

// Include all different types of cutters here:
#if HAS_CUTTER_TYPE(VFD_H2X)
  #include "cutters/vfd_h2x.h"
  #define SPINDLE_CLASS VFD_H2x
#endif

#define PCT_TO_PWM(X) ((X) * 255 / 100)

class Cutter
{
  // If the CNC machine only has 1 cutter, this implementation exploits this fact and calls member
  // functions directly. This improves performance a bit, and results in less code. Modern compilers
  // will compile all the plumbing away, which means that this will basically decay to the old
  // implementation that only supported one cutter.

  static SPINDLE_CLASS current;
  static CutterState state;
  static CutterProperties currentProperties;

  static inline void update_state()
  {
    current.request_state(&state);
  }

  static inline void update_state()
  {
    current.request_state(&state);
  }

public:
  /**
   * Part 1 of the cutter code: control code, handling of g-codes, etc.
   */

  static inline void init()
  {
    current.init();
    currentProperties = current.cutter_info();
  }

  static inline void set_enabled(const bool enable)
  {
    state.enabled = enable;
    update_state();
  }

  static inline void set_direction(const bool reverse)
  {
    state.direction = reverse ? -1 : 1;
    update_state();
  }

  static inline void set_power(const cutter_power_t value)
  {
    state.speed = value;
    update_state();
  }

  static inline void kill_all()
  {
    // KILL first, then SYNC until it's done! We want to stop everything as
    // quickly as possible if someone asks for a kill for whatever reasons.
    current.kill();
    current.kill_sync();
  }

  /**
   * Part 2 of the cutter code: translation of information from the cutter to the UI.
   */

  static inline bool enabled()
  {
    return state.enabled();
  }

  static inline bool isReady()
  {
    return current.isReady();
  }

#if HAS_LCD_MENU
  cutter_power_t menuPower; // Power set via LCD menu in PWM, PERCENT, or RPM

  static inline bool mpower_min() { return currentProperties.min_speed; }
  static inline bool mpower_max() { return currentProperties.max_speed; }

  static inline void enable_with_dir(const bool reverse) {
    isReady = true;

    // TODO FIXME: should we translate PWM/Percent to RPM?
    
    set_power(menuPower);
    set_direction(reverse);
    set_enabled(true);
  }

  FORCE_INLINE static void enable_forward() { enable_with_dir(false); }
  FORCE_INLINE static void enable_reverse() { enable_with_dir(true); }

  static inline void update_from_mpower() {
    state.speed = menuPower;
    if (isReady)
    {
      update_state();
    }
  }

  static inline void apply_block_power(const uint8_t inpow)
  {
    return current.apply_power_immediately(inpow);
  }

#endif
};

extern Cutter cutter;

#endif

#endif
