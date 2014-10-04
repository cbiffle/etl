/*
 * About This File
 * ---------------
 *
 * This file can be optionally included in applications that don't need to
 * run any code between hardware reset and CRT startup.
 *
 * This covers the vast majority of embedded applications!  You should strongly
 * consider using this unless you are responsible for e.g. initializing the
 * memory controllers that will manage C's data or bss areas.
 *
 * Note that simply running code between C initialization (e.g. zeroing bss)
 * and running C++ constructors can be done with preinit hooks, and does not
 * require manual invocation of the CRT.
 *
 * One potential drawback of this startup sequence: if main returns, we simply
 * enter an infinite loop!  If you would like something more nuanced to occur
 * (for example, immediate software reset), use the manual CRT invocation.
 *
 * Using This File
 * ---------------
 *
 * Ensure that this file is compiled and linked into your application.  If
 * you're using Cobble, add a dependency edge from your binary to the target
 *   //etl/armv7m:implicit_crt0
 *
 * Ensure that your linker script designates the standard ETL ARMv7M reset
 * vector as the entry point.  Using GNU ld, the syntax is:
 *
 *   ENTRY(etl_armv7m_reset_handler)
 *
 * Define a function called 'main' in your application.  The function must
 * have C linkage -- GCC hardcodes this for functions called 'main', so you
 * probably don't have to do anything specific.
 */

#include "etl/attribute_macros.h"
#include "etl/armv7m/crt0.h"
#include "etl/armv7m/exception_table.h"

/*
 * Prototype for the conventional "main" routine.  It returns an int here
 * because GCC is very particular about the signature of functions called
 * "main".  Note that we do not provide command line arguments.
 */
extern "C" int main();

ETL_NAKED  // avoid using stack to save meaningless entry context.
ETL_NORETURN
void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  (void) main();
  while (true);
}
