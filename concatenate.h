#ifndef _ETL_CONCATENATE_H_INCLUDED
#define _ETL_CONCATENATE_H_INCLUDED

/*
 * A macro for concatenating text tokens.  Doing this is surprisingly involved
 * with the C preprocessor.
 */
#define ETL_CONCATENATE(a, b) _ETL_CONCATENATE(a, b)
#define _ETL_CONCATENATE(a, b) a ## b

#endif  // _ETL_CONCATENATE_H_INCLUDED
