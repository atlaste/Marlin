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
 * Note that we don't actually do any v-table calls. This isn't really about performance, but
 * more about that I didn't want to change everything right away. The reason it's set up like
 * this, is to support multiple cutters in the future. Think f.ex about having a laser next
 * to your spindle (which is quite common).
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
};
