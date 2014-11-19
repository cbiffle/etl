#ifndef _ETL_ARMV7M_SYS_TICK_H_INCLUDED
#define _ETL_ARMV7M_SYS_TICK_H_INCLUDED

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

struct SysTick {
  #define ETL_BFF_DEFINITION_FILE "etl/armv7m/sys_tick.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern SysTick sys_tick;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_SYS_TICK_H_INCLUDED
