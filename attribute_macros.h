#ifndef _ETL_ATTRIBUTE_MACROS_H_INCLUDED
#define _ETL_ATTRIBUTE_MACROS_H_INCLUDED

/**
 * @addtogroup preprocessor
 * @{
 */

/** @file
 *
 * Provides macro shorthand for GCC's attribute syntax.
 */

/*
 * Function attributes.  Place before return type in declaration.
 */

/**
 * Marks a function that does not return.  Such functions often have a `void`
 * return type, but this isn't necessary; a *noreturn* function can be used
 * in an expression of any type, because it won't return.
 */
#define ETL_NORETURN __attribute__((noreturn))

/**
 * Marks a function that should omit normal prologue/epilogue.  In GCC this is
 * safe only for functions written in assembly.
 */
#define ETL_NAKED __attribute__((naked))

/**
 * Marks a function that should *really* be inlined.  This overrides
 * conservative heuristics at -Os.
 *
 * Note that this macro includes the `inline` keyword, because GCC requires it
 * for the attribute to work.
 *
 * Think of this as "like normal `inline`, but LOUDER."
 */
#define ETL_INLINE inline __attribute__((always_inline))

/**
 * Marks a function that should *really* *never* be inlined, for real.  This is
 * useful for functions that explicitly manipulate their own return addresses,
 * for example.
 */
#define ETL_NOINLINE __attribute__((noinline))


/*
 * Data attributes.
 */

/**
 * Lay out data in an aggregate as specified, efficiency be damned.
 */
#define ETL_PACKED __attribute__((packed))

/**
 * Ensure object starts at a multiple of `n` bytes.
 *
 * @deprecated should almost always be replaced by the C++11 `alignas`
 * keyword.
 */
#define ETL_ALIGNED(n) __attribute__((aligned(n)))


/*
 * Multi-purpose attributes
 */

/**
 * Put object in specified linker section.
 */
#define ETL_SECTION(s) __attribute__((section(s)))

/**
 * Consider object to be 'used' during linker GC.
 */
#define ETL_USED __attribute__((used))

/**
 * Object can be overridden during link time.
 */
#define ETL_WEAK __attribute__((weak))

/**
 * Object is an alias for something else.
 */
#define ETL_ALIAS(sym) __attribute__((alias(sym)))

/**@}*/

#endif  // _ETL_ATTRIBUTE_MACROS_H_INCLUDED
