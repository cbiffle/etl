#ifndef _ETL_SIZE_H_INCLUDED
#define _ETL_SIZE_H_INCLUDED

namespace etl {

/*
 * A type large enough to hold the maximum size of an object in memory.
 * The standard defines sizeof in relation to this type, so we exploit this.
 */
typedef decltype(sizeof(char(0))) Size;

}  // namespace etl

#endif  // _ETL_SIZE_H_INCLUDED
