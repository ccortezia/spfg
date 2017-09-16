#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"

// -------------------------------------------------------------------------------------------------
// Local macros
// -------------------------------------------------------------------------------------------------

#define SPFG_GR_ID0 1
#define GEN_SPFG_DP_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_DPS + idx + 1)
#define GEN_SPFG_FN_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_FNS + idx + 1)

// -------------------------------------------------------------------------------------------------
// Private Type Definitions
// -------------------------------------------------------------------------------------------------


typedef struct spfg_block_name {
    char chars[SPFG_BLOCK_NAME_MAX_LENGTH];
} spfg_block_name_t;

typedef struct spfg_dp {
    spfg_dp_id_t id;
    spfg_dp_type_t type;
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
    spfg_fn_type_t type;
    spfg_block_name_t name;
    spfg_phase_t phase;
    spfg_dp_id_t in_dp_ids[SPFG_MAX_FN_IN_DPS];
    uint8_t in_dp_ids_len;
    spfg_dp_id_t out_dp_ids[SPFG_MAX_FN_OUT_DPS];
    uint8_t out_dp_ids_len;
} spfg_fn_t;

typedef struct spfg_fnx {
    spfg_fn_t *fn;
    spfg_dp_t *in_dps[SPFG_MAX_FN_IN_DPS];
    spfg_dp_t *out_dps[SPFG_MAX_FN_OUT_DPS];
} spfg_fnx_t;

typedef struct spfg_gr_ctl {
    spfg_phase_t curr_phase;
    spfg_step_t curr_fnx_idx;
}  spfg_gr_ctl_t;

typedef struct spfg_gr {
    spfg_gr_id_t id;
    spfg_block_name_t name;
    spfg_dp_t dps[SPFG_MAX_GRID_DPS];
    spfg_fn_t fns[SPFG_MAX_GRID_FNS];
    spfg_gr_ctl_t ctl;
} spfg_gr_t;

typedef struct spfg_grx {
    spfg_gr_t *gr;
    spfg_fnx_t fnx[SPFG_MAX_GRID_FNS];
} spfg_grx_t;

typedef struct spfg_gr_exph {
} spfg_gr_exph_t;

typedef struct spfg_gr_exp {
    spfg_gr_exph_t header;
    spfg_gr_t data;
} spfg_gr_exp_t;


// -------------------------------------------------------------------------------------------------
// Private Library Data
// -------------------------------------------------------------------------------------------------

static spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
static spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

static char initialized;


// -------------------------------------------------------------------------------------------------
// Private utilities
// -------------------------------------------------------------------------------------------------

static spfg_err_t spfg_block_name_create(const char *ascii, spfg_block_name_t *name)
{
    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!ascii) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!*ascii) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if (strnlen(ascii, sizeof(name->chars)) >= sizeof(name->chars)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    memset(name->chars, 0, sizeof(name->chars));
    strcpy(name->chars, ascii);
    name->chars[sizeof(name->chars) - 1] = 0;
    return SPFG_ERROR_NO;
}

static spfg_err_t find_global_gr(spfg_gr_id_t gr_id, unsigned int *idx)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (gr_id == 0) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (int i = 0; i < SPFG_MAX_GRID_CNT; i++) {
        if (global_grs[i].id == gr_id) {
            *idx = i;
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t resolve_global_gr(spfg_gr_id_t gr_id, spfg_gr_t **gr)
{
    spfg_err_t err;
    unsigned int idx;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = find_global_gr(gr_id, &idx)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to resolve gr from id: err=[%d]\n", err);
        return SPFG_ERROR_NOT_FOUND;
    }

    *gr = &global_grs[idx];
    return SPFG_ERROR_NO;
}


static spfg_err_t resolve_gr_dp(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp)
{
    if (!dp) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_DPS; i++) {
        if (gr->dps[i].id == dp_id) {
            *dp = &gr->dps[i];
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t resolve_gr_fn(spfg_gr_t *gr, spfg_fn_id_t fn_id, spfg_fn_t **fn)
{
    if (!fn) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_FNS; i++) {
        if (gr->fns[i].id == fn_id) {
            *fn = &gr->fns[i];
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_free_global_gr(unsigned int *idx, spfg_gr_t **gr)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_CNT; i++) {

        if (!global_grs[i].name.chars[0]) {
            *idx = i;

            if (gr) {
                *gr = &global_grs[i];
            }

            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_free_gr_dp(spfg_gr_t *gr, unsigned int *idx, spfg_dp_t **dp)
{
    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_DPS; i++) {

        if (!gr->dps[i].name.chars[0]) {
            *idx = i;

            if (dp) {
                *dp = &gr->dps[i];
            }

            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_free_gr_fn(spfg_gr_t *gr, unsigned int *idx, spfg_fn_t **fn)
{
    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_FNS; i++) {

        if (!gr->fns[i].name.chars[0]) {
            *idx = i;

            if (fn) {
                *fn = &gr->fns[i];
            }

            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_changed_input_for_fnx(spfg_fnx_t *fnx, unsigned int *idx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {

        if (!fnx->in_dps[i]->name.chars[0]) {
            continue;
        }

        if (fnx->in_dps[i]->emitted) {
            if (idx) {
                *idx = i;
            }
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t clear_changed_input_for_fnx(spfg_fnx_t *fnx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {
        fnx->in_dps[i]->emitted = 0;
    }

    return SPFG_ERROR_NO;
}

// -------------------------------------------------------------------------------------------------
// Private Grid Composition API
// -------------------------------------------------------------------------------------------------

static spfg_err_t spfg_dp_gr_create(spfg_gr_t *gr, unsigned int dp_idx, spfg_dp_type_t dp_type, const char *name)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    // TODO: dp type validation

    if ((err = spfg_block_name_create(name, &gr->dps[dp_idx].name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set datapoint name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    gr->dps[dp_idx].id = GEN_SPFG_DP_ID(gr->id, dp_idx);
    gr->dps[dp_idx].type = dp_type;

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_fn_reindex(spfg_grx_t *grx, spfg_fnx_t *fnx)
{
    // TODO: ensure grx->fnx is still sorted by fnx->fn->phase + fnx->fn->id.

    spfg_err_t err;

    memset(fnx->in_dps, 0, SPFG_MAX_FN_IN_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->fn->in_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->in_dp_ids[i], &fnx->in_dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    memset(fnx->out_dps, 0, SPFG_MAX_FN_OUT_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; i < SPFG_MAX_FN_OUT_DPS && fnx->fn->out_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->out_dp_ids[i], &fnx->out_dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_resolve_dp_ids(spfg_gr_t *gr, spfg_dp_id_t *dp_ids, spfg_dp_t *dps[], uint8_t length)
{
    spfg_err_t err;

    for (int i = 0; dp_ids[i] && i < length; i++) {
        if ((err = resolve_gr_dp(gr, dp_ids[i], &dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t validate_fn_signature(spfg_dp_t *in_dps[], uint8_t in_dps_len,
                                        spfg_dp_t *out_dps[], uint8_t out_dps_len,
                                        spfg_dp_type_t *in_dp_types, uint8_t in_dp_types_len,
                                        spfg_dp_type_t *out_dp_types, uint8_t out_dp_types_len,
                                        const char *fn_name) {

    uint8_t cnt = 0;

    if (in_dps_len < in_dp_types_len) {
        fprintf(stderr, "%d missing input datapoints for function '%s' expecting %d input datapoints\n", in_dp_types_len - in_dps_len, fn_name, in_dp_types_len);
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (cnt = 0; cnt < in_dps_len && cnt < SPFG_MAX_FN_IN_DPS; cnt++) {

        if (cnt >= in_dp_types_len) {
            fprintf(stderr, "unexpected input datapoint '%s' (position %d) for function '%s' expecting only %d input datapoints\n", in_dps[cnt]->name.chars, cnt, fn_name, in_dp_types_len);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }

        if (in_dps[cnt]->type != in_dp_types[cnt]) {
            fprintf(stderr, "input datapoint '%s' type %d is incompatible with expected input type %d (position %d) for function '%s'\n", in_dps[cnt]->name.chars, in_dps[cnt]->type, in_dp_types[cnt], cnt, fn_name);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }

    if (out_dps_len < out_dp_types_len) {
        fprintf(stderr, "%d missing output datapoints for function '%s' expecting %d output datapoints\n", out_dp_types_len - out_dps_len, fn_name, out_dp_types_len);
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (cnt = 0; cnt < out_dps_len && cnt < SPFG_MAX_FN_OUT_DPS; cnt++) {

        if (cnt >= out_dp_types_len) {
            fprintf(stderr, "unexpected output datapoint '%s' (position %d) for function '%s' expecting only %d output datapoints\n", out_dps[cnt]->name.chars, cnt, fn_name, out_dp_types_len);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }

        if (out_dps[cnt]->type != out_dp_types[cnt]) {
            fprintf(stderr, "output datapoint '%s' type %d is incompatible with expected output type %d (position %d) for function '%s'\n", out_dps[cnt]->name.chars, out_dps[cnt]->type, out_dp_types[cnt], cnt, fn_name);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_fn_validate(spfg_fn_type_t type,
                                   spfg_dp_t *in_dps[], uint8_t in_dps_len,
                                   spfg_dp_t *out_dps[], uint8_t out_dps_len,
                                   const char *fn_name) {

    switch (type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            spfg_dp_type_t in_dp_types[] = {SPFG_DP_BOOL, SPFG_DP_BOOL};
            spfg_dp_type_t out_dp_types[] = {SPFG_DP_BOOL};
            return validate_fn_signature(
                in_dps, in_dps_len,
                out_dps, out_dps_len,
                in_dp_types, 2,
                out_dp_types, 1,
                fn_name);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

static spfg_err_t spfg_fn_gr_create(spfg_gr_t *gr, int fn_idx,
                                    spfg_fn_type_t type,
                                    spfg_phase_t phase,
                                    spfg_dp_id_t *in_dp_ids, uint8_t in_dp_ids_len,
                                    spfg_dp_id_t *out_dp_ids, uint8_t out_dp_ids_len,
                                    const char *name)
{
    spfg_err_t err;
    spfg_fn_t *fn = &gr->fns[fn_idx];

    if ((err = spfg_block_name_create(name, &fn->name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set function name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    fn->type = type;
    fn->phase = phase;
    fn->id = GEN_SPFG_FN_ID(gr->id, fn_idx);
    memcpy(fn->in_dp_ids, in_dp_ids, in_dp_ids_len * sizeof(spfg_dp_id_t));
    fn->in_dp_ids_len = in_dp_ids_len;
    memcpy(fn->out_dp_ids, out_dp_ids, out_dp_ids_len * sizeof(spfg_dp_id_t));
    fn->out_dp_ids_len = out_dp_ids_len;

    return SPFG_ERROR_NO;
}


// -------------------------------------------------------------------------------------------------
// Private Grid Evaluation API
// -------------------------------------------------------------------------------------------------

static spfg_err_t impl_dp_set_bool(spfg_dp_t *dp, spfg_boolean_t value)
{
    dp->emitted = dp->value.boolean != value;
    dp->value.boolean = value;
    return SPFG_ERROR_NO;
}

static spfg_err_t impl_fn_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

static spfg_err_t eval_fn_and_bool_bool_ret_bool(spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_boolean_t out;
    spfg_err_t err;

    if ((err = impl_fn_and_bool_bool_ret_bool(
        fnx->in_dps[0]->value.boolean,
        fnx->in_dps[1]->value.boolean,
        &out)) != SPFG_ERROR_NO) {
        return err;
    }

    if ((err = impl_dp_set_bool(fnx->out_dps[0], out)) != SPFG_ERROR_NO) {
        return err;
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t eval_fnx(spfg_fnx_t *fnx, spfg_ts_t ts) {

    switch (fnx->fn->type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            return eval_fn_and_bool_bool_ret_bool(fnx, ts);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

static spfg_err_t spfg_run_fnx(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = find_changed_input_for_fnx(fnx, NULL)) != SPFG_ERROR_NO) {
        if (err == SPFG_ERROR_NOT_FOUND) {
            return SPFG_ERROR_NO;
        }
        fprintf(stderr, "failed to find fn input change for fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    if ((err = eval_fnx(fnx, ts)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to run fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    if ((err = clear_changed_input_for_fnx(fnx)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to clear fn input emitted flag for fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_resume_cycle_grx(spfg_grx_t *grx, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata)
{
    spfg_err_t err = SPFG_ERROR_NO;
    spfg_fnx_t *fnx;

    for (;;) {

        // Stop condition: array boundary protection.
        if (grx->gr->ctl.curr_fnx_idx >= SPFG_MAX_GRID_FNS) {
            break;
        }

        fnx = &grx->fnx[grx->gr->ctl.curr_fnx_idx];

        // Stop condition: no more pending functions to evaluate.
        if (!fnx->fn) {
            break;
        }

        // Stop condition: no more pending functions to evaluate.
        if (!fnx->fn->name.chars[0]) {
            break;
        }

        if (fnx->fn->phase != grx->gr->ctl.curr_phase) {
            grx->gr->ctl.curr_phase += 1;
        }

        // Stop condition: control callback.
        if (cb) {
            err = cb(grx->gr->id, fnx->fn->id, grx->gr->ctl.curr_phase, udata);

            if (err == SPFG_LOOP_CONTROL_STOP) {
                break;
            }

            if (err != SPFG_ERROR_NO) {
                fprintf(stderr, "failed to run cb on grid %d: err=[%d]\n", grx->gr->id, err);
                return SPFG_ERROR_CYCLE_FAILURE;
            }
        }

        if ((err = spfg_run_fnx(grx, fnx, ts)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to run fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
            return SPFG_ERROR_CYCLE_FAILURE;
        }

        grx->gr->ctl.curr_fnx_idx += 1;
    }

    return SPFG_ERROR_NO;
}

// -------------------------------------------------------------------------------------------------
// Initialization API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_init()
{
    if (initialized) {
        return SPFG_ERROR_ALREADY_INITIALIZED;
    }

    // TODO: check if ID0's for internal data won't overflow runtime storage size for the id types.

    memset(global_grs, 0, sizeof(global_grs));
    memset(global_grxs, 0, sizeof(global_grxs));

    initialized = 1;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_finish()
{
    if (!initialized) {
        return SPFG_ERROR_NOT_INITIALIZED;
    }

    memset(global_grs, 0, sizeof(global_grs));
    memset(global_grxs, 0, sizeof(global_grxs));

    initialized = 0;

    return SPFG_ERROR_NO;
}

// -------------------------------------------------------------------------------------------------
// Public Grid Composition API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_gr_create(spfg_gr_id_t *gr_id, const char *name)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    spfg_gr_t *gr;
    unsigned int gr_idx;

    if ((err = find_free_global_gr(&gr_idx, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    if ((err = spfg_block_name_create(name, &gr->name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set grid name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    global_grxs[gr_idx].gr = gr;
    gr->id = gr_idx + SPFG_GR_ID0;

    *gr_id = gr->id;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_gr_remove(spfg_gr_id_t gr_id)
{
    spfg_err_t err;
    spfg_gr_t *gr;

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    memset(gr, 0, sizeof(spfg_gr_t));

    return SPFG_ERROR_NO;
}

spfg_err_t spfg_gr_get_ids(spfg_gr_id_t *output, spfg_gr_cnt_t maxlen, spfg_gr_cnt_t *count)
{
    if (!output) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!count) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    *count = 0;

    for (uint32_t idx = 0; idx < SPFG_MAX_GRID_CNT; idx++) {

        if (!global_grs[idx].name.chars[0]) {
            break;
        }

        if (*count + 1 > maxlen) {
            return SPFG_ERROR_BUFFER_OVERFLOW;
        }

        output[*count] = global_grs[idx].id;
        *count += 1;
    }

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_dp_create(spfg_gr_id_t gr_id, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!dp_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    spfg_gr_t *gr;

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    unsigned int dp_idx;
    spfg_dp_t *dp;

    if ((err = find_free_gr_dp(gr, &dp_idx, &dp)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    if ((err = spfg_dp_gr_create(gr, dp_idx, dp_type, name)) != SPFG_ERROR_NO) {
        return err;
    }

    *dp_id = dp->id;

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_dp_remove(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id)
{
    spfg_err_t err;
    spfg_gr_t *gr;
    spfg_dp_t *dp;

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    for (int i = 0; i < SPFG_MAX_GRID_FNS; i++) {
        if (!gr->fns[i].name.chars[0]) {
            continue;
        }
        for (int j = 0; j < SPFG_MAX_FN_IN_DPS; j++) {
            if (gr->fns[i].in_dp_ids[j] == dp_id) {
                return SPFG_ERROR_FN_INTEGRITY;
            }
        }
        for (int j = 0; j < SPFG_MAX_FN_OUT_DPS; j++) {
            if (gr->fns[i].out_dp_ids[j] == dp_id) {
                return SPFG_ERROR_FN_INTEGRITY;
            }
        }
    }

    if ((err = resolve_gr_dp(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    memset(dp, 0, sizeof(spfg_dp_t));

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_fn_create(spfg_gr_id_t gr_id,
                                 spfg_fn_type_t type,
                                 spfg_phase_t phase,
                                 spfg_dp_id_t *in_dp_ids, uint8_t in_dp_ids_len,
                                 spfg_dp_id_t *out_dp_ids, uint8_t out_dp_ids_len,
                                 const char *name,
                                 spfg_fn_id_t *fn_id)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (in_dp_ids_len > SPFG_MAX_FN_IN_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    if (out_dp_ids_len > SPFG_MAX_FN_OUT_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    // Resolve input id into local memory references.

    unsigned int gr_idx;

    if ((err = find_global_gr(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_gr_t *gr = &global_grs[gr_idx];

    // Validate datapoints against target function signature.

    spfg_dp_t *in_dps[SPFG_MAX_FN_IN_DPS];
    spfg_dp_t *out_dps[SPFG_MAX_FN_OUT_DPS];

    if ((err = spfg_resolve_dp_ids(gr, in_dp_ids, in_dps, in_dp_ids_len)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    if ((err = spfg_resolve_dp_ids(gr, out_dp_ids, out_dps, out_dp_ids_len)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    if ((err = spfg_fn_validate(type, in_dps, in_dp_ids_len, out_dps, out_dp_ids_len, name)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_VALIDATE_FN;
    }

    // Evolve grid schema (generic memory construction).

    unsigned int fn_idx;
    spfg_fn_t *fn;

    if ((err = find_free_gr_fn(gr, &fn_idx, &fn)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    if ((err = spfg_fn_gr_create(gr, fn_idx, type, phase,
                               in_dp_ids, in_dp_ids_len,
                               out_dp_ids, out_dp_ids_len,
                               name)) != SPFG_ERROR_NO) {
        return err;
    }

    // Review function indexing (specific to local memory construction).

    spfg_grx_t *grx = &global_grxs[gr_idx];
    spfg_fnx_t *fnx = &grx->fnx[fn_idx];
    fnx->fn = fn;

    if ((err = spfg_fn_reindex(grx, fnx)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to reindex function: err=[%d]\n", err);
        return SPFG_ERROR_REINDEX_FN;
    }

    *fn_id = fn->id;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_fn_remove(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id)
{
    spfg_err_t err;
    spfg_gr_t *gr;
    spfg_fn_t *fn;

    // TODO: study if reindex is necessary

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_gr_fn(gr, fn_id, &fn)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_FN_ID;
    }

    memset(fn, 0, sizeof(spfg_fn_t));

    return SPFG_ERROR_NO;
}


// -------------------------------------------------------------------------------------------------
// Public Grid Evaluation API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_dp_set_int(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_int_t value)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}


extern spfg_err_t spfg_dp_set_real(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_real_t value)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}

extern spfg_err_t spfg_dp_set_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t value)
{
    spfg_err_t err;
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_gr_dp(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    return impl_dp_set_bool(dp, value);
}

extern spfg_err_t spfg_dp_get_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted)
{
    spfg_err_t err;
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if (!value) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_gr_dp(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    *value = dp->value.boolean;

    if (emitted) {
        *emitted = dp->value.boolean;
    }

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_dp_set_word(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_word_t word)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}

// ---

extern spfg_err_t spfg_reset_cycle(spfg_gr_id_t gr_id)
{
    unsigned int gr_idx;
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = find_global_gr(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_grx_t *grx = &global_grxs[gr_idx];
    grx->gr->ctl.curr_phase = 0;
    grx->gr->ctl.curr_fnx_idx = 0;
    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_run_cycle(spfg_gr_id_t gr_id, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata)
{
    unsigned int gr_idx;
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = find_global_gr(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return spfg_resume_cycle_grx(&global_grxs[gr_idx], ts, cb, udata);
}


// -------------------------------------------------------------------------------------------------
// Import / Export API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_gr_export_bin(spfg_gr_id_t gr_id, void *outbuf, uint32_t outbuf_len)
{
    spfg_err_t err;

    if (!outbuf) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (outbuf_len < sizeof(spfg_gr_exp_t)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    spfg_gr_t *gr;

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)outbuf;
    memset(grxp, 0, sizeof(spfg_gr_exp_t));
    memcpy(&grxp->data, gr, sizeof(spfg_gr_t));

    return SPFG_ERROR_NO;
}

spfg_err_t spfg_gr_import_bin(void *data, uint32_t data_len, spfg_gr_id_t *gr_id, const char *name)
{
    spfg_err_t err;

    if (!data) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (data_len < sizeof(spfg_gr_exp_t)) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if ((err = spfg_gr_create(gr_id, name)) != SPFG_ERROR_NO) {
        return err;
    }

    spfg_gr_t *gr;

    (void) resolve_global_gr(*gr_id, &gr);

    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)data;

    memcpy(&gr->dps, &grxp->data.dps, sizeof(grxp->data.dps));
    memcpy(&gr->fns, &grxp->data.fns, sizeof(grxp->data.fns));
    memcpy(&gr->ctl, &grxp->data.ctl, sizeof(spfg_gr_ctl_t));

    return SPFG_ERROR_NO;
}

// -------------------------------------------------------------------------------------------------
// Inspection API
// -------------------------------------------------------------------------------------------------

spfg_err_t spfg_info(spfg_info_t *info)
{
    if (!info) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(info, 0, sizeof(spfg_info_t));

    info->version_major = VERSION_MAJOR;
    info->version_minor = VERSION_MINOR;
    info->version_patch = VERSION_PATCH;
    info->max_grid_cnt = SPFG_MAX_GRID_CNT;
    info->max_fn_in_dps = SPFG_MAX_FN_IN_DPS;
    info->max_fn_out_dps = SPFG_MAX_FN_OUT_DPS;
    info->max_grid_fns = SPFG_MAX_GRID_FNS;
    info->max_grid_dps = SPFG_MAX_GRID_DPS;
    info->max_phases = SPFG_MAX_PHASES;

    return SPFG_ERROR_NO;
}
