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
 * This class has all the code required for the LCDs, the planner and the
 * g-codes. Each physical spindle configured in the configuration_adv.h file is
 * converted to an instance here, which is instantiated only once. During the runtime,
 * tool change commands can be used to pick the right spindle/laser.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(CUTTER_FEATURE)

#include "cutter_types.h"
#include "cutters/cutter_base.h"

// Include all different types of cutters here:
#if HAS_CUTTER_TYPE(PWM_SPINDLE)
  #include "cutters/pwm_spindle.h"
#endif

#if HAS_CUTTER_TYPE(PWM_LASER)
  #include "cutters/pwm_laser.h"
#endif

#if HAS_CUTTER_TYPE(VFD_H2X)
  #include "cutters/vfd_h2x.h"
#endif

#if ENABLED(LASER_POWER_INLINE)
  #include "../module/planner.h"
#endif

#define PCT_TO_PWM(X) ((X) * 255 / 100)

class Cutter
{
  // If the CNC machine only has 1 cutter, this implementation exploits this fact and calls member
  // functions directly. This improves performance a bit, and results in less code. Modern compilers
  // will compile all the plumbing away, which means that this will basically decay to the old
  // implementation that only supported one cutter.

#if HAS_SINGLE_CUTTER
  static CUTTER_INSTANCE_TYPE(1) current;
  static CutterState state;
  static CutterProperties currentProperties;

  static inline void update_state()
  {
    current.request_state(&state);
  }
#else
  static CutterBase* current;
  static CutterState state;
  static CutterProperties currentProperties;

  // Define the cutter instances here. For each type of cutter, we have 1 instance:
  #if HAS_CUTTER_INST(1)
    static CUTTER_INSTANCE_TYPE(1) cutter1;
  #endif
  #if HAS_CUTTER_INST(2)
    static CUTTER_INSTANCE_TYPE(2) cutter2;
  #endif
  #if HAS_CUTTER_INST(3)
    static CUTTER_INSTANCE_TYPE(3) cutter3;
  #endif
  #if HAS_CUTTER_INST(4)
    static CUTTER_INSTANCE_TYPE(4) cutter4;
  #endif

  static inline CutterBase* get_cutter_instance(uint8_t instance)
  {
    switch (instance)
    {
    case 2:
#if HAS_CUTTER_INST(2)
      return &cutter2;
#endif
#if HAS_CUTTER_INST(3)
      return &cutter3;
#endif
#if HAS_CUTTER_INST(4)
      return &cutter4;
#endif

    default:
#if HAS_CUTTER_INST(1)
      return &cutter1;
#else
#error "Cutter 1 has to be defined for spindles/lasers to work.";
#endif
    }
  }

  static inline void update_state()
  {
    current->request_state(&state);
  }
#endif

public:
  /**
   * Part 1 of the cutter code: control code, handling of g-codes, etc.
   */

  static inline void init()
  {
#if HAS_SINGLE_CUTTER
    current.init();
    currentProperties = current.cutter_info();
#elif HAS_CUTTER_INST(1)
    cutter1.init();
    current = &cutter1;
    currentProperties = cutter1.cutter_info();
#else
  #error "Cutter 1 has to be defined for spindles/lasers to work.";
#endif

#if HAS_CUTTER_INST(2)
    cutter2.init();
#endif

#if HAS_CUTTER_INST(3)
    cutter3.init();
#endif

#if HAS_CUTTER_INST(4)
    cutter4.init();
#endif
  }

#if HAS_SINGLE_CUTTER
  static inline void tool_change(const uint8_t ) {}
#else
  static inline void tool_change(const uint8_t tool)
  {
    // TODO FIXME: Fix mapping of tool <> instance!? Is this implementation correct?

    if (current != nullptr) {
      current->kill();
    }
    current = get_cutter_instance(tool);
    currentProperties = current->cutter_info();
  }
#endif

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
  static void set_ocr_power(const uint8_t value)
  {
    // While it might seem like a good solution to set the OCR power directly
    // on the PWM class, it is actually wrong. If a direction change, or enable
    // comes by, the spindle might be disabled and re-enabled, which gets rid of
    // the old state.
    //
    // Easiest solution is to simply translate everything into RPM. And be careful
    // with rounding:
    //
    // current->min_speed() translates to value == 0
    // current->max_speed() translates to value == 255

    auto speed = uint16_t(currentProperties.min_speed +
                          int32_t((currentProperties.max_speed - currentProperties.min_speed) * value) / 255);
    state.speed = speed;
    update_state();
  }

  static inline void set_power(const cutter_power_t value)
  {
    state.speed = value;
    update_state();
  }

  static inline void kill_all()
  {
#if HAS_SINGLE_CUTTER
    cutter1.kill();
#elif HAS_CUTTER_INST(1)
    cutter1.kill();
#endif

#if HAS_CUTTER_INST(2)
    cutter2.kill();
#endif

#if HAS_CUTTER_INST(3)
    cutter3.kill();
#endif

#if HAS_CUTTER_INST(4)
    cutter4.kill();
#endif
  }

  /**
   * Part 2 of the cutter code: translation of information from the cutter to the UI.
   */
};

extern Cutter cutter;

#endif
