///
/// @defgroup debug_printf Configurable printing for debugging.
///
/// This module defines a macro and a set of control bits to allow selective printing of user-specified debugging information.
///
/// @{
///

///
/// @file
/// Implementation of debug printf macros and control bits.
///
/// @author Richard Low
///
/// @date 2017-09-25
///
/// @copyright (C) 2017 ThinkRF Inc.
///

#pragma once

/// Special-purpose flag for diagnosing data dropout 2017-11-10.
/// @note If this flag is set, ensure g_debug_mask does not have the DEBUG_FILE_OUT bit set.
//#define DEBUG_DROPOUT	(0)

/// Configurable debug printing flags
#define DEBUG_ERROR       (1 << 31)
#define DEBUG_WARN        (1 << 30)
#define DEBUG_INFO        (1 << 29)
#define DEBUG_CONFIG      (1 << 28)
#define DEBUG_SWEEP_PLAN  (1 << 27)
#define DEBUG_COLLECT     (1 << 26)
#define DEBUG_SPEC_DATA	  (1 << 25)
#define DEBUG_FILE_OUT    (1 << 24)
#define DEBUG_SWEEP_CFG   (1 << 23)
#define DEBUG_SPEED       (1 << 22)
#define DEBUG_PEAKS       (1 << 21)

/// Special groups of debug settings
#define DEBUG_PERFORMANCE_ALL (DEBUG_ERROR | DEBUG_WARN | DEBUG_INFO)
#define DEBUG_SPECTRUM_ALL    (DEBUG_SWEEP_PLAN | DEBUG_COLLECT | DEBUG_SPEED | DEBUG_PEAKS | DEBUG_SPEC_DATA | DEBUG_FILE_OUT | DEBUG_SWEEP_CFG)
#define DEBUG_ALL			  (DEBUG_PERFORMANCE_ALL | DEBUG_SPECTRUM_ALL)		// Ensure this always has all flags set.

//extern uint32_t g_debug_mask;	///< In user application, set to any combination of above flags to enable corresponding output, or zero for no output.
uint32_t g_debug_mask = 0;	

#define DEBUG_PRINTF(DEBUG_MASK, FMT, ...) \
  do { \
    if(g_debug_mask & DEBUG_MASK) \
      fprintf(stderr, "%s(): " FMT "\n", __FUNCTION__, __VA_ARGS__); \
  } while(0)

/// @}


