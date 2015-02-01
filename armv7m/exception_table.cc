#include "etl/armv7m/exception_table.h"

#include "etl/attribute_macros.h"

namespace etl {
namespace armv7m {

// Architectural sanity check:
static_assert(sizeof(ExceptionTable) == 16 * sizeof(Word),
              "ExceptionTable size is wrong.");

/*
 * There are a couple of nuances to the exception table definition.
 *
 *  1. We place it in a section called .etl_armv7m_exception_table.  There's
 *     nothing special about that name, except that the linker scripts look for
 *     it and put it in the right place.
 *
 *  2. We make the table const, ensuring that the linker will let us place it
 *     in Flash if the linker script wants it there.
 *
 *  3. Of course, this table doesn't include any vendor-specific interrupt
 *     vectors, which are typically tacked on right at the end using a separate
 *     section in the linker script.
 *
 * While this is marked ETL_USED, this is often not sufficient given how we
 * build the code.  The linker script may need to contain an explicit EXTERN
 * declaration ensuring that this gets linked in.
 */
ETL_SECTION(".etl_armv7m_exception_table")
ETL_USED
ExceptionTable const exception_table = {
  &etl_armv7m_initial_stack_top,
  etl_armv7m_reset_handler,

  #define ETL_ARMV7M_EXCEPTION(name) etl_armv7m_##name##_handler,
  #define ETL_ARMV7M_EXCEPTION_RESERVED(n) nullptr,
  #include "etl/armv7m/exceptions.def"
  #undef ETL_ARMV7M_EXCEPTION
  #undef ETL_ARMV7M_EXCEPTION_RESERVED
};

}  // namespace armv7m
}  // namespace etl
