#pragma once

#include "cutter_base.h"

/**
 * Debug class for cutters. The debug cutter itself is not a cutter, but implements
 * the interface and dumps the requested commands back to the serial output. This
 * enables for easy testing of the cutter tool changes, requested speed changes, etc.
 */
class DebugCutter : public CutterBase
{
  CutterState state;

public:
  void init() override;

  const CutterProperties& cutter_info() override;

  void request_state(const CutterState& newState) override;
  void get_state(CutterState& result) override;

  void kill() override;
  void kill_sync() override;
};
