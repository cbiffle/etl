#ifndef _ETL_ARMV7M_MPU_H_INCLUDED
#define _ETL_ARMV7M_MPU_H_INCLUDED

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

/*
 * The ARMv7-M Memory Protection Unit, or MPU, is defined by section B3.5
 * of the ARMv7-M ARM.
 *
 * See the comments in the register definition file for more details.
 */
struct Mpu {
  enum class AccessPermissions : Byte {
    p_none_u_none = 0b000,
    p_write_u_none = 0b001,
    p_write_u_read = 0b010,
    p_write_u_write = 0b011,
    p_read_u_none = 0b101,
    p_read_u_read = 0b110,
  };

#define ETL_BFF_DEFINITION_FILE "etl/armv7m/mpu.reg"
#include "etl/biffield/generate.h"
#undef ETL_BFF_DEFINITION_FILE
};

extern Mpu mpu;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_MPU_H_INCLUDED
