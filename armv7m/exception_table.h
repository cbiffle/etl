#ifndef _ETL_ARMV7M_EXCEPTION_TABLE_H_INCLUDED
#define _ETL_ARMV7M_EXCEPTION_TABLE_H_INCLUDED

#include "etl/armv7m/types.h"

/*
 * ARMv7M Exceptions
 *
 * The architecture defines 16 exceptions, some of which are reserved.  These
 * signal processor-internal conditions, as opposed to interrupts, which are
 * defined by the SoC and signal external conditions.
 *
 * Exception names and ordering are defined in the exceptions.def x-macro
 * file.
 *
 * For ease of interfacing with assembly, exception handlers are defined as
 * C-style unmangled symbols in the top-level namespace.
 *
 * We don't provide any default exception linkage.  The application is
 * responsible for mapping *all* exceptions to handler functions, which may be
 * trivial.  (Note that this is explicitly different from CMSIS implementations,
 * which usually provide weak definitions.)
 */

/*
 * Handler hooks are named `etl_armv7m_foo_handler`, where `foo` is the name of
 * the exception as defined in exceptions.def.  e.g. etl_armv7m_reset_handler.
 * They're defined with C linkage to allow them to be easily defined outside of
 * this namespace, by the application.
 */

extern "C" {
  void etl_armv7m_reset_handler();

  #define ETL_ARMV7M_EXCEPTION(name) void etl_armv7m_ ## name ## _handler();
  #define ETL_ARMV7M_EXCEPTION_RESERVED(n) /* nothing */
  #include "etl/armv7m/exceptions.def"
  #undef ETL_ARMV7M_EXCEPTION
  #undef ETL_ARMV7M_EXCEPTION_RESERVED

  /*
   * The exception table also contains the initial stack pointer, to be used
   * when calling the reset handler.  The application must define this, either
   * in code or the linker script, to be the word just above the initial stack.
   *
   * It's const because, typically, it's an address just above the top of RAM --
   * so you'd best not try to write it.
   */

  extern etl::armv7m::Word const etl_armv7m_initial_stack_top;
}

namespace etl {
namespace armv7m {

// ARMv7-M ISR entry points look like this:
typedef void (*ExceptionHandler)(void);

struct ExceptionTable {
  Word const *initial_stack_top;
  ExceptionHandler reset_handler;

  #define ETL_ARMV7M_EXCEPTION(name) ExceptionHandler name ## _handler;
  #define ETL_ARMV7M_EXCEPTION_RESERVED(n) ExceptionHandler __reserved ## n;
  #include "etl/armv7m/exceptions.def"
  #undef ETL_ARMV7M_EXCEPTION
  #undef ETL_ARMV7M_EXCEPTION_RESERVED
};

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_EXCEPTION_TABLE_H_INCLUDED
