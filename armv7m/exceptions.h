#ifndef _ETL_ARMV7M_EXCEPTIONS_H_INCLUDED
#define _ETL_ARMV7M_EXCEPTIONS_H_INCLUDED

namespace etl {
namespace armv7m {

/*
 * This provides number and name for each architectural exception, using the
 * same numbering as the IPSR and vectactive registers.
 */
enum class Exception {
  reset = 1,  // Not present in exceptions.def for convenience

  #define ETL_ARMV7M_EXCEPTION(name) name,
  #define ETL_ARMV7M_EXCEPTION_RESERVED(n) __reserved_ ## n,
  #include "etl/armv7m/exceptions.def"
  #undef ETL_ARMV7M_EXCEPTION
  #undef ETL_ARMV7M_EXCEPTION_RESERVED
};

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_EXCEPTIONS_H_INCLUDED
