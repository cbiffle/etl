#ifndef ETL_ARMV7M_SCB_FP_H_
#define ETL_ARMV7M_SCB_FP_H_

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

struct ScbFp {
  #define ETL_BFF_DEFINITION_FILE etl/armv7m/scb_fp.reg
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern ScbFp scb_fp;

}  // namespace armv7m
}  // namespace etl

#endif  // ETL_ARMV7M_SCB_FP_H_
