#include "etl/assert.h"

namespace etl {

void assertion_failed(char const *, int, char const *, char const *) {
  while (true);
}

}  // namespace etl
