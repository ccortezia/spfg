#ifndef __SPFG_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Error and Info return codes
// ----------------------------------------------------------------------------

#define SPFG_LOOP_CONTROL_STOP              1
#define SPFG_ERROR_NO                       0
#define SPFG_ERROR_BAD_PARAM_NULL_POINTER  -1
#define SPFG_ERROR_BAD_PARAM_INVALID_VALUE -2
#define SPFG_ERROR_ALREADY_INITIALIZED     -3
#define SPFG_ERROR_NOT_INITIALIZED         -4
#define SPFG_ERROR_OUT_OF_SLOTS            -5
#define SPFG_ERROR_BAD_BLOCK_NAME          -6
#define SPFG_ERROR_NOT_FOUND               -7
#define SPFG_ERROR_INVALID_GR_ID           -8
#define SPFG_ERROR_INVALID_DP_ID           -9
#define SPFG_ERROR_INVALID_FN_ID           -10
#define SPFG_ERROR_CYCLE_FAILURE           -11
#define SPFG_ERROR_UNIMPLEMENTED           -12
#define SPFG_ERROR_REINDEX_FN              -13
#define SPFG_ERROR_VALIDATE_FN             -14
#define SPFG_ERROR_BUFFER_OVERFLOW         -15
#define SPFG_ERROR_FN_INTEGRITY            -16

// ----------------------------------------------------------------------------
// Storage sizes
// ----------------------------------------------------------------------------

#define SPFG_GR_TOTAL 1

#define SPFG_MAX_FN_IN_DPS 3
#define SPFG_MAX_FN_OUT_DPS 3
#define SPFG_MAX_GRID_FNS 64
#define SPFG_MAX_GRID_DPS 256
#define SPFG_MAX_PHASES SPFG_MAX_GRID_FNS

#define SPFG_BLOCK_NAME_MAX_LENGTH 20

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------

typedef signed char spfg_err_t;
typedef unsigned char spfg_gr_id_t;
typedef unsigned int spfg_dp_id_t;
typedef unsigned int spfg_fn_id_t;
typedef unsigned int spfg_ts_t;
typedef unsigned short spfg_phase_t;

typedef int spfg_int_t;
typedef double spfg_real_t;
typedef char spfg_boolean_t;

typedef struct word {
    char *str;
    size_t len;
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


typedef spfg_err_t (*spfg_cycle_cb_t)(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id, spfg_phase_t phase, void *cdata);

// ----------------------------------------------------------------------------
// Initialization API
// ----------------------------------------------------------------------------

spfg_err_t spfg_init();
spfg_err_t spfg_finish();

// ----------------------------------------------------------------------------
// Grid Composition API
// ----------------------------------------------------------------------------

spfg_err_t spfg_gr_create(spfg_gr_id_t *id, const char *name);
spfg_err_t spfg_gr_remove(spfg_gr_id_t id);

spfg_err_t spfg_dp_create(spfg_gr_id_t gr_id, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id);
spfg_err_t spfg_dp_remove(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id);
spfg_err_t spfg_dp_set_int(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, int value);
spfg_err_t spfg_dp_set_real(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_real_t value);
spfg_err_t spfg_dp_set_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t value);
spfg_err_t spfg_dp_get_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted);
spfg_err_t spfg_dp_set_word(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_word_t word);

spfg_err_t spfg_fn_create(spfg_gr_id_t gr_id,
                          spfg_fn_type_t type,
                          spfg_phase_t phase,
                          spfg_dp_id_t *in_dp_ids, size_t in_dp_ids_len,
                          spfg_dp_id_t *out_dp_ids, size_t out_dp_ids_len,
                          const char *name,
                          spfg_fn_id_t *fn_id);
spfg_err_t spfg_fn_remove(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id);

// ----------------------------------------------------------------------------
// Grid Evaluation API
// ----------------------------------------------------------------------------

spfg_err_t spfg_reset_cycle(spfg_gr_id_t gr_id);
spfg_err_t spfg_run_cycle(spfg_gr_id_t gr_id, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata);

// ----------------------------------------------------------------------------
// Import / Export API
// ----------------------------------------------------------------------------

spfg_err_t spfg_gr_export_schema(spfg_gr_id_t gr_id, void *outbuf, size_t outbuf_len);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_H__
