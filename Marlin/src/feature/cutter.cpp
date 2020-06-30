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

/**
 * feature/spindle_laser.cpp
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(CUTTER_FEATURE)

#include "cutter.h"

static CutterState state;
static uint8_t current_instance;

// Define the cutter instances here. For each type of cutter, we have 1 instance:
#if HAS_CUTTER(1)
CUTTER_INSTANCE_TYPE(1) Cutter::cutter1 CUTTER_INSTANCE_BUILD(1);
#endif
#if HAS_CUTTER(2)
CUTTER_INSTANCE_TYPE(2) Cutter::cutter2 CUTTER_INSTANCE_BUILD(2);
#endif
#if HAS_CUTTER(3)
CUTTER_INSTANCE_TYPE(3) Cutter::cutter3 CUTTER_INSTANCE_BUILD(3);
#endif
#if HAS_CUTTER(4)
CUTTER_INSTANCE_TYPE(4) Cutter::cutter4 CUTTER_INSTANCE_BUILD(4);
#endif

static void update_state();

public:
  static void init();

  static void tool_change(int tool);

  static void set_enabled(const bool enable);
  static void set_direction(const bool reverse);
  static void set_ocr_power(const uint8_t value);
  static void set_power(const cutter_power_t value);

  static inline void kill_all()
  {
  }

#endif // HAS_CUTTER
