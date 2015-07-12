#ifndef _ETL_QUOTE_H_INCLUDED
#define _ETL_QUOTE_H_INCLUDED

/**
 * @addtogroup preprocessor
 * @{
 */

/** @file
 *
 * Provides the #ETL_QUOTE macro.
 */

/**
 * A macro for expanding macros inside string literals.
 *
 * The C preprocessor won't rewrite macros inside string literals, and won't
 * directly stringify a macro.  This is particularly annoying for dynamic
 * `include`s.  The ETL_QUOTE macro rewrites macros inside its argument and
 * turns the result into a string literal.
 *
 * @hideinitializer
 *
 * ~~~{.cc}
 * #define INCLUDE_FILE file/to/include.h
 * // ...later...
 * #include ETL_QUOTE(INCLUDE_FILE)
 * ~~~
 */
#define ETL_QUOTE(x) _ETL_QUOTE(x)
#define _ETL_QUOTE(x) #x

/**@}*/

#endif  // _ETL_QUOTE_H_INCLUDED
