#ifndef __SPFG_H__
#define __SPFG_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Version Information
// ----------------------------------------------------------------------------

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

// ----------------------------------------------------------------------------
// Error and Info return codes
// ----------------------------------------------------------------------------

#define SPFG_LOOP_CONTROL_STOP              1
#define SPFG_ERROR_NO                       0
#define SPFG_ERROR_FAIL                    -1
#define SPFG_ERROR_BAD_PARAM_NULL_POINTER  -2
#define SPFG_ERROR_BAD_PARAM_INVALID_VALUE -3
#define SPFG_ERROR_ALREADY_INITIALIZED     -4
#define SPFG_ERROR_NOT_INITIALIZED         -5
#define SPFG_ERROR_OUT_OF_SLOTS            -6
#define SPFG_ERROR_BAD_BLOCK_NAME          -7
#define SPFG_ERROR_NOT_FOUND               -8
#define SPFG_ERROR_INVALID_GR_ID           -9
#define SPFG_ERROR_INVALID_DP_ID           -10
#define SPFG_ERROR_INVALID_FN_ID           -11
#define SPFG_ERROR_EVAL_FN_FAILURE         -12
#define SPFG_ERROR_EVAL_CB_FAILURE         -13
#define SPFG_ERROR_UNIMPLEMENTED           -14
#define SPFG_ERROR_REINDEX                 -15
#define SPFG_ERROR_VALIDATE_FN             -16
#define SPFG_ERROR_BUFFER_OVERFLOW         -17
#define SPFG_ERROR_FN_INTEGRITY            -18
#define SPFG_ERROR_ALREADY_EXISTS          -19
#define SPFG_ERROR_INVALID_FN_CYCLE        -20

// ----------------------------------------------------------------------------
// Storage sizes
// ----------------------------------------------------------------------------

#define SPFG_MAX_FN_IN_DPS 2
#define SPFG_MAX_FN_OUT_DPS 1
#define SPFG_MAX_GRID_FNS 20
#define SPFG_MAX_GRID_DPS 60
#define SPFG_MAX_PHASES SPFG_MAX_GRID_FNS
#define SPFG_BLOCK_NAME_MAX_LENGTH 10

// Fixed value for small usage profile, should provide some margin for internal
// expansion without changes to the public ABI.
#define SPFG_RUNTIME_SIZE 1024 * 4

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------

typedef int8_t spfg_err_t;
typedef uint32_t spfg_dp_id_t;
typedef uint32_t spfg_fn_id_t;
typedef uint32_t spfg_phase_t;
typedef uint32_t spfg_ts_t;

typedef int32_t spfg_int_t;
typedef double spfg_real_t;
typedef bool spfg_boolean_t;

typedef struct word_s {
    char *str;
    uint16_t len;
} spfg_word_t;

typedef enum spfg_dp_type_e {
    SPFG_DP_INT  = 1,
    SPFG_DP_REAL = 2,
    SPFG_DP_BOOL = 3,
} spfg_dp_type_t;

typedef enum spfg_fn_type_e {
    SPFG_FN_INVERT_BOOL_RET_BOOL   = 1,
    SPFG_FN_AND_BOOL_BOOL_RET_BOOL = 2,
 } spfg_fn_type_t;

typedef spfg_dp_id_t spfg_gr_dp_cnt_t;
typedef spfg_fn_id_t spfg_gr_fn_cnt_t;
typedef spfg_phase_t spfg_gr_phase_cnt_t;
typedef uint8_t spfg_fn_dp_in_cnt_t;
typedef uint8_t spfg_fn_dp_out_cnt_t;

typedef struct spfg_info_s {
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t version_patch;
    spfg_fn_dp_in_cnt_t max_fn_in_dps;
    spfg_fn_dp_out_cnt_t max_fn_out_dps;
    spfg_gr_dp_cnt_t max_grid_fns;
    spfg_gr_fn_cnt_t max_grid_dps;
    spfg_gr_phase_cnt_t max_phases;
} spfg_info_t;

typedef struct spfg_runtime {
    uint8_t _[SPFG_RUNTIME_SIZE];
} spfg_runtime_t;

typedef spfg_err_t (*spfg_run_cb_t)(spfg_runtime_t *, spfg_fn_id_t, spfg_phase_t, void * /* udata */);

// -------------------------------------------------------------------------------------------------
// Grid Composition API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_rt_init(spfg_runtime_t *, const char * /* name */);
spfg_err_t spfg_rt_finish(spfg_runtime_t *);
spfg_err_t spfg_dp_create(spfg_runtime_t *, spfg_dp_type_t, const char * /* name */, spfg_dp_id_t *);
spfg_err_t spfg_dp_remove(spfg_runtime_t *, spfg_dp_id_t);
spfg_err_t spfg_fn_create(spfg_runtime_t *, spfg_fn_type_t, spfg_phase_t,
                             spfg_dp_id_t [] /* in_dp_ids */, uint8_t /* in_dp_ids_len */,
                             spfg_dp_id_t [] /* out_dp_ids */, uint8_t /* out_dp_ids_len */,
                             const char * /* name */,
                             spfg_fn_id_t *);
spfg_err_t spfg_fn_remove(spfg_runtime_t *, spfg_fn_id_t);

// -------------------------------------------------------------------------------------------------
// Grid Evaluation API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_rt_reset_cycle(spfg_runtime_t *);
spfg_err_t spfg_rt_run_cycle(spfg_runtime_t *, spfg_ts_t, spfg_run_cb_t, void * /* cdata */);
spfg_err_t spfg_dp_set_int(spfg_runtime_t *, spfg_dp_id_t, spfg_int_t);
spfg_err_t spfg_dp_set_real(spfg_runtime_t *, spfg_dp_id_t, spfg_real_t);
spfg_err_t spfg_dp_set_word(spfg_runtime_t *, spfg_dp_id_t, spfg_word_t);
spfg_err_t spfg_dp_set_bool(spfg_runtime_t *, spfg_dp_id_t, spfg_boolean_t);
spfg_err_t spfg_dp_get_bool(spfg_runtime_t *, spfg_dp_id_t, spfg_boolean_t * /* value */, spfg_boolean_t * /* emitted */);

// -------------------------------------------------------------------------------------------------
// Import / Export API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_rt_import_bin(spfg_runtime_t *, void *, uint32_t);
spfg_err_t spfg_rt_export_bin(spfg_runtime_t *, void *, uint32_t);
spfg_err_t spfg_rt_import_json(spfg_runtime_t *, char *, uint32_t);
spfg_err_t spfg_rt_export_json(spfg_runtime_t *, char *, uint32_t /* maxlen */, uint32_t * /* outlen */);

// -------------------------------------------------------------------------------------------------
// Inspection API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_info(spfg_info_t *);
spfg_err_t spfg_runtime_size(uint32_t *size);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_H__
