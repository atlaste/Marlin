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

#include "../../inc/MarlinConfig.h"

#if ENABLED(VFD_MODBUS)

#if defined(__SAM3X8E__) || defined(__SAMD21G18A__)
#if (VFD_MODBUS_RX_PIN == 19 && VFD_MODBUS_TX_PIN == 18)
#include <HardwareSerial.h>
#define VFDSerial Serial1
#elif (VFD_MODBUS_RX_PIN == 17 && VFD_MODBUS_TX_PIN == 16)
#include <HardwareSerial.h>
#define VFDSerial Serial2
#elif (VFD_MODBUS_RX_PIN == 15 && VFD_MODBUS_TX_PIN == 14)
#include <HardwareSerial.h>
#define VFDSerial Serial3
#endif
#endif

#if !defined(VFDSerial)
#ifdef VFD_MODBUS_PARITY
#error "VFD parity is not supported when using software serial RX/TX ports";
#endif

#include "SoftwareSerial.h"
SoftwareSerial VFDSerial(VFD_MODBUS_RX_PIN, VFD_MODBUS_TX_PIN); // RX, TX
#define USE_SOFTWARE_SERIAL
#endif

#ifndef VFD_MODBUS_BAUD
#error "You should define VFD_MODBUS_BAUD according to the VFD settings"
#endif

#include "vfd_modbus.h"

 // Helper functions with details:
uint16_t VFDModbus::get_crc_value(uint8_t* data_value, uint8_t length)
{
  // Implementation from the manual... not efficient, but it works:

  uint16_t crc_value = 0xffff;
  while (length--)
  {
    crc_value ^= *data_value++;
    for (int i = 0; i < 8; ++i)
    {
      if (crc_value & 0x0001)
      {
        crc_value = (crc_value >> 1) ^ 0xa001u;
      }
      else
      {
        crc_value = crc_value >> 1;
      }
    }
  }

  return crc_value;
}

void VFDModbus::crc_check_value(uint8_t* data_value, uint8_t length)
{
  uint16_t crc_value = get_crc_value(data_value, length);

  // Low byte first, then the high byte
  data_value[length + 0] = uint8_t(crc_value & 0xFF);
  data_value[length + 1] = uint8_t(crc_value >> 8);
}

bool VFDModbus::validate_crc_value(uint8_t* data_value, uint8_t length)
{
  uint16_t crc_value = get_crc_value(data_value, length);

  return
    data_value[length + 0] == uint8_t(crc_value & 0xff) &&
    data_value[length + 1] == uint8_t(crc_value >> 8);
}

int VFDModbus::receive_data_detail(uint8_t* buffer)
{
  const auto waitTimePerChar = 1000000 / VFD_MODBUS_BAUD;
  const auto timeForEndPacket = 4 * waitTimePerChar;
  const auto maxWaitIterations = VFD_MODBUS_BAUD; // 0.1 second

  int index = 0;
  int n;
  for (int i = 0; i < maxWaitIterations && (n = VFDSerial.available()) == 0; ++i)
  {
    delayMicroseconds(waitTimePerChar);
  }

  if (n == 0)
  {
    // No data received within the allotted time:
    SERIAL_ECHOPGM("VFD/RS485 error: no response from VFD/RS485 within the allotted time.\r\n");
    return 0;
  }
  else
  {
    for (;;)
    {
      if (n + index > RECEIVE_BUFFER_SIZE)
      {
        // Read the remainder to flush the buffer:
        while ((n = VFDSerial.available()) != 0)
        {
          VFDSerial.read();
        }

        SERIAL_ECHOPGM("VFD/RS485 error: packet that was received from VFD/RS485 was too long and is ignored.\r\n");
        return 0;
      }
      else
      {
        // We have data. Read it to our buffer:
        VFDSerial.readBytes(vfd_receive_buffer + index, n);
        index += n;

        // Check if new data is available:
        n = VFDSerial.available();
        if (n == 0)
        {
          // Not yet, but we might just be polling too fast. The spec sais that we
          // need to wait the time of 4 characters:
          //
          // delayMicroseconds(timeForEndPacket);
          //
          // ... but that doesn't work. Instead, it can apparently take up to 20 ms
          // before the thing does its job and give us the right answer. Here goes.

          for (int i = 0; i < 20 && n == 0; ++i)
          {
            safe_delay(1);
            n = VFDSerial.available();
          }

          if (n == 0)
          {
            // Still no data, this means we're done.

#ifdef VFD_MODBUS_DEBUG
            debug_rs485(false, vfd_receive_buffer, index);
#endif

            return index;
          }
        }
        // Otherwise there's new data (n>0) which we need to process
      }
    }
  }
}

void VFDModbus::send_data_detail(uint8_t* buffer, int length)
{
  // send index
  crc_check_value(buffer, length);
  VFDSerial.flush();

  // We assume half-duplex communication:
  digitalWrite(VFD_MODBUS_RTS_PIN, HIGH);
#ifdef VFD_MODBUS_RTS_PIN2
  digitalWrite(VFD_MODBUS_RTS_PIN2, HIGH);
#endif

  const auto waitTimePerChar = 1000000 / VFD_MODBUS_BAUD;
  delayMicroseconds(waitTimePerChar * 4);

  // Before we set the MAX485 chip RTS to low, we have to flush:
  VFDSerial.write(buffer, length + 2);
  VFDSerial.flush();

  // And immediately set it back to low, to ensure that
  // incoming data gets processed by the MAX485.
  digitalWrite(VFD_MODBUS_RTS_PIN, LOW);
#ifdef VFD_RTS_PIN2
  digitalWrite(VFD_MODBUS_RTS_PIN2, LOW);
#endif

#ifdef VFD_MODBUS_DEBUG
  debug_rs485(true, buffer, length + 2);
#endif
}

void VFDModbus::init_rs485()
{
#ifdef VFD_MODBUS_DEBUG
  SERIAL_ECHOPGM("VFD modbus initializing.\r\n");
#endif

  init_pins();
  safe_delay(10); // give it some time to start up...
}

int VFDModbus::query(int send_length)
{
  while (true)
  {
    send_data_detail(vfd_send_buffer, send_length);

    // Each iteration of receive_data_detail will take at most 0.1 seconds. 50 iterations = ~5 seconds
    // Under normal circumstances the communication should be *much* faster though, to be exact: it would 
    // take `((number of characters + 8) * 9) / baud` seconds. Typical is 8 characters, which is 0.0075s.
    //
    // All measures here are mostly there for when things are wrong, like when your VFD is unresponsive.
    // You don't want a broken piece, so better to just wait until the problem is resolved...
    for (int i = 0; i < 50; ++i)
    {
      int n = receive_data_detail();

      // n = 0 -> No response. Give it some time:
      if (n == 0)
      {
        safe_delay(10);
        send_data_detail(vfd_send_buffer, send_length);
      }
      else if (n > 0)
      {
        // If we have a response, we have to check the CRC16 checksum:
        if (!validate_crc_value(vfd_receive_buffer, n - 2))
        {
          // No luck, we have to try again, because the checksum failed:
          SERIAL_ECHOPGM("VFD/RS485 error: communication checksum failed, have to retry.\r\n");

          send_data_detail(vfd_send_buffer, send_length);
        }
        else
        {
          // CRC validates OK:
          // 
          // We expect: 01.    03.   0002   0002
          //            [addr] [cmd] [len]  [data]
          //
          // We ignore len; it can vary with packages and we already checked CRC.
          if (vfd_receive_buffer[0] == vfd_send_buffer[0] &&
              vfd_receive_buffer[1] == vfd_send_buffer[1])
          {
            // error if it doesn't add up
            return n;
          }
          else if (vfd_receive_buffer[1] != vfd_send_buffer[1])
          {
            // error can be a colliding packet
            // 
            // try again:
            SERIAL_ECHOPGM("VFD/RS485 error: response originated from other modbus address\r\n");

            send_data_detail(vfd_send_buffer, send_length);
          }
        }
      }
    }

    SERIAL_ECHOPGM("VFD/RS485 error: general error communicating with VFD/RS485. Check baud rate and parity settings!\r\n");
  }
}

#endif
