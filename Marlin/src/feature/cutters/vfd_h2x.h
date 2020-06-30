#pragma once

#include "cutter_base.h"
#include "vfd_modbus.h"

class VFD_H2x : public VFDModbus
{
  CutterState activeState;
  CutterProperties properties;

  uint8_t address;

  int get_direction_state();
  void set_current_direction(int direction);

  uint16_t get_max_rpm();
  uint16_t get_current_rpm();
  void set_speed(const uint16_t rpm);

  void power_sync();

public:
  VFD_H2x(uint8_t modbusAddress);

  void init() override;

  const CutterProperties& cutter_info() override;

  void request_state(const CutterState& newState) override;

  void get_state(CutterState& state) override;

  void kill() override;

  void kill_sync() override;
};
