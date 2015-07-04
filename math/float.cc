#include "etl/math/float.h"

#include "etl/prediction.h"

#include "etl/math/inline_fsplit.h"

namespace etl {
namespace math {

SplitFloat fsplit(float value) {
  /*
   * Just instantiate the inline definition as a non-inlined function.
   */
  return fsplit_inl(value);
}

}  // namespace math
}  // namespace etl
