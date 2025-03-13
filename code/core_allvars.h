#ifndef ALLVARS_H
#define ALLVARS_H

/*
 * This is a transitional header that includes the refactored headers
 * for backward compatibility. In future updates, code should directly
 * include the specific headers needed rather than core_allvars.h.
 *
 * IMPORTANT NOTE:
 * When modifying global parameters that have been moved to the SageConfig
 * structure, you must ensure that both the global variable and the structure
 * field are kept in sync. Some functions still use global variables and some
 * use the SageConfig structure.
 */

#include "config.h"
#include "constants.h"
#include "globals.h"
#include "types.h"

#endif /* #ifndef ALLVARS_H */
