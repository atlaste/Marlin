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
 * VFD/Modbus spindle support, contributed by Stefan de Bruijn,
 * github:atlaste, july 2020.
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(DEBUG_CUTTER)

void DebugCutter::init()
{
  SERIAL_ECHOPGM("Cutter init.\n");
}

const CutterProperties& DebugCutter::cutter_info()
{
  SERIAL_ECHOPGM("Cutter info requested.\n");
  SERIAL_ECHO(rpm);
  SERIAL_ECHOLNPGM(".\n");

  static CutterProperties info;
  info.min_speed = 1000;
  info.max_speed = 20000;
  info.has_reverse = true;
  return info;
}

void DebugCutter::request_state(const CutterState& newState)
{
  SERIAL_ECHOPGM("Set state to rpm=");
  SERIAL_ECHO(newState.speed);
  if (newState.direction_forward) { SERIAL_ECHO(", forward, "); }
  else { SERIAL_ECHO(", reverse, "); }

  if (newState.enabled) { SERIAL_ECHO(", enabled"); }
  else { SERIAL_ECHO(", disabled"); }
  SERIAL_ECHOLNPGM(".\n");

  state = newState;
}

void DebugCutter::get_state(CutterState& result)
{
  SERIAL_ECHOPGM("Reporting state with RPM -1.\n");;
  result = state;
  result.speed--;
}

void DebugCutter::kill() {
  SERIAL_ECHOPGM("Kill request.\n");
}

void DebugCutter::kill_sync() {
  SERIAL_ECHOPGM("Kill sync.\n");
}

#endif
