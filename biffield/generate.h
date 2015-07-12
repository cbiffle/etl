/*
 * To produce register field, accessor, and type declarations, include this
 * file after defining ETL_BFF_DEFINITION_FILE.
 */
#define _ETL_BFF_INCLUDED_THROUGH_GENERATE

/*
 * The order of these steps is significant.  See the individual files for
 * details.
 */
public:
#include "etl/biffield/generate_constants.h"
#include "etl/biffield/generate_value_types.h"
#include "etl/biffield/generate_accessors.h"

private:
#include "etl/biffield/checks.h"
#include "etl/biffield/generate_fields.h"

public:

#undef _ETL_BFF_INCLUDED_THROUGH_GENERATE
