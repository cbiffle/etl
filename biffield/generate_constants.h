#ifndef _ETL_BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include etl/biffield/generate_constants.h directly, \
         use etl/biffield/generate.h
#endif

/*
 * This file produces metadata structs about the structure of registers and
 * bitfields, so that code can (for example) find the bit index and size of
 * a given field using only compile-time constants.
 */


/*******************************************************************************
 * For a register 'foo', generate a non-constructible type 'foo_meta' to serve
 * as a namespace for static constants and typedefs describing the shape of the
 * bitfields:
 *
 *  - 'foo_meta::access_type' gives the type used to load/store the register
 *    to/from memory.
 *
 *  - 'foo_meta::bar' contains information about field 'bar'.
 */

#define ETL_BFF_REG_RESERVED(__at, __rn, __n)


#define ETL_BFF_REG_ARRAY_RW(__at, __rn, __n, __fs) \
  _ETL_BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)

#define ETL_BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  _ETL_BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)

#define ETL_BFF_REG_ARRAY_WO(__at, __rn, __n, __fs) \
  _ETL_BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)

#define _ETL_BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs) \
  _ETL_BFF_REG_ANY(__at, __rn, __fs)


#define ETL_BFF_REG_RW(__at, __rn, __fs) _ETL_BFF_REG_ANY(__at, __rn, __fs)
#define ETL_BFF_REG_RO(__at, __rn, __fs) _ETL_BFF_REG_ANY(__at, __rn, __fs)
#define ETL_BFF_REG_WO(__at, __rn, __fs) _ETL_BFF_REG_ANY(__at, __rn, __fs)

#define _ETL_BFF_REG_ANY(__at, __rn, __fs) \
  struct __rn ## _meta { \
    __rn ## _meta() = delete; \
    __rn ## _meta(__rn ## _meta const &) = delete; \
    \
    typedef __at access_type; \
    __fs \
  };


/*******************************************************************************
 * For each field 'bar' in register 'foo', generate a non-constructible type
 * 'foo_meta::bar' to serve as a namespace for constants and typedefs describing
 * the shape of the field:
 *
 *  - 'foo_meta::bar::low_bit' and 'foo_meta::bar::high_bit' give the low and
 *    high zero-based bit numbers for the field.
 *  - 'foo_meta::bar::bit_count' gives the number of bits in the field.
 *  - 'foo_meta::bar::low_mask' is a value of the register's access type where
 *    the low-order 'bit_count' bits are set, and the rest are clear.
 *  - 'foo_meta::bar::in_place_mask' is similar, but the bits are shifted into
 *    the field's position.
 */

// Normal and enum fields have common metadata generation.
#define ETL_BFF_FIELD(__bf, __ft, __fn) _ETL_BFF_FIELD_COMMON(__bf, __ft, __fn)
#define ETL_BFF_FIELD_E(__bf, __ft, __fn, __es) _ETL_BFF_FIELD_COMMON(__bf, __ft, __fn)

#define _ETL_BFF_FIELD_COMMON(__bf, __ft, __fn) \
  struct __fn { \
    __fn() = delete; \
    __fn(__fn const &) = delete; \
    \
    static constexpr unsigned low_bit =  0 ? __bf; \
    static constexpr unsigned high_bit = 1 ? __bf; \
    static constexpr unsigned bit_count = high_bit - low_bit + 1; \
    static constexpr unsigned shift_amt = \
        bit_count % (sizeof(access_type) * 8); \
    static constexpr access_type low_mask = \
        bit_count == sizeof(access_type) * 8 \
            ? access_type(0) - 1 \
            : (access_type(1) << shift_amt) - 1; \
    static constexpr access_type in_place_mask = low_mask << low_bit; \
  };

// Field arrays are handled differently.
#define ETL_BFF_FIELD_ARRAY(__bf, __n, __ft, __fn) \
  struct __fn { \
    __fn() = delete; \
    __fn(__fn const &) = delete; \
    \
    static constexpr unsigned low_bit =  0 ? __bf; \
    static constexpr unsigned high_bit = 1 ? __bf; \
    static constexpr unsigned bit_count = high_bit - low_bit + 1; \
    static constexpr unsigned shift_amt = \
        bit_count % (sizeof(access_type) * 8); \
    static constexpr access_type low_mask = \
        bit_count == sizeof(access_type) * 8 \
            ? access_type(0) - 1 \
            : (access_type(1) << shift_amt) - 1; \
    static constexpr access_type in_place_mask = low_mask << low_bit; \
    \
    static constexpr unsigned bits_per_element = __n; \
    static constexpr unsigned element_count = bit_count / bits_per_element; \
    static constexpr access_type low_element_mask = \
        (access_type(1) << bits_per_element) - 1; \
    \
    static constexpr unsigned low_bit_of(unsigned idx) { \
      return low_bit + idx * bits_per_element; \
    } \
    \
    static constexpr access_type in_place_mask_of(unsigned idx) { \
      return low_element_mask << low_bit_of(idx); \
    } \
  };



/*******************************************************************************
 * Include the file and clean up.
 */

#include ETL_BFF_DEFINITION_FILE

#undef _ETL_BFF_FIELD_COMMON
#undef ETL_BFF_FIELD
#undef ETL_BFF_FIELD_E
#undef ETL_BFF_FIELD_ARRAY
#undef ETL_BFF_WAZ

#undef ETL_BFF_REG_RW
#undef ETL_BFF_REG_RO
#undef ETL_BFF_REG_WO
#undef _ETL_BFF_REG_ANY

#undef ETL_BFF_REG_ARRAY_RW
#undef ETL_BFF_REG_ARRAY_WO
#undef ETL_BFF_REG_ARRAY_RO
#undef _ETL_BFF_REG_ARRAY_ANY

#undef ETL_BFF_REG_RESERVED
