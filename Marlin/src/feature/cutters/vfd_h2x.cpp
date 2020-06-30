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

#if ENABLED(VFD_H2X)

int VFD_H2x::get_direction_state()
{
  // Send: 01 03 30 00 00 01

  vfd_send_buffer[0] = address;
  vfd_send_buffer[1] = 0x03; // READ
  vfd_send_buffer[2] = 0x30; // Command group ID
  vfd_send_buffer[3] = 0x00;
  vfd_send_buffer[4] = 0x00; // Message ID
  vfd_send_buffer[5] = 0x01;

  int count = query(6);

  if (count < 6) { return -2; }

  // Receive: 01 03 00 02 00 02
  //                      ----- status
  uint16_t status =
    (uint16_t(vfd_receive_buffer[4]) << 8) |
    uint16_t(vfd_receive_buffer[5]);

  switch (status)
  {
  case 1: return 1;
  case 2: return -1;
  case 3: return 0;
  default:
    // Other values will result in -2, which will result in a command to set it.
    return -2;
  }
}

void VFD_H2x::set_current_direction(int direction)
{
  vfd_send_buffer[0] = address;
  vfd_send_buffer[1] = 0x06; // WRITE
  vfd_send_buffer[2] = 0x20; // Command ID 0x2000
  vfd_send_buffer[3] = 0x00;
  vfd_send_buffer[4] = 0x00;
  vfd_send_buffer[5] = direction < 0 ? 0x02 : (direction == 0 ? 0x06 : 0x01);

  query(6);
}

uint16_t VFD_H2x::get_max_rpm()
{
  static uint16_t max_rpm = 0;

  if (max_rpm == 0)
  {
    // Send: 01 03 B005 0002

    vfd_send_buffer[0] = address;
    vfd_send_buffer[1] = 0x03; // READ
    vfd_send_buffer[2] = 0xB0; // B0.05 = Get RPM
    vfd_send_buffer[3] = 0x05;
    vfd_send_buffer[4] = 0x00; // Read 2 values
    vfd_send_buffer[5] = 0x02;

    int count = query(6);

    //  Recv: 01 03 00 04 5D C0 03 F6 
    //                    -- -- = 24000 (val #1)
    uint16_t rpm =
      (uint16_t(vfd_receive_buffer[4]) << 8) |
      uint16_t(vfd_receive_buffer[5]);

    max_rpm = rpm;

#ifdef VFD_H2X_DEBUG
    SERIAL_ECHOPGM("VFD max rpm is ");
    SERIAL_ECHO(rpm);
    SERIAL_ECHOLNPGM(".\n");
#endif
  }

  return max_rpm;
}

uint16_t VFD_H2x::get_current_rpm()
{
  // Send: 01 03 700C 0002

  vfd_send_buffer[0] = address;
  vfd_send_buffer[1] = 0x03; // READ
  vfd_send_buffer[2] = 0x70; // B0.05 = Get RPM
  vfd_send_buffer[3] = 0x0C;
  vfd_send_buffer[4] = 0x00; // Read 2 values
  vfd_send_buffer[5] = 0x02;

  int count = query(6);

  //  Recv: 01 03 0004 095D 0000
  //                   ---- = 2397 (val #1)
  uint16_t rpm =
    (uint16_t(vfd_receive_buffer[4]) << 8) |
    uint16_t(vfd_receive_buffer[5]);

  return rpm;
}

void VFD_H2x::set_speed(const uint16_t pwr)
{
  // We have to know the max RPM before we can set the current RPM:
  auto max_rpm = get_max_rpm();

  // Speed is in [0..10'000] where 10'000 = 100%.
  // We have to use a 32-bit integer here; typical values are 10k/24k rpm.
  // I've never seen a 400K RPM spindle in my life, and they aren't supported
  // by this modbus protocol anyways... So I guess this is OK.
  uint16_t speed = (uint32_t(rpm) * 10000L) / uint32_t(max_rpm);
  if (speed < 0) { speed = 0; }
  if (speed > 10000) { speed = 10000; }

  vfd_send_buffer[0] = address;
  vfd_send_buffer[1] = 0x06; // WRITE
  vfd_send_buffer[2] = 0x10; // Command ID 0x1000
  vfd_send_buffer[3] = 0x00;
  vfd_send_buffer[4] = uint8_t(speed >> 8); // RPM
  vfd_send_buffer[5] = uint8_t(speed & 0xFF);

  query(6);

#ifdef VFD_H2X_DEBUG
  SERIAL_ECHOPGM("VFD speed set to ");
  SERIAL_ECHO(speed);
  SERIAL_ECHOLNPGM(" (0-10k).\n");
#endif
}

void VFD_H2x::power_sync()
{
  int target_rpm = power;

  // Wait while we're not up to speed. Tolerance is 5% of max rpm.
  // See VFD doc for details.
  auto tolerance = get_max_rpm() / 20;
  while (true)
  {
    auto current_speed = get_current_rpm();

#ifdef VFD_H2X_DEBUG
    SERIAL_ECHOPGM("VFD current speed ");
    SERIAL_ECHO(current_speed);
    SERIAL_ECHOLNPGM(".\r\n");
#endif

    if (current_speed >= (target_rpm - tolerance) &&
      current_speed <= (target_rpm + tolerance))
    {
      return;
    }
    else
    {
      safe_delay(100);
    }
  }
}

VFD_H2x::VFD_H2x(uint8_t modbusAddress) :
  address(modbusAddress)
{}

bool VFD_H2x::init() 
{
#ifdef VFD_H2X_DEBUG
  SERIAL_ECHOPGM("VFD initializing.\r\n");
#endif

  init_rs485();

  // Initialize spindle properties:
  properties.min_speed = 0;
  properties.max_speed = get_max_rpm();
  properties.has_reverse = true;

  // If the spindle is running, we're going to stop it:
  if (get_direction_state() != 0)
  {
    set_current_direction(0);
  }
  activeState.enabled = false;
  activeState.direction_forward = true;

  // Set speed to 0 RPM:
  if (get_current_rpm() != 0)
  {
    set_speed(0);
  }
  activeState.speed = 0;
}

const CutterProperties& VFD_H2x::cutter_info() override
{
  return properties;
}

void VFD_H2x::request_state(const CutterState& newState)
{
  // Apply new state by handling the changes:
  if (!newState.enabled)
  {
    if (activeState.enabled)
    {
      // From enabled -> disabled
      activeState.enabled = false;
      activeState.speed = 0;
      activeState.direction_forward = newState.direction_forward;

      set_current_direction(0);
      set_speed(0);
      power_sync();
    }
    else
    {
      // Was already disabled.
      activeState.direction_forward = newState.direction_forward;
    }
  }
  else
  {
    if (activeState.direction_forward != newState.direction_forward)
    {
      // Change direction! We do this in 2 passes:
      //
      // 1. First go to speed = 0 by disabling the spindle
      activeState.direction_forward = newState.direction_forward;
      activeState.speed = 0;
      set_current_direction(0);
      power_sync();

      // 2. Next, inverse direction:
      activeState.speed = newState.speed;
      activeState.direction_forward = newState.direction_forward;
      set_speed(activeState.speed);
      set_current_direction(activeState.direction_forward ? 1 : -1);
      power_sync();
    }
    else
    {

      bool needsSync = false;

      // update power
      if (activeState.speed != newState.speed)
      {
        activeState.speed = newState.speed;
        set_speed(activeState.speed);
        needsSync = true;
      }

      // new state is enabled.
      if (!activeState.enabled)
      {
        set_current_direction(activeState.direction_forward ? 1 : -1);
        activeState.enabled = true;
        needsSync = true;
      }

      // sync
      if (needsSync) {
        power_sync();
      }
    }
  }
}

void VFD_H2x::get_state(CutterState& speed)
{
  state.direction_forward = activeState.direction_forward;
  state.enabled = activeState.enabled;
  state.speed = activeState.speed;
}

void VFD_H2x::kill()
{
  activeState.speed = 0;
  activeState.enabled = false;
  activeState.direction_forward = true;

  set_current_direction(0);
  set_speed(0);
}

void VFD_H2x::kill_sync()
{
  power_sync();
}

#endif 
