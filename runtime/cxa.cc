#include "etl/assert.h"

extern "C" void __cxa_pure_virtual();

void __cxa_pure_virtual() {
  ETL_ASSERT(false);
}
