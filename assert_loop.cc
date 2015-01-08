#include "etl/assert.h"

namespace etl {

void assertion_failed(char const * file,
                      int line,
                      char const * function,
                      char const * expression) {
  // Parameters are named to help GDB display them.
  while (true);
}

}  // namespace etl
