#ifndef _ETL_DESTROY_H_INCLUDED
#define _ETL_DESTROY_H_INCLUDED

namespace etl {

/*
 * Invokes an object's destructor.  This is sugar for the pattern
 *    t.~T();
 *
 * Note that the compiler does not realize the object's life has ended, so it's
 * easy to accidentally use a destroyed object.  Be careful.
 */
template <typename T>
void destroy(T & object) {
  object.~T();
}

}  // namespace etl

#endif  // _ETL_DESTROY_H_INCLUDED
