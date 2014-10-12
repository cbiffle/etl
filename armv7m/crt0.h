#ifndef _ETL_ARMV7M_CRT0_H_INCLUDED
#define _ETL_ARMV7M_CRT0_H_INCLUDED

namespace etl {
namespace armv7m {

/*
 * Sets up basic invariants expected by C++ programs.  Normally called by the
 * reset handler, early on.
 */
void crt0_init();

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_CRT0_H_INCLUDED
