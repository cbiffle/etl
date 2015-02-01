#ifndef _ETL_ARMV7M_EXCEPTION_FRAME_H_INCLUDED
#define _ETL_ARMV7M_EXCEPTION_FRAME_H_INCLUDED

/*
 * Describes the structure pushed onto the active stack when the processor
 * begins servicing an interrupt.
 */

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

struct ExceptionFrame {
  Word r0, r1, r2, r3;
  Word r12;
  Word r14, r15;
  Word psr;
};

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_EXCEPTION_FRAME_H_INCLUDED
