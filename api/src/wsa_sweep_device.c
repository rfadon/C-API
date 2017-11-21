///
/// @ingroup sweep
///
/// @{
///

///
/// @file
/// Implementation of the sweep configuration and spectrum data collection module.
///
/// Full documentation is in wsa_sweep_device.h.
///
/// @author Mohammad Farhan (original)
/// @author Richard Low (rewrite of wsa_plan_sweep() and wsa_capture_power_spectrum()).
///
/// @date 2017-09-25 (rewrite)
///
/// @copyright (C) 2017 ThinkRF Inc.
///

///
/// \name External References
///
/// @{

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "debug_printf.h"				// Useful diagnostic output function.

#define _USE_MATH_DEFINES
#include <math.h>

#include "wsa_sweep_device.h"
#include "wsa_lib.h"
#include "wsa_dsp.h"
#include "wsa_debug.h"
#include "wsa_error.h"

// #define FIXED_POINT 32
#include "kiss_fft.h"

/// @}
///
/// \name Internal Constants
///
/// @{

#define HZ (1ULL)					///< Hz per Hz
#define KHZ (1000ULL)				///< Hz per kHz
#if defined (MHZ)
#undef MHZ							// Use ours instead.
#endif
#define MHZ (1000000ULL)			///< Hz per MHz
#define GHZ (1000000000ULL)			///< Hz per GHz

/// Device modes.
#define MODE_ZIF 1					///< Zero IF mode
#define MODE_HDR 2					///< High dynamic range mode
#define MODE_SH 3					///< Superheterodyne mode
#define MODE_SHN 4					///< Narrowband superheterodyne mode
#define MODE_DECSH 5				///< SH mode with decimation
#define MODE_DECSHN 6				///< SHN mode with decimation
#define MODE_IQIN 7					///< Unknown mode
#define MODE_DD 8					///< Direct digitization mode
#define MODE_AUTO 255				///< Unknown mode

// Data sample types.
#define SAMPLETYPE_IQ 1				///< Complex data
#define SAMPLETYPE_I_ONLY 2			///< Real data

// Internal error values used only in this module.
#define EFREQOUTOFRANGE (1)			///< Frequency out of range
#define EUNSUPPORTED (2)			///< Unsupported device mode
#define EINVCAPTSIZE (3)			///< NOTUSED
#define ESWEEPNOMEM (4)				///< NOTUSED
#define EBADPARAM (5)				///< Bad device or sweep configuration parameter

#define TIMEOUT_5S (5000)			///< Timeout used in reading packets.

/// @}
///
/// \name Private Objects and Functions
///
/// @{

/// Table of device properties for each operating mode.
static struct wsa_sweep_device_properties_t wsa_sweep_device_properties[] = {

    // Legend for Each Entry
    // {
    // 	mode, sample_type, fshift_available
    // 	min_tunable, max_tunable, tuning_resolution,
    //	full_bw, usable_bw, passband_center, usable_left, usable_right
    // 	min_decimation, max_decimation,
    // }

    /// SHN Mode
    {
        MODE_SHN, SAMPLETYPE_I_ONLY, 1,
        50ULL * MHZ, 27ULL * GHZ, 10,
        62500 * KHZ, 10 * MHZ, 35 * MHZ, 30 * MHZ, 40 * MHZ,
        4, 512
    },

    /// SH Mode
    {
        MODE_SH, SAMPLETYPE_I_ONLY, 1,
        50ULL * MHZ, 27ULL * GHZ, 10,
        62500 * KHZ, 40 * MHZ, 35 * MHZ, 15 * MHZ, 55 * MHZ,
        4, 512
    },
    /// DD Mode
    {
        MODE_DD, SAMPLETYPE_I_ONLY, 1,
        50ULL * MHZ, 27ULL * GHZ, 10,
        62500 * KHZ, 50 * MHZ, 31250 * KHZ, 0 * MHZ, 50 * MHZ,
        1, 1
    },
    /// Terminate with null entry.
    {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0
    }
};


///
/// Convert a mode from a string to an integer.
///
/// @param[in] modestr A character string indicating a device mode.
///
/// @returns 0 if the mode string is invalid, otherwise the constant equivalent to the mode string
///
static uint32_t mode_string_to_const( const char *modestr )
{
    if (!strcmp(modestr, "ZIF"))
        return MODE_ZIF;
    else if (!strcmp(modestr, "HDR"))
        return MODE_HDR;
    else if (!strcmp(modestr, "SH"))
        return MODE_SH;
    else if (!strcmp(modestr, "SHN"))
        return MODE_SHN;
    else if (!strcmp(modestr, "DECSH"))
        return MODE_DECSH;
    else if (!strcmp(modestr, "DECSHN"))
        return MODE_DECSHN;
    else if (!strcmp(modestr, "IQIN"))
        return MODE_IQIN;
    else if (!strcmp(modestr, "DD"))
        return MODE_DD;
    else if (!strcmp(modestr, "AUTO"))
        return MODE_AUTO;

    return 0;
}


///
/// Convert a device mode from an integer to a character string.
///
/// @param[in] modeint An integer indicating a device mode.
///
/// @returns The equivalent mode string if the integer is valid, otherwise "NULL".
///
static const char *mode_const_to_string(uint32_t modeint)
{
    if (modeint == MODE_ZIF)
        return "ZIF";
    else if (modeint == MODE_HDR)
        return "HDR";
    else if (modeint == MODE_SH)
        return "SH";
    else if (modeint == MODE_SHN)
        return "SHN";
    else if (modeint == MODE_DECSH)
        return "DECSH";
    else if (modeint == MODE_DECSHN)
        return "DECSHN";
    else if (modeint == MODE_IQIN)
        return "IQIN";
    else if (modeint == MODE_DD)
        return "DD";
    else if (modeint == MODE_AUTO)
        return "AUTO";

    return NULL;
}


///
/// Create a new sweep plan entry and initialize it with the values given.
///
/// @param[in] fcstart The initial tuning frequency in Hz.
/// @param[in] fcstop The final tuning frequency in Hz.
/// @param[in] fstep The desired tuning step size in Hz.
/// @param[in] spp The number of samples per data packet.
/// @param[in] ppb The number of data packets per block (tuning step).
/// @param[in] dd_mode A boolean value which when true indicates that this sweep includes a DD mode block.
///
/// @return A pointer to the allocated sweep plan structure, or NULL if the allocation failed.
///
struct wsa_sweep_plan *wsa_sweep_plan_entry_new(uint64_t fcstart, uint64_t fcstop, uint32_t fstep, uint32_t spp, uint32_t ppb, uint8_t dd_mode)
{
    struct wsa_sweep_plan *plan;

    plan = malloc(sizeof(struct wsa_sweep_plan));
    if (plan == NULL)
        return NULL;

    plan->next_entry = NULL;
    plan->fcstart = fcstart;
    plan->fcstop = fcstop;
    plan->fstep = fstep;
    plan->spp = spp;
    plan->ppb = ppb;
    plan->dd_mode = dd_mode;

    return plan;
}


///
/// Retrieve the appropriate properties structure for the requested device mode.
///
/// @param[in] mode A character string holding the desired device mode.
/// @return A pointer to the properties structure, or NULL if the mode is not supported.
///
static struct wsa_sweep_device_properties_t *wsa_get_sweep_device_properties( uint32_t mode )
{
    struct wsa_sweep_device_properties_t *ptr;

    for (ptr = wsa_sweep_device_properties; ptr->mode; ptr++) {
        if (ptr->mode == mode)
            return ptr;
    }

    // Couldn't find the mode, so indicate failure.
    return NULL;
}


///
/// Converts desired sweep parameters and a selected sweep device to a
/// sweep configuration for that device.
///
/// @author Mohammad Farhan (original)
/// @author Richard Low (rewrite)
///
/// @date 2017-8-31 (rewrite)
///
/// @param[in] sweep_device The sweep device object we're operating on
/// @param[in,out] pscfg The config object which describes what we're trying to sweep
/// @retval[out] 0 Success
/// @retval[out] negative An error has occurred.
///
/// @note
/// 1. Only SH, SHN modes are supported right now, with no decimation.
/// 2. The following elements of *pscfg are output:
///		- samples_per_packet
///		- packets_per_block
///		- only_dd
///		- sweep_plan
///		- packet_total
///
static int16_t wsa_plan_sweep( struct wsa_sweep_device *sweep_device, struct wsa_power_spectrum_config *pscfg )
{
    struct wsa_sweep_device_properties_t *mode_props = NULL;
    struct wsa_descriptor dev_props = sweep_device->real_device->descr;
    struct wsa_sweep_plan *our_plan = NULL;

    uint64_t fcstart;
    uint64_t fcstop;
    uint64_t block_count;

    uint32_t fstep;
    uint32_t half_usable_bw;
    uint32_t required_points;
    uint32_t actual_spp;
    uint32_t actual_ppb;

    uint8_t need_dd_mode = FALSE;

    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "%s", "REQUEST");
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Start freq (fstart): %12llu Hz", pscfg->fstart);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Stop freq (fstop):   %12llu Hz", pscfg->fstop);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "RBW:                 %12llu Hz", pscfg->rbw);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Rcvr mode:           %s", mode_const_to_string(pscfg->mode));

    // 1. Sanity check input parameters.

    // Null pointers
    if (!sweep_device || !pscfg) {
        DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "%s", "?? Bad device or bad sweep config.");
        return (-EBADPARAM);
    }

    // Nonsense frequency range
    if (pscfg->fstop < (pscfg->fstart + pscfg->rbw)) {
        DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "%s", "?? Bad frequency range.");
        return (-EFREQOUTOFRANGE);
    }

    // Check that frequency parameters are in range for the device.
    if ((pscfg->fstart < dev_props.min_tune_freq) || (pscfg->fstop > dev_props.max_tune_freq)) {
        DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "%s", "?? Frequency range exceeds device capability.");
        return (WSA_ERR_INV_SWEEP_FREQ);
    }


    // 2. Calculate sweep parameters.

    // Get properties for the mode we want to use.
    mode_props = wsa_get_sweep_device_properties(pscfg->mode);
    if (!mode_props) {
        return (-EUNSUPPORTED);
    }

    // Get points per sweep segment.
	// Round up ensure we get at least RBW resolution.
    required_points = (mode_props->full_bw + (uint32_t)pscfg->rbw - 1) / (uint32_t)pscfg->rbw;
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Original points per segment: %lu", required_points);

    // TODO: Compensate for width of FFT window function. E.g. if window width is 4 bins, we need rbw = rbw / 4.
	//       Defer until next generation API so we don't affect all the other software that does this the same way.

    // Round up to a multiple of WSA_SPP_MULTIPLE due to hardware constraints.
    required_points = ((required_points + WSA_SPP_MULTIPLE - 1) / WSA_SPP_MULTIPLE) * WSA_SPP_MULTIPLE;

    // We only support SH, SHN with no decimation for now.
    // This means only real data, so double the FFT size.
    required_points *= 2;

    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Final points per segment: %lu", required_points);

    //
    // Now we need to calculate the following parameters:
    // - number of sweep list entries
    // - for each entry:
    //    - start frequency
    //    - stop frequency
    //    - frequency step size
    //    - samples per packet \____ same for all sweep list entries
    //    - packets per block  /
    //    - DD mode flag
    //

    // We want to maximize packet size to make most efficient use of buffers.
    // Min packet size is WSA_MIN_SPP; max is WSA_MAX_SPP (currently 256, 64000).
    // We already made sure above that actual_spp is at least WSA_MIN_SPP.
    if (required_points < WSA_MIN_SPP) {
        actual_spp = WSA_MIN_SPP;
        actual_ppb = 1;
    } else if (required_points > WSA_MAX_SPP) {
        actual_spp = WSA_MAX_SPP;
        actual_ppb = ((required_points % WSA_MAX_SPP) == 0) ? required_points / WSA_MAX_SPP : required_points / WSA_MAX_SPP + 1;
    } else {
        actual_spp = required_points;
        actual_ppb = 1;
    }
    pscfg->samples_per_packet = actual_spp;
    pscfg->packets_per_block = actual_ppb;

    // Compute start and stop frequencies, step size.

    half_usable_bw = mode_props->usable_bw / 2;

    ///
    // @note
    // In SH, SHN modes, the tuning frequency is at the 35 MHz point in the baseband, and there is
    // half_usable_bw on either side of this.

    // 1) Start frequency -- depends on whether we need DD mode.
    if (pscfg->fstart < mode_props->min_tunable) {
        need_dd_mode = TRUE;

        // fcstart is for the non-DD segments.
        fcstart = mode_props->min_tunable + half_usable_bw;

    } else {
        need_dd_mode = FALSE;
		fcstart = pscfg->fstart + half_usable_bw;
    }

	// But fstart_actual is the requested start frequency, whether DD mode or not.
	pscfg->fstart_actual = pscfg->fstart;

    // Round down by tuning resolution to prevent any gap.
	fcstart = (fcstart / (uint64_t)mode_props->tuning_resolution) * (uint64_t)mode_props->tuning_resolution;

	// Same for our actual start frequency.
	pscfg->fstart_actual = (pscfg->fstart_actual / (uint64_t)mode_props->tuning_resolution) * (uint64_t)mode_props->tuning_resolution;

    // 2) Step size
    // Set to one RBW less than usable bandwidth so we don't miss anything at the boundary of two segments.
    // Then quantize to tuning resolution so our final fstep might be even a bit smaller.
    fstep = mode_props->usable_bw - (uint32_t)pscfg->rbw;
    fstep = (fstep / (uint64_t)mode_props->tuning_resolution) * (uint64_t)mode_props->tuning_resolution;

    // 3) Stop frequency -- the centre frequency that's the first multiple of fstep above fcstart that
    // exceeds the requested stop frequency. This will ensure we don't miss the last little bit of the spectrum.
	fcstop = fcstart + (((pscfg->fstop - fcstart) + (uint64_t)fstep - 1LLU) / (uint64_t)fstep) * (uint64_t)fstep;
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, " Rounded up fcstop: %llu", fcstop);

    // Constrain fcstop.
    // Among other things, if we need DD mode and fstop was < 50MHz, start might now be greater than stop.
    // Also, if fcstop exceeds maximum tunable frequency, back it off by one step.
	fcstop = (fcstop < fcstart) ? fcstart : fcstop;
    if (fcstop >= dev_props.max_tune_freq) {
        fcstop -= fstep;
    }
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Constrained fcstop: %llu", fcstop);

	pscfg->fstop_actual = fcstop + half_usable_bw;

    // Check if we will only get DD mode data.
    pscfg->only_dd = (need_dd_mode && (pscfg->fstop < mode_props->min_tunable)) ? TRUE : FALSE;

    // Create sweep plan entry.
    pscfg->sweep_plan = wsa_sweep_plan_entry_new(fcstart, fcstop, fstep, required_points, actual_ppb, need_dd_mode);

    // Calculate total number of data blocks and thus packet total (a multiple of number of blocks).
    block_count = 1 + (fcstop - fcstart) / (uint64_t)fstep;
    if (need_dd_mode) {
        // Assume DD mode will use one block.
        // TODO: Verify DD mode only needs one block.
        block_count++;
    }
    pscfg->packet_total = (uint32_t)block_count * actual_ppb;

    // Dump info on what we did if the correct flags are set.
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "%s", "AS CONFIGURED");
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Start freq (fcstart):  %12llu Hz", fcstart);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Stop freq (fcstop):    %12llu Hz", fcstop);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Step size (fstep):     %12lu Hz", fstep);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "SPP:                   %12lu", pscfg->samples_per_packet);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "PPB:                   %12lu", pscfg->packets_per_block);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Step count:            %12llu", block_count);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "DD segment needed:     %s", need_dd_mode ? "YES" : "NO");
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "DD only?               %s", pscfg->only_dd ? "YES" : "NO");
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "Total packets:         %12lu", pscfg->packet_total);

    return (0);
}


///
/// Convert a sweep plan into a list of sweep entries and load them into the device.
///
/// @param sweep_device The sweep device to use.
/// @param cfg Pointer to the sweep configuration which holds all sweep info, including the sweep plan.
///
/// @return 0 on success, otherwise a negative error code.
///
static int16_t wsa_sweep_plan_load( struct wsa_sweep_device *wsasweepdev, struct wsa_power_spectrum_config *cfg )
{
    int16_t result;
    struct wsa_device *wsadev = wsasweepdev->real_device;
    struct wsa_sweep_plan *plan_entry;
    int32_t atten_val = 0;
    char dd[255] = "DD";
    char atten_cmd[255];
    char * strtok_context = NULL;
    plan_entry = cfg->sweep_plan;

    // Reset the device before the capture.
    result = wsa_reset(wsadev);

    // Abort any previous capture, flush and clean
    result = wsa_system_abort_capture(wsadev);    
    result = wsa_flush_data(wsadev);
	result = wsa_clean_data_socket(wsadev);

    // Get the device ID and initialize the object.
    result = _wsa_dev_init(wsadev);

    // Clear any existing sweep entries.
    wsa_sweep_entry_delete_all(wsadev);

    // Create a new sweep entry.
    wsa_sweep_entry_new(wsadev);

    // Setup the sweep list to only run once.
    wsa_set_sweep_iteration(wsadev, 1);

    atten_val = (int32_t)wsasweepdev->device_settings.attenuator;

    // Set attenuation if the device is a 408 model.
    if (strstr(wsadev->descr.dev_model, WSA5000408) != NULL ||
        strstr(wsadev->descr.dev_model, R5500408) != NULL) {
        result = wsa_set_sweep_attenuation(wsadev, atten_val);
    }
    // Send the command for 418/427 models.
    else {
        sprintf(atten_cmd, "SWEEP:ENTRY:ATT:VAR %u\n", atten_val);
        result = wsa_send_scpi(wsadev, atten_cmd);
    }

    // if DD mode is required, create one sweep entry with DD mode.
    if (cfg->sweep_plan->dd_mode == 1) {
        result = wsa_set_sweep_rfe_input_mode(wsadev, dd);
        result = wsa_set_sweep_samples_per_packet(wsadev, (int32_t)(plan_entry->spp));
        result = wsa_set_sweep_packets_per_block(wsadev, (int32_t)(plan_entry->ppb));
        result = wsa_sweep_entry_save(wsadev, 0);
    }

    // Set sweep-wide settings.
    result = wsa_set_sweep_rfe_input_mode(wsadev, mode_const_to_string(cfg->mode));

    // Loop over sweep plan, converting to sweep entries and save.
    for (plan_entry = cfg->sweep_plan; plan_entry; plan_entry = plan_entry->next_entry) {

        // Set settings and check the errors.
        result = wsa_set_sweep_freq(wsadev, (uint64_t)plan_entry->fcstart, (uint64_t)plan_entry->fcstop);
        doutf(DHIGH, "wsa_sweep_plan_load: Setting sweep entry start freq: %0.6f, stop %0.6f \n", (float)plan_entry->fcstart, (float)plan_entry->fcstop);
        DEBUG_PRINTF(DEBUG_SWEEP_CFG, "Sweep start: %0.6f, stop: %0.6f", (float)plan_entry->fcstart, (float)plan_entry->fcstop);
        if (result < 0) {
            fprintf(stderr, "ERROR %d fstart fstop\n", result);
        }

        result = wsa_set_sweep_freq_step(wsadev, (uint64_t)plan_entry->fstep);
        DEBUG_PRINTF(DEBUG_SWEEP_CFG, "Sweep step: %0.6f", (float)plan_entry->fstep);
        if (result < 0) {
            fprintf(stderr, "ERROR fstep\n");
        }

        wsa_set_sweep_samples_per_packet(wsadev, plan_entry->spp);
        if (result < 0) {
            fprintf(stderr, "ERROR spp\n");
        }

        wsa_set_sweep_packets_per_block(wsadev, plan_entry->ppb);
        if (result < 0) {
            fprintf(stderr, "ERROR ppb\n");
        }

        // Save to end of list.
        if (cfg->only_dd != 1) {
            wsa_sweep_entry_save(wsadev, 0);
        }
    }
    return 0;
}


/// @}
///
/// \name Public Functions
/// @{

struct wsa_sweep_device *wsa_sweep_device_new( struct wsa_device *device )
{
    struct wsa_sweep_device *sweepdev;

    // Allocate memory for our object.
    sweepdev = malloc(sizeof(struct wsa_sweep_device));
    if (sweepdev == NULL)
        return NULL;

    // Initialize the structure.
    sweepdev->real_device = device;

    return sweepdev;
}


void wsa_sweep_device_free( struct wsa_sweep_device *sweepdev )
{
    // Free the memory of the sweep device, (but not the real device, it came from the parent).
    free(sweepdev);
}


void wsa_sweep_device_set_attenuator( struct wsa_sweep_device *sweep_device, unsigned int val )
{
    sweep_device->device_settings.attenuator = (uint8_t)val;
}


unsigned int wsa_sweep_device_get_attenuator( struct wsa_sweep_device *sweep_device )
{
    return sweep_device->device_settings.attenuator;
}


int16_t wsa_power_spectrum_alloc( struct wsa_sweep_device *sweep_device, uint64_t fstart, uint64_t fstop,
                                  uint32_t rbw, char const *mode, struct wsa_power_spectrum_config **pscfgptr )
{
    struct wsa_power_spectrum_config *pscfg;
    int16_t result;

    uint32_t calculated_rbw = 0;

    pscfg = malloc(sizeof(struct wsa_power_spectrum_config));
    if (pscfg == NULL) {
        doutf(DHIGH, "wsa_power_spectrum_alloc: Failed to initialize struct wsa_power_spectrum_config\n");
        return -15;
    }

    // Initialize a few things.
    pscfg->sweep_plan = NULL;
    pscfg->buf = NULL;

    // Copy the sweep settings into the sweep configuration object.
    pscfg->mode = mode_string_to_const(mode);
    pscfg->fstart = fstart;
    pscfg->fstop = fstop;
    pscfg->rbw = (uint32_t)rbw;

    // Find a way to collect the data.
    result = wsa_plan_sweep(sweep_device, pscfg);
    if (result < 0) {
        return result;
    }

    // Now allocate enough buffer for the spectrum.
	pscfg->buflen = (uint32_t)(((float)(pscfg->fstop_actual - pscfg->fstart_actual)) / ((float)(pscfg->rbw)));

    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "actual fstart = %llu, actual fstop = %llu, rbw = %llu", pscfg->fstart_actual, pscfg->fstop_actual, pscfg->rbw);
    doutf(DHIGH, "wsa_power_spectrum_alloc: Calculated Buffer length to be: %d\n", pscfg->buflen);
    DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "buflen = %lu", pscfg->buflen);

    // Allocate data to the buffer.
    pscfg->buf = malloc(sizeof(float) * pscfg->buflen);

    if (pscfg->buf == NULL) {
        DEBUG_PRINTF(DEBUG_SWEEP_PLAN, "?? Malloc failed for pscfg->buf");
        free(pscfg);
        return -1;
    }

    *pscfgptr = pscfg;
    return 0;
}


void wsa_power_spectrum_free( struct wsa_power_spectrum_config *cfg )
{
    struct wsa_sweep_plan *plan, *next;

    // Free the plan, if there is one.
    plan = cfg->sweep_plan;
    for (;;) {

        // List is null terminated.
        if (plan == NULL)
            break;

        // Store next pointer before freeing.
        next = plan->next_entry;

        // Free the memory.
        free(plan);

        // Go to the next item.
        plan = next;
    }

    // Free the buffer.
    if (cfg->buf) {
        free(cfg->buf);
    }

    // Free the struct.
    free(cfg);
}


int16_t wsa_configure_sweep(struct wsa_sweep_device *sweep_device, struct wsa_power_spectrum_config *pscfg)
{
    int16_t result = 0;

    // Load the sweep plan.
    result = wsa_sweep_plan_load(sweep_device, pscfg);

    return result;
}


int16_t wsa_capture_power_spectrum(struct wsa_sweep_device *sweep_device,
                                   struct wsa_power_spectrum_config *cfg, float **buf)
{

    uint32_t const spectrum_samples = cfg->samples_per_packet * cfg->packet_total;		// Samples in the whole spectrum
    uint32_t const block_samples = cfg->samples_per_packet * cfg->packets_per_block;	// Samples in one block

    struct wsa_device * const dev = sweep_device->real_device;

    struct wsa_sweep_device_properties_t *prop;			// Device properties for mode of sweep
    struct wsa_sweep_device_properties_t *dd_prop;		// Device properties for DD mode

    kiss_fft_scalar *idata;		// Input to FFT
    kiss_fft_cpx *fftout;		// Output from FFT
    kiss_fft_scalar tmpscalar;

    struct wsa_vrt_packet_header header;
    struct wsa_vrt_packet_trailer trailer;
    struct wsa_receiver_packet receiver;
    struct wsa_digitizer_packet digitizer;
    struct wsa_extension_packet sweep;

    float pkt_reflevel = 0;
    float tmp_float;

    uint64_t pkt_fcenter = 0;

    int32_t *i32_buffer = NULL;
    int16_t *tmp_buffer;
    int16_t *q16_buffer = NULL;

    uint32_t i;
    uint32_t total_packet_count;
    uint32_t buf_offset = 0;
    uint32_t istart, istop, ilen;
    uint32_t samples_per_block, fftlen;
    uint32_t packet_count_this_block;
    uint32_t offset;
    uint32_t tmp_u32;
    uint32_t total_samples = 0;

    int16_t result;
    int16_t dd_packet = 0;

    // Allocate space for each data buffer.
    // TODO: Check for malloc() returning NULL everywhere, even though this should never happen.
    tmp_buffer = (int16_t *)malloc(sizeof(int16_t) * cfg->samples_per_packet);			// Buffer to hold one packet's data.
    idata = (kiss_fft_scalar *)malloc(sizeof(kiss_fft_scalar) * block_samples);			// Buffer to hold one block (time domain).
    fftout = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx) * block_samples);				// Buffer to hold one block (freq domain).
    doutf(DHIGH, "wsa_capture_power_spectrum: Created data buffers, block size is %lu", block_samples);

    // Assign the caller's convenience pointer.
    if (*buf) {
        *buf = cfg->buf;
    }

    // Poison our buffer.
    // Buflen is the length of the complete power spectrum buffer, i.e. (fstop - fstart) / rbw.
    for (i = 0; i < cfg->buflen; i++) {
        cfg->buf[i] = POISONED_BUFFER_VALUE;
    }

    // Get device properties for this mode.
    prop = wsa_get_sweep_device_properties(cfg->mode);
    if (prop == NULL) {
        doutf(DMED, "Unsupported RFE mode: %d - %s\n", cfg->mode, mode_const_to_string(cfg->mode));
        return -EUNSUPPORTED;
    }

    // Get the properties for DD mode.
    dd_prop = wsa_get_sweep_device_properties(MODE_DD);

    // Start the sweep.
    wsa_sweep_start(sweep_device->real_device);
    doutf(DHIGH, "wsa_capture_power_spectrum() Sweep started.\n");

    // PROCESS PACKETS OF INTEREST

    total_packet_count = 0;
    packet_count_this_block = 0;
    header.packet_type = IF_PACKET_TYPE;		// Forces buffer poisoning first time through.

    do {

        // Check if we're expecting a DD mode block.
        // It will be the first block.
        dd_packet = ((total_packet_count < cfg->packets_per_block) && (cfg->sweep_plan->dd_mode == 1)) ? 1 : 0;

        // Read the next packet.
        result = wsa_read_vrt_packet(dev, &header, &trailer, &receiver, &digitizer, &sweep,
                                     tmp_buffer, q16_buffer, i32_buffer, cfg->samples_per_packet, TIMEOUT_5S);

        if (result < 0) {
            doutf(DHIGH, "wsa_read_vrt_packet() returned error %d\n", result);

            // We will return with the work incomplete.
            // The caller should detect the error code and take action.
            return result;
        }

        //  Watch for the receiver context packets we need...
        if ((header.packet_type == CONTEXT_PACKET_TYPE) && (header.stream_id == RECEIVER_STREAM_ID)) {

            // ...and grab the center frequency from each.
            if ((receiver.indicator_field & FREQ_INDICATOR_MASK) == FREQ_INDICATOR_MASK) {
                pkt_fcenter = (uint64_t)receiver.freq;
				assert((pkt_fcenter >= cfg->fstart_actual) && (pkt_fcenter <= cfg->fstop_actual));

                // TODO Check that this center frequency does not ever change over one block.
            }
        }

        // Process a data packet.
        // This involves converting data format, loading packet data into the correct place in the
        // larger block buffer, and if this is the last packet in the block, converting to
        // frequency domain and copying the correct slice to the output buffer.
        else if (header.packet_type == IF_PACKET_TYPE) {

            // TODO: Check that data packets are the stream type we expect (I14 sign extended to int16).
            // TODO: If we don't have a pkt_center frequency here, then bail out and return an error code.

            doutf(DMED, "wsa_capture_power_spectrum: Received data packet at %llu Hz.\n", pkt_fcenter);

            pkt_reflevel = (float)digitizer.reference_level;

            // Move incoming data into the FFT input buffer at the correct
            // location and convert to range [-1.0, +1.0].
            offset = packet_count_this_block * cfg->samples_per_packet;
            for (i = 0; i < (int)cfg->samples_per_packet; i++) {
                idata[offset + i] = (float)tmp_buffer[i] / 8192.0f;			// TODO: Check this math is done correctly.
            }

            packet_count_this_block++;
            total_packet_count++;

            DEBUG_PRINTF(DEBUG_COLLECT, "Received data packet %lu at %llu Hz.", total_packet_count, pkt_fcenter);

            // If we're done a block, process it.
            if (packet_count_this_block >= cfg->packets_per_block) {

                packet_count_this_block = 0;

                samples_per_block = header.samples_per_packet * cfg->packets_per_block;
                DEBUG_PRINTF(DEBUG_COLLECT, "Processing a block, length = %lu samples.", samples_per_block);

                // TODO: Remove the need to keep two sets of books?
                // Cfg->samples_per_packet should be identical to header.samples_per_packet.
                // Also, samples_per_block should be == block_samples.

                // We only support SH mode with no decimation, so data is known to be from an I16 packet, i.e. only real data.

                // Window and normalize the data. We only support Hanning window for now.
                // TODO: Speed up windowing if possible.
                // TODO: Add more window types.
                window_hanning_scalar_array(idata, samples_per_block);

                // Transform to frequency domain.
                // TODO: Check how we can speed up the FFT.
                // TODO: Check if we can zero-pad after windowing and use only radix-2 FFTs.
                rfft(idata, fftout, samples_per_block);

                // Real input data, so only half the FFT output data is needed.
                // We doubled this up back in wsa_plan_sweep() when we realized we were only going to use SH or SHN modes.
                fftlen = samples_per_block / 2;
                DEBUG_PRINTF(DEBUG_COLLECT, "FFT length = %lu", fftlen);

                // Extract the correct slice of spectrum data.
                if (0 == dd_packet) {

                    // Non-DD Mode

                    // Calculate indices of the slice of data we want.
                    if (trailer.spectral_inversion_indicator) {

                        // Spectral inversion, so reverse the data.
                        reverse_cpx(fftout, fftlen);		// At this point fftlen is only the lower half of the spectrum data.

                        // Now the start index is the width of the upper skirt band up from the bottom,
                        // and the end is the width of the lower skirt band up down from the upper end.
                        // E.g. with full BW = 62.5 MHz, lower edge at 15 and upper edge at 55,
                        //     - start index is (62.5 - 55) / 62.5 X data length, and
                        //     - stop index is (62.5 - 15 / 62.5) X data length.
                        // Use rounding, not truncation.
                        istart = (uint32_t)(((float)fftlen + 0.5f) * (float)(prop->full_bw - prop->usable_right) / (float)prop->full_bw);
                        istop = (uint32_t)(((float)fftlen + 0.5f) * (float)(prop->full_bw - prop->usable_left) / (float)prop->full_bw);
                        DEBUG_PRINTF(DEBUG_COLLECT, "Non-DD, inverted spectrum. istart = %u, istop = %u", istart, istop);

                    } else {

                        // Normal data, so the start is "usable_left" up from the bottom,
                        // and the end is "usable_right" down from the top.
                        // Use rounding, not truncation.
                        istart = (uint32_t)(((float)fftlen + 0.5f) * (float)prop->usable_left / (float)prop->full_bw);
                        istop = (uint32_t)(((float)fftlen + 0.5f) * (float)prop->usable_right / (float)prop->full_bw);

                        DEBUG_PRINTF(DEBUG_COLLECT, "Non-DD, normal spectrum. istart = %lu, istop = %lu", istart, istop);

                    }

                    ilen = istop - istart;
                    assert(ilen < fftlen);
                    DEBUG_PRINTF(DEBUG_COLLECT, "ilen = %lu", ilen);

                    // Now calculate where that slice has to go in the power spectrum output buffer,
                    // based on the centre frequency of this block reported by the device.
                    // We will probably overwrite some previously written data and that is OK.
                    // Make sure we don't underflow with the first buffer.
                    //
                    tmp_float = (float)(pkt_fcenter - cfg->fstart_actual) / (float)(cfg->fstop_actual - cfg->fstart_actual);		// Fraction of the sweep
                    tmp_float *= (float)(cfg->buflen);														// Fraction of the buffer
                    tmp_u32 = (uint32_t)(tmp_float + 0.5f);													// Rounded up
                    if (tmp_u32 < (ilen / 2)) {
                        buf_offset = 0;
                    } else {
                        buf_offset = tmp_u32 - ilen / 2;		// We just computed offset of centre of packet. Find offset of lower edge.
                    }

                    assert((buf_offset >= 0) && (buf_offset < cfg->buflen));

				} else {

                    // DD Mode

                    istart = (uint32_t)(((float)fftlen + 0.5f) * (float)cfg->fstart / (float)prop->full_bw);

                    // Fstart is the original start freq for the sweep, unlike fcstart.

                    // If fstop is higher than the upper edge of DD band (50MHz), then for
                    // a DD mode segment, just take data up to that band edge.
                    // This point in the FFT data will be 50 MHz / 62.5 MHz, or at 0.8 of the buffer.
                    if (cfg->fstop > prop->min_tunable) {
                        istop = (uint32_t)(0.8f * ((float)fftlen + 0.5f));
                    } else {
                        istop = (uint32_t)(((float)fftlen + 0.5f) * (float)cfg->fstop / (float)prop->full_bw);
                    }

                    ilen = istop - istart;
                    assert(ilen < fftlen);

                    buf_offset = 0;				// Target for DD mode spectral data is always the first part of the output buffer.
                    DEBUG_PRINTF(DEBUG_COLLECT, "DD, normal spectrum. istart = %lu, istop = %lu, ilen = %lu", istart, istop, ilen);

                }

                // For the usable section, convert to power, apply reflevel and copy into buffer.
                // Loop until end of input data or end of output buffer, whichever comes first.
                for (i = 0; ((i < ilen) && (buf_offset + i < cfg->buflen)); i++) {
                    tmpscalar = cpx_to_power(fftout[i + istart]) / samples_per_block;
                    tmpscalar = 2 * power_to_logpower(tmpscalar);
                    cfg->buf[buf_offset + i] = tmpscalar + pkt_reflevel - (float)KISS_FFT_OFFSET;
                }

                // Keep track of total number of spectrum samples (bins).
                // TODO: Confirm assumption of monotonically increasing buf_offset, with no holes in the spectral data, i.e. blocks have
                //       steadily increasing centre frequency, and new data always overwrites a bit of the old or exactly abuts.
                total_samples = buf_offset + i;

            }	// endif (ppb count == PPB)

        }	// dnd if (header type == IF_PACKET_TYPE)

        else {
            // TODO: Handle other packet types we're not interested in.
        }

    } while (total_packet_count < cfg->packet_total);

    DEBUG_PRINTF(DEBUG_COLLECT, "total_samples = %lu", total_samples);

	//*** Heavyweight resync don@bearanascence.com 16Nov17
	{
		int iResult = 0;
		//iResult = wsa_system_abort_capture(dev);
		//iResult = wsa_flush_data(dev);
		// Test case: 9kHz-8GHz/20kHz RBW sweep
		// With the following line in place, the gap occurs at the top end of the spectrum.
		// If this line is commented out, then the gap monotonically advances down through
		// the buffer on each successive sweep. 
		//iResult = wsa_clean_data_socket(dev);  
	}
	//*** end of heavyweight resync

	//*** Poison-search in buffer don@bearanascence.com 16Nov17
	{
		// if total_samples != cfg->buflen, we know we are in trouble...
		int iPoisonFound = 0;	// Total poison values found
		int iPoisonRunStart = 0;	// Start index of most recently found run
		int iPoisonRunLength = 0;	// length of most recently found run
		int bInPoison = 0;	// Interpret boolean
		for (int i = 0; i < cfg->buflen; i++) {
			if (POISONED_BUFFER_VALUE == cfg->buf[i]) {
				iPoisonFound++;
				if (0 == bInPoison) {
					bInPoison = 1;
					iPoisonRunStart = i;
					iPoisonRunLength = 1;
				} else {
					iPoisonRunLength += 1;
				}
			} else {
				bInPoison = 0;
			}
		}
		if (iPoisonFound > 0) {
			int j = 0;	// So I can breakpoint within this scope
		}
	}
	//*** End of poison-search

    free(fftout);
    free(idata);
    free(tmp_buffer);

	return 0;
}

/// @}				// name Public Functions

/// @}				// group sweep
