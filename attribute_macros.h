#ifndef _ETL_ATTRIBUTE_MACROS_H_INCLUDED
#define _ETL_ATTRIBUTE_MACROS_H_INCLUDED

/*
 * These are shorthand for GCC's attribute syntax.  (The ETL_INLINE macro
 * also includes a keyword needed to make the attribute work.)
 */

/*
 * Function attributes.  Place before return type in declaration.
 */

// Function does not return.
#define ETL_NORETURN __attribute__((noreturn))

// Function should omit normal prologue/epilogue.
#define ETL_NAKED __attribute__((naked))

// Function body should *really* not be generated outline.
// This overrides conservative heuristics at -Os.
#define ETL_INLINE inline __attribute__((always_inline))

// Function should *never* be inlined.  Really.
#define ETL_NOINLINE __attribute__((noinline))


/*
 * Data attributes.
 */

// Lay out data as specified.  May hurt efficiency.
#define ETL_PACKED __attribute__((packed))

// Ensure object starts at multiple of n bytes.
#define ETL_ALIGNED(n) __attribute__((aligned(n)))


/*
 * Multi-purpose attributes
 */

// Put object in specified linker section.
#define ETL_SECTION(s) __attribute__((section(s)))

// Consider object to be 'used' during linker GC.
#define ETL_USED __attribute__((used))

// Object can be overridden during link time.
#define ETL_WEAK __attribute__((weak))

// Object is an alias for something else.
#define ETL_ALIAS(sym) __attribute__((alias(sym)))


#endif  // _ETL_ATTRIBUTE_MACROS_H_INCLUDED
