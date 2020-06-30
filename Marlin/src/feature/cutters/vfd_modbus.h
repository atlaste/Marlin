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
#pragma once

#include "cutter_base.h"

class VFDModbus : public CutterBase
{
  // 50 bytes should be plenty for whatever the VFD can throw at us.
  static const int RECEIVE_BUFFER_SIZE = 50;
  static const int SEND_BUFFER_SIZE = 20;

  // Some buffers:
  uint8_t vfd_receive_buffer[RECEIVE_BUFFER_SIZE];
  uint8_t vfd_send_buffer[SEND_BUFFER_SIZE];

#ifdef VFD_RS485_DEBUG
  static inline void debug_rs485(bool sending, uint8_t* ptr, int size)
  {
    if (sending) {
      SERIAL_ECHOPGM("Send: ");
    }
    else {
      SERIAL_ECHOPGM("Recv: ");
    }

    char tmp[4];
    for (int i = 0; i < size; ++i) {
      uint8_t current = ptr[i];

      tmp[0] = (char)("0123456789ABCDEF"[current >> 4]);
      tmp[1] = (char)("0123456789ABCDEF"[current & 0xF]);
      tmp[2] = ' ';
      tmp[3] = '\0';
      SERIAL_ECHOPGM(tmp);
    }
    SERIAL_ECHOPGM("\r\n");
  }
#endif

  // Helper functions with details:
  static uint16_t get_crc_value(uint8_t* data_value, uint8_t length);

  static void crc_check_value(uint8_t* data_value, uint8_t length);

  static bool validate_crc_value(uint8_t* data_value, uint8_t length);
  
  static int receive_data_detail(uint8_t* buffer);

  static void send_data_detail(uint8_t* buffer, int length);

protected:
  static void init_rs485();

  int query(int send_length);
};
