#ifndef _ETL_STM32L4XX_TYPES_H_INCLUDED
#define _ETL_STM32L4XX_TYPES_H_INCLUDED

#include "etl/armv7m/types.h"

/*
 * Re-export ARMv7-M types in the stm32l4xx namespace for convenience.
 */

namespace etl {
namespace stm32l4xx {

using etl::armv7m::DoubleWord;
using etl::armv7m::Word;
using etl::armv7m::HalfWord;
using etl::armv7m::Byte;

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_TYPES_H_INCLUDED
