#pragma once

#include "../cutter_types.h"

struct CutterState
{
  CutterState() :
    speed(0),
    direction_forward(true),
    enabled(false)
  {}

  /**
   * Current configured cutter power (in RPM). This is the speed/power of the
   * laser/spindle when the cutter is enabled.
   */
  cutter_power_t speed;

  /**
    * Current direction is forward.
    */
  bool direction_forward;

  /**
   * Enabled. When disabled, direction and rpm might still have a value.
   */
  bool enabled;
};

struct CutterProperties
{
  /**
   * Minimum RPM:
   */
  cutter_power_t min_speed;

  /**
   * Maximum RPM:
   */
  cutter_power_t max_speed;

  /**
   * Has reverse:
   */
  bool has_reverse;

  /**
   * Supports ocr (0-255) speed:
   */
  bool supports_ocr;
};

/**
 * Base class for all cutter devices. This class merely shows the API, but doesn't actually
 * provide any real functionality. As such, it also doesn't expose any members.
 *
 * Because we don't want to do too many v-table calls during normal operations, most of the
 * calls are 
 *
 * TODO: We could static_assert the interface of a derived type by using SFINAE.
 */
class CutterBase
{
public:
  virtual void init() = 0;
  
  virtual const CutterProperties& cutter_info() = 0;
  
  virtual void request_state(const CutterState& newState) = 0;
  virtual void get_state(CutterState& result) = 0;
  
  virtual void kill() = 0;
  virtual void kill_sync() = 0;

  // ???
  virtual bool isReady() = 0;
};
