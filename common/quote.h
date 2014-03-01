#ifndef ETL_COMMON_QUOTE_H_
#define ETL_COMMON_QUOTE_H_

/*
 * The C preprocessor won't rewrite macros inside string literals.  This is
 * particularly annoying for dynamic #includes.  The ETL_QUOTE macro rewrites
 * macros inside its argument and turns the result into a string literal.
 */
#define _ETL_QUOTE(x) #x
#define ETL_QUOTE(x) _ETL_QUOTE(x)

#endif  // ETL_COMMON_QUOTE_H_
