#ifndef _ETL_CONCATENATE_H_INCLUDED
#define _ETL_CONCATENATE_H_INCLUDED

/**
 * @addtogroup preprocessor
 * @{
 */

/** @file
 *
 * Provides the #ETL_CONCATENATE macro.
 */

/**
 * A macro for concatenating text tokens after expanding both sides using the
 * preprocessor.  This operation is surprisingly involved.
 *
 * @hideinitializer
 */
#define ETL_CONCATENATE(a, b) _ETL_CONCATENATE(a, b)
#define _ETL_CONCATENATE(a, b) a ## b

/**@}*/

#endif  // _ETL_CONCATENATE_H_INCLUDED
