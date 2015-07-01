#ifndef _ETL_ARMV7M_ACTLR_H_INCLUDED
#define _ETL_ARMV7M_ACTLR_H_INCLUDED

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

/*
 * The Auxiliary Control Register, which is sort of lurking off on its own
 * and is implementation-defined.
 */
extern Word volatile actlr;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_ACTLR_H_INCLUDED
