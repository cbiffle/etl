#ifndef _ETL_ARMV7M_SCB_H_INCLUDED
#define _ETL_ARMV7M_SCB_H_INCLUDED

#include "etl/armv7m/exceptions.h"
#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

struct Scb {
  enum class CpAccess {
    none = 0b00,
    privileged = 0b01,
    full = 0b11,
  };

  #define ETL_BFF_DEFINITION_FILE "etl/armv7m/scb.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE

 public:
  /*
   * At reset, the processor sets MemManage, Bus, and Usage faults to the Hard
   * Fault handler, which complicates diagnostics.  This fixes that.
   */
  void enable_faults();

  /*
   * Sets the priority of a configurable-priority exception.
   */
  void set_exception_priority(Exception, Byte);
};

extern Scb scb;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_SCB_H_INCLUDED
