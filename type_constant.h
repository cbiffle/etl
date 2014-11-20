#ifndef _ETL_TYPE_CONSTANT_H_INCLUDED
#define _ETL_TYPE_CONSTANT_H_INCLUDED

namespace etl {

/*
 * TypeConstant creates a type of trivial values associated with a given type.
 * This can be used to pass a placeholder for a type in a value context,
 * without any overhead that might be associated with creating an actual
 * instance of the type (or even a pointer).
 *
 * A TypeConstant<T> is also a type-metafunction returning T, for use with
 * things like std::conditional.
 *
 * For compatibility, this exposes both std-style and etl-style names.
 */
template <typename T>
struct TypeConstant {
  using Type = T;
  using type = T;
};

}  // namespace etl

#endif  // _ETL_TYPE_CONSTANT_H_INCLUDED
