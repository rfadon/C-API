///
/// @ingroup debug_printf
///
/// @{
///

///
/// @file
/// Definition of global flag variable for debug printf macros.
///
/// @author Richard Low
///
/// @date 2017-11-28
///
/// @copyright (C) 2017 ThinkRF Inc.
///

#include "thinkrf_stdint.h"
#include "debug_printf.h"

// Define a default bitmask with all output disabled.
// See debug_printf.h for possible values of this bitmask, and adjust as desired in your application.
uint32_t g_debug_mask = 0;

/// @}
