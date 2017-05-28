#ifndef __SPFG_H__

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Error and Info return codes
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// Storage sizes
// ----------------------------------------------------------------------------

#define SPFG_GR_TOTAL 1

#define SPFG_MAX_FN_IN_DPS 2
#define SPFG_MAX_FN_OUT_DPS 2
#define SPFG_MAX_GRID_FNS 256
#define SPFG_MAX_GRID_DPS 512
#define SPFG_MAX_PHASES SPFG_MAX_GRID_FNS

#define SPFG_BLOCK_NAME_MAX_LENGTH 20

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------

typedef unsigned int spfg_ts_t;
typedef signed char spfg_err_t;
typedef unsigned char spfg_gr_id_t;
typedef unsigned int spfg_dp_id_t;
typedef unsigned int spfg_fn_id_t;
typedef unsigned short spfg_phase_t;
typedef unsigned short spfg_step_t;

typedef int spfg_int_t;
typedef double spfg_real_t;
typedef char spfg_boolean_t;

typedef struct word {
    char *str;
    size_t len;
} spfg_word_t;

typedef enum spfg_dp_type_e {
    SPFG_DP_INT,
    SPFG_DP_REAL,
    SPFG_DP_BOOL,
} spfg_dp_type_t;

typedef enum spfg_fn_type_e {
    SPFG_FN_INVERT_BOOL_RET_BOOL,
    SPFG_FN_AND_BOOL_BOOL_RET_BOOL,
 } spfg_fn_type_t;



typedef struct spfg_block_name {
    char chars[SPFG_BLOCK_NAME_MAX_LENGTH];
    size_t len;
} spfg_block_name_t;


typedef struct spfg_dp {
    spfg_dp_id_t id;
    spfg_block_name_t name;
    spfg_boolean_t emitted;
    union value_u {
        spfg_real_t real;
        spfg_int_t integer;
        spfg_boolean_t boolean;
    } value;
} spfg_dp_t;

typedef struct spfg_fn {
    spfg_fn_id_t id;
    spfg_block_name_t name;
    spfg_phase_t phase;
    spfg_fn_type_t type;
    spfg_dp_t *in_dps[SPFG_MAX_FN_IN_DPS];
    spfg_dp_t *out_dps[SPFG_MAX_FN_OUT_DPS];
} spfg_fn_t;

typedef struct spfg_gr_ctl {
    spfg_phase_t curr_phase;
    spfg_step_t curr_fn_step;
}  spfg_gr_ctl_t;

typedef struct spfg_gr {
    spfg_gr_id_t id;
    spfg_block_name_t name;
    spfg_dp_t dps[SPFG_MAX_GRID_DPS];
    spfg_fn_t fns[SPFG_MAX_GRID_FNS];
    spfg_gr_ctl_t ctl;
} spfg_gr_t;


// ----------------------------------------------------------------------------
// Utility API
// ----------------------------------------------------------------------------

spfg_err_t spfg_block_name_create(const char *ascii, spfg_block_name_t *name);

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

spfg_err_t spfg_dp_create(spfg_gr_id_t grid_id, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id);
spfg_err_t spfg_dp_remove(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id);
spfg_err_t spfg_dp_set_int(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id, int value);
spfg_err_t spfg_dp_set_real(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id, spfg_real_t value);
spfg_err_t spfg_dp_set_bool(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id, spfg_boolean_t value);
spfg_err_t spfg_dp_get_bool(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted);
spfg_err_t spfg_dp_set_word(spfg_gr_id_t grid_id, spfg_dp_id_t dp_id, spfg_word_t word);

spfg_err_t spfg_fn_create(spfg_gr_id_t grid_id,
                          spfg_fn_type_t type,
                          spfg_phase_t phase,
                          spfg_dp_id_t *in_dps, size_t in_dps_len,
                          spfg_dp_id_t *out_dps, size_t out_dps_len,
                          const char *name,
                          spfg_fn_id_t *fn_id);
spfg_err_t spfg_fn_remove(spfg_gr_id_t grid_id, spfg_fn_id_t fn_id);

spfg_err_t spfg_run_cycle(spfg_gr_id_t gr_id, spfg_ts_t ts);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_H__