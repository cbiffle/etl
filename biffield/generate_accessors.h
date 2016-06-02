#ifndef _ETL_BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include etl/biffield/generate_accessors.h directly, \
         use etl/biffield/generate.h
#endif

/*
 * Generate typesafe accessors for registers.
 */


/*******************************************************************************
 * For each register 'foo', generate some combination of:
 *
 *  - 'read_foo()' if the register can be read.
 *  - 'write_foo(foo_value_t)' if the register can be written.
 *
 * Arrays of registers take an additional index argument to both read and write.
 */

#define ETL_BFF_REG_RESERVED(__at, __rn, __n)


#define ETL_BFF_REG_ARRAY_RW(__at, __rn, __n, __fs) \
  ETL_BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  ETL_BFF_REG_ARRAY_WO(__at, __rn, __n, __fs)

#define ETL_BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  __rn ## _value_t read_##__rn(unsigned index) { \
    return __rn ## _value_t(_##__rn[index]); \
  } \
  \
  __rn ## _value_t const volatile * const_addr_of_##__rn(unsigned index) { \
    return reinterpret_cast<__rn ## _value_t const volatile *>( \
        &_##__rn[index]); \
  }


#define ETL_BFF_REG_ARRAY_WO(__at, __rn, __n, __fs) \
  void write_##__rn(unsigned index, __rn##_value_t value) { \
    _##__rn[index] = __at(value); \
  } \
  \
  __rn ## _value_t volatile * addr_of_##__rn(unsigned index) { \
    return reinterpret_cast<__rn ## _value_t volatile *>( \
        &_##__rn[index]); \
  } \
  \
  bool swap_##__rn(unsigned index, __rn##_value_t old_value, \
                                   __rn##_value_t new_value) { \
    __at old_bits = __at(old_value); \
    __at new_bits = __at(new_value); \
    return __sync_bool_compare_and_swap(&_##__rn[index], old_bits, new_bits); \
  } \
  \
  template <typename F> \
  void update_##__rn(unsigned index, F && fn) { \
    bool succeeded = false; \
    do { \
      auto before = this->read_##__rn(index); \
      succeeded = this->swap_##__rn(index, before, fn(before)); \
    } while (!succeeded); \
  }


#define ETL_BFF_REG_RW(__at, __n, __fs) \
  ETL_BFF_REG_RO(__at, __n, __fs) \
  ETL_BFF_REG_WO(__at, __n, __fs)

#define ETL_BFF_REG_RO(__at, __n, __fs) \
  __n ## _value_t read_##__n() { \
    return __n ## _value_t(_##__n); \
  } \
  \
  __n ## _value_t const volatile * const_addr_of_##__n() { \
    return reinterpret_cast<__n ## _value_t const volatile *>( \
        &_##__n); \
  }

#define ETL_BFF_REG_WO(__at, __n, __fs) \
  void write_##__n(__n##_value_t value) { \
    _##__n = __at(value); \
  } \
  \
  void write_##__n(__at value) { \
    _##__n = value; \
  } \
  \
  __n ## _value_t volatile * addr_of_##__n() { \
    return reinterpret_cast<__n ## _value_t volatile *>( \
        &_##__n); \
  } \
  \
  bool swap_##__n(__n##_value_t old_value, \
                  __n##_value_t new_value) { \
    __at old_bits = __at(old_value); \
    __at new_bits = __at(new_value); \
    return __sync_bool_compare_and_swap(&_##__n, old_bits, new_bits); \
  } \
  \
  template <typename F> \
  void update_##__n(F && fn) { \
    bool succeeded = false; \
    do { \
      auto before = this->read_##__n(); \
      succeeded = this->swap_##__n(before, fn(before)); \
    } while (!succeeded); \
  }




/*******************************************************************************
 * Include the file and clean up.
 */

#include ETL_BFF_DEFINITION_FILE

#undef ETL_BFF_REG_RW
#undef ETL_BFF_REG_RO
#undef ETL_BFF_REG_WO

#undef ETL_BFF_REG_ARRAY_RW
#undef ETL_BFF_REG_ARRAY_WO
#undef ETL_BFF_REG_ARRAY_RO

#undef ETL_BFF_REG_RESERVED
