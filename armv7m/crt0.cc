#include "etl/armv7m/crt0.h"

#include "etl/armv7m/types.h"
#include "etl/attribute_macros.h"

namespace etl {
namespace armv7m {

typedef void (*InitFnPtr)();

extern "C" {
  /*
   * These symbols are created by the linker script.
   */
  extern Word const _data_init_image_start;
  extern Word _data_start;
  extern Word _data_end;

  extern Word _bss_start;
  extern Word _bss_end;

  extern InitFnPtr _preinit_array_start, _preinit_array_end;
  extern InitFnPtr _init_array_start, _init_array_end;

  extern void _init();

  void init_epilogue();
}

void crt0_init() {
  // Initialize data.
  {
    Word const *src = &_data_init_image_start;
    Word *dest = &_data_start;
    while (dest < &_data_end) {
      *dest++ = *src++;
    }
  }

  // Zero-fill BSS.
  for (Word *dest = &_bss_start; dest < &_bss_end; ) {
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
ETL_USED
ETL_NAKED void init_epilogue() {
  asm volatile ("pop {r4-r11, pc}");
}

}  // namespace armv7m
}  // namespace etl
