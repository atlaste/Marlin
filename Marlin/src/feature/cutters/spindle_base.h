#pragma once

#include "../spindle_laser_types.h"

struct CutterState
{
  /**
   * Current configured cutter power (in RPM). This is the speed/power of the
   * laser/spindle when the cutter is enabled.
   */
  cutter_power_t rpm;

  /**
    * Current direction:
    * -1 = reversed
    *  0 = stopped (initial)
    *  1 = forward
    */
  int8_t direction;
};

/**
 * Base class for 
 */
class CutterBase
{
public:
  virtual bool init() = 0;

  virtual bool has_reverse() = 0;

  virtual cutter_power_t min_speed() = 0;
  virtual cutter_power_t max_speed() = 0;

  virtual bool request_state(const CutterState& newState) = 0;
  virtual bool get_state(CutterState& speed) = 0;

  virtual void kill() = 0;
};
