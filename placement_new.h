#ifndef _ETL_PLACEMENT_NEW_H_INCLUDED
#define _ETL_PLACEMENT_NEW_H_INCLUDED

#include "etl/size.h"

#ifndef _NEW  // This header should do nothing in hosted tests.

/*
 * This defines placement new, which cannot be placed in a namespace.
 */

inline void * operator new(etl::Size, void *p) noexcept { return p; }

#endif  // _NEW

#endif  // _ETL_PLACEMENT_NEW_H_INCLUDED
