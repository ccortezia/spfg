#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"

#define SPFG_GR_ID0 1
#define GEN_SPFG_DP_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_DPS + idx)
#define GEN_SPFG_FN_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_FNS + idx)

static spfg_gr_t global_grs[SPFG_GR_TOTAL];

static char initialized;

// ----------------------------------------------------------------------------
// Utility API
// ----------------------------------------------------------------------------

extern spfg_err_t spfg_block_name_create(const char *ascii, spfg_block_name_t *name)
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

    strncpy(name->chars, ascii, SPFG_BLOCK_NAME_MAX_LENGTH);
    name->chars[SPFG_BLOCK_NAME_MAX_LENGTH - 1] = 0;
    name->len = strnlen(name->chars, SPFG_BLOCK_NAME_MAX_LENGTH);
    return SPFG_ERROR_NO;
}

static spfg_err_t find_gr_by_id(spfg_gr_id_t gr_id, unsigned int *idx)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_GR_TOTAL; i++) {
        if (global_grs[i].id == gr_id) {
            *idx = i;
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t resolve_gr_from_id(spfg_gr_id_t gr_id, spfg_gr_t **gr)
{
    spfg_err_t err;
    unsigned int idx;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = find_gr_by_id(gr_id, &idx)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to resolve gr from id: err=[%04X]\n", err);
        return SPFG_ERROR_NOT_FOUND;
    }

    *gr = &global_grs[idx];
    return SPFG_ERROR_NO;
}


static spfg_err_t resolve_dp_from_id(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp)
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

static spfg_err_t find_free_gr(unsigned int *idx)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_GR_TOTAL; i++) {

        if (!global_grs[i].name.chars[0]) {
            *idx = i;
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_free_dp(spfg_gr_t *gr, unsigned int *idx)
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
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_free_fn(spfg_gr_t *gr, unsigned int *idx)
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
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t find_changed_input_for_fn(spfg_fn_t *fn, unsigned int *idx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS; i++) {

        if (!fn->in_dps[i]->name.chars[0]) {
            continue;
        }

        if (fn->in_dps[i]->emitted) {
            if (idx) {
                *idx = i;
            }
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

static spfg_err_t clear_changed_input_for_fn(spfg_fn_t *fn)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS; i++) {
        fn->in_dps[i]->emitted = 0;
    }

    return SPFG_ERROR_NO;
}


// ----------------------------------------------------------------------------
// Initialization API
// ----------------------------------------------------------------------------

extern spfg_err_t spfg_init()
{
    if (initialized) {
        return SPFG_ERROR_ALREADY_INITIALIZED;
    }

    // TODO: check if ID0's for internal data won't overflow runtime storage size for the id types.

    memset(global_grs, 0, sizeof(global_grs));

    initialized = 1;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_finish()
{
    if (!initialized) {
        return SPFG_ERROR_NOT_INITIALIZED;
    }

    memset(global_grs, 0, sizeof(global_grs));

    initialized = 0;

    return SPFG_ERROR_NO;
}

// ----------------------------------------------------------------------------
// Grid Composition API
// ----------------------------------------------------------------------------

extern spfg_err_t spfg_gr_create(spfg_gr_id_t *gr_id, const char *name)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    unsigned int gr_idx;

    if ((err = find_free_gr(&gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    spfg_gr_t *gr = &global_grs[gr_idx];

    if ((err = spfg_block_name_create(name, &gr->name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set grid name: err=[%04X]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    *gr_id = gr_idx + SPFG_GR_ID0;
    gr->id = *gr_id;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_gr_remove(spfg_gr_id_t gr_id)
{
    return SPFG_ERROR_NO;
}

// ---

extern spfg_err_t spfg_dp_create(spfg_gr_id_t gr_id, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    unsigned int gr_idx;

    if ((err = find_gr_by_id(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    unsigned int dp_idx;

    if ((err = find_free_dp(&global_grs[gr_idx], &dp_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    if ((err = spfg_block_name_create(name, &global_grs[gr_idx].dps[dp_idx].name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set datapoint name: err=[%04X]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    *dp_id = GEN_SPFG_DP_ID(gr_id, dp_idx);
    global_grs[gr_idx].dps[dp_idx].id = *dp_id;

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_dp_remove(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id)
{
    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_dp_set_int(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, int value)
{
    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_dp_set_real(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_real_t value)
{
    return SPFG_ERROR_NO;
}


static spfg_err_t _spfg_dp_set_bool(spfg_dp_t *dp, spfg_boolean_t value)
{
    dp->emitted = dp->value.boolean != value;
    dp->value.boolean = value;
    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_dp_set_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t value)
{
    spfg_err_t err;
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if ((err = resolve_gr_from_id(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_dp_from_id(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    return _spfg_dp_set_bool(dp, value);
}

extern spfg_err_t spfg_dp_get_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted)
{
    spfg_err_t err;
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if (!value) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = resolve_gr_from_id(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_dp_from_id(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
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
    return SPFG_ERROR_NO;
}

// ---

extern spfg_err_t spfg_fn_create(spfg_gr_id_t gr_id,
                                 spfg_fn_type_t type,
                                 spfg_phase_t phase,
                                 spfg_dp_id_t *in_dps, size_t in_dps_len,
                                 spfg_dp_id_t *out_dps, size_t out_dps_len,
                                 const char *name,
                                 spfg_fn_id_t *fn_id)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    unsigned int gr_idx;

    if ((err = find_gr_by_id(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    unsigned int fn_idx;

    spfg_gr_t *gr = &global_grs[gr_idx];

    if ((err = find_free_fn(gr, &fn_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_OUT_OF_SLOTS;
    }

    spfg_fn_t *fn = &gr->fns[fn_idx];

    if ((err = spfg_block_name_create(name, &fn->name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set function name: err=[%04X]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    *fn_id = GEN_SPFG_FN_ID(gr_id, fn_idx);

    fn->id = *fn_id;
    fn->type = type;
    fn->phase = phase;

    spfg_dp_t *tmp_dp;

    for (int i = 0; i < in_dps_len; i++) {
        if ((err = resolve_dp_from_id(gr, in_dps[i], &tmp_dp)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
        fn->in_dps[i] = tmp_dp;
    }

    for (int i = 0; i < out_dps_len; i++) {
        if ((err = resolve_dp_from_id(gr, out_dps[i], &tmp_dp)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
        fn->out_dps[i] = tmp_dp;
    }

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_fn_remove(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id)
{
    return SPFG_ERROR_NO;
}


// ----------------------------------------------------------------------------
// Grid Evaluation API
// ----------------------------------------------------------------------------

static spfg_err_t impl_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_fn_and_bool_bool_ret_bool(spfg_fn_t *fn, spfg_ts_t ts)
{
    spfg_boolean_t out;
    spfg_err_t err;

    if ((err = impl_and_bool_bool_ret_bool(
        fn->in_dps[0]->value.boolean,
        fn->in_dps[1]->value.boolean,
        &out)) != SPFG_ERROR_NO) {
        return err;
    }

    if ((err = _spfg_dp_set_bool(fn->out_dps[0], out)) != SPFG_ERROR_NO) {
        return err;
    }

    return SPFG_ERROR_NO;
}

// ---


static spfg_err_t run_fn(spfg_fn_t *fn, spfg_ts_t ts) {

    switch (fn->type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            return spfg_fn_and_bool_bool_ret_bool(fn, ts);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

static spfg_err_t spfg_run_gr_fn_step(spfg_gr_t *gr, spfg_fn_t *fn, spfg_ts_t ts)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = find_changed_input_for_fn(fn, NULL)) != SPFG_ERROR_NO) {
        if (err == SPFG_ERROR_NOT_FOUND) {
            return SPFG_ERROR_NO;
        }
        fprintf(stderr, "failed to find fn input change for fn %s on grid %d: err=[%04X]\n", fn->name.chars, gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    if ((err = run_fn(fn, ts)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to run fn %s on grid %d: err=[%04X]\n", fn->name.chars, gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    if ((err = clear_changed_input_for_fn(fn)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to clear fn input emitted flag for fn %s on grid %d: err=[%04X]\n", fn->name.chars, gr->id, err);
        return SPFG_ERROR_CYCLE_FAILURE;
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_run_gr_phase(spfg_gr_t *gr, spfg_ts_t ts, spfg_phase_t phase)
{
    spfg_err_t err = SPFG_ERROR_NO;

    for (gr->ctl.curr_fn_step = 0; gr->ctl.curr_fn_step < SPFG_MAX_GRID_FNS; gr->ctl.curr_fn_step++) {

        spfg_fn_t *fn = &gr->fns[gr->ctl.curr_fn_step];

        if (!fn->name.chars[0]) {
            continue;
        }

        if (fn->phase != phase) {
            continue;
        }

        if ((err = spfg_run_gr_fn_step(gr, fn, ts)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to run fn %s on grid %d: err=[%04X]\n", fn->name.chars, gr->id, err);
            return SPFG_ERROR_CYCLE_FAILURE;
        }
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_run_gr_cycle(spfg_gr_t *gr, spfg_ts_t ts)
{
    spfg_err_t err = SPFG_ERROR_NO;

    for (int i = 0; i < SPFG_MAX_PHASES; i++) {

        if ((err = spfg_run_gr_phase(gr, ts, gr->ctl.curr_phase)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to run phase %d on grid %d: err=[%04X]\n", gr->ctl.curr_phase, gr->id, err);
            return SPFG_ERROR_CYCLE_FAILURE;
        }

        gr->ctl.curr_phase += 1;
    }

    gr->ctl.curr_phase = 0;
    gr->ctl.curr_fn_step = 0;
    return SPFG_ERROR_NO;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_run_cycle(spfg_gr_id_t gr_id, spfg_ts_t ts)
{
    unsigned int gr_idx;
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = find_gr_by_id(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return spfg_run_gr_cycle(&global_grs[gr_idx], ts);
}
