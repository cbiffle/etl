#ifndef _ETL_ARMV7M_REGISTERS_H_INCLUDED
#define _ETL_ARMV7M_REGISTERS_H_INCLUDED

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

/*
 * Reads the contents of the Process Stack Pointer.
 */
inline Word get_psp() {
  Word result;
  asm ("mrs %0, PSP" : "=r"(result));
  return result;
}

/*
 * Sets the contents of the Process Stack Pointer.  Note that this will blow
 * the compiler's mind if done while executing from the PSP.
 */
inline void set_psp(Word value) {
  asm ("msr PSP, %0" :: "r"(value));
}

/*
 * Reads the contents of the Main Stack Pointer.
 */
inline Word get_msp() {
  Word result;
  asm ("mrs %0, MSP" : "=r"(result));
  return result;
}

}  // namespace armv7m
}  // namespace etl 

#endif  // _ETL_ARMV7M_REGISTERS_H_INCLUDED
