#include "etl/armv7m/crt0.h"

#include "etl/armv7m/types.h"
#include "etl/common/attribute_macros.h"

namespace etl {
namespace armv7m {

typedef void (*InitFnPtr)();

extern "C" {
  /*
   * These symbols are created by the linker script.
   */
  extern Word _etext;
  extern Word _data;
  extern Word _edata;
  extern Word _ebss;

  extern InitFnPtr _preinit_array_start, _preinit_array_end;
  extern InitFnPtr _init_array_start, _init_array_end;

  extern void _init();

  void init_epilogue();
}

void crt0_init() {
  // Initialize data.
  for (Word *src = &_etext, *dest = &_data;
       dest < &_edata; ) {
    *dest++ = *src++;
  }

  // Zero-fill BSS.
  for (Word *dest = &_edata; dest < &_ebss; ) {
    *dest++ = 0;
  }

  // Run the funky three-phase init process that GCC seems to expect.
  for (InitFnPtr *f = &_preinit_array_start; f != &_preinit_array_end; ++f) {
    (*f)();
  }

  _init();

  for (InitFnPtr *f = &_init_array_start; f != &_init_array_end; ++f) {
    (*f)();
  }
}

ETL_SECTION(".init_prologue")
ETL_NAKED void _init() {
  asm volatile ("push {r4-r11, lr}");
}

ETL_SECTION(".init_epilogue")
ETL_NAKED void init_epilogue() {
  asm volatile ("pop {r4-r11, pc}");
}

}  // namespace armv7m
}  // namespace etl
