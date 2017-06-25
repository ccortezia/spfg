#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"

#define SPFG_GR_ID0 1
#define GEN_SPFG_DP_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_DPS + idx + 1)
#define GEN_SPFG_FN_ID(gr_id, idx) ((gr_id - SPFG_GR_ID0) * SPFG_MAX_GRID_FNS + idx + 1)

static spfg_gr_t global_grs[SPFG_GR_TOTAL];
static spfg_grx_t global_grxs[SPFG_GR_TOTAL];

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

static spfg_err_t find_global_gr(spfg_gr_id_t gr_id, unsigned int *idx)
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

static spfg_err_t find_free_global_gr(unsigned int *idx, spfg_gr_t **gr)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_GR_TOTAL; i++) {

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
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS; i++) {

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
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS; i++) {
        fnx->in_dps[i]->emitted = 0;
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
    *gr_id = gr_idx + SPFG_GR_ID0;
    gr->id = *gr_id;

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_gr_remove(spfg_gr_id_t gr_id)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}

// ---

extern spfg_err_t spfg_dp_gr_create(spfg_gr_t *gr, unsigned int dp_idx, spfg_dp_type_t dp_type, const char *name)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = spfg_block_name_create(name, &gr->dps[dp_idx].name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set datapoint name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    gr->dps[dp_idx].id = GEN_SPFG_DP_ID(gr->id, dp_idx);
    gr->dps[dp_idx].type = dp_type;

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
    return SPFG_ERROR_UNIMPLEMENTED;
}


extern spfg_err_t spfg_dp_set_int(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, int value)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}


extern spfg_err_t spfg_dp_set_real(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_real_t value)
{
    return SPFG_ERROR_UNIMPLEMENTED;
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

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if ((err = resolve_gr_dp(gr, dp_id, &dp)) != SPFG_ERROR_NO) {
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

static spfg_err_t spfg_fn_reindex(spfg_grx_t *grx, spfg_fnx_t *fnx)
{
    // TODO: ensure grx->fnx is still sorted by fnx->fn->phase + fnx->fn->id.

    spfg_err_t err;
    spfg_dp_t *tmp_dp;

    memset(fnx->in_dps, 0, SPFG_MAX_FN_IN_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; fnx->fn->in_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->in_dp_ids[i], &tmp_dp)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
        fnx->in_dps[i] = tmp_dp;
    }

    memset(fnx->out_dps, 0, SPFG_MAX_FN_OUT_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; fnx->fn->out_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->out_dp_ids[i], &tmp_dp)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
        fnx->out_dps[i] = tmp_dp;
    }

    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_fn_gr_create(spfg_gr_t *gr, int fn_idx,
                                    spfg_fn_type_t type,
                                    spfg_phase_t phase,
                                    spfg_dp_id_t *in_dp_ids, size_t in_dp_ids_len,
                                    spfg_dp_id_t *out_dp_ids, size_t out_dp_ids_len,
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


extern spfg_err_t spfg_fn_create(spfg_gr_id_t gr_id,
                                 spfg_fn_type_t type,
                                 spfg_phase_t phase,
                                 spfg_dp_id_t *in_dp_ids, size_t in_dp_ids_len,
                                 spfg_dp_id_t *out_dp_ids, size_t out_dp_ids_len,
                                 const char *name,
                                 spfg_fn_id_t *fn_id)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    // Resolve input id into local memory references.

    unsigned int gr_idx;

    if ((err = find_global_gr(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_gr_t *gr = &global_grs[gr_idx];
    spfg_grx_t *grx = &global_grxs[gr_idx];

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

    spfg_fnx_t *fnx = &grx->fnx[fn_idx];
    fnx->fn = fn;

    if ((err = spfg_fn_reindex(grx, fnx)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to reindex function: err=[%d]\n", err);
        return SPFG_ERROR_REINDEX_FN;
    }

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_fn_remove(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id)
{
    return SPFG_ERROR_UNIMPLEMENTED;
}


// ----------------------------------------------------------------------------
// Grid Evaluation API
// ----------------------------------------------------------------------------

static spfg_err_t impl_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

static spfg_err_t spfg_fn_and_bool_bool_ret_bool(spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_boolean_t out;
    spfg_err_t err;

    if ((err = impl_and_bool_bool_ret_bool(
        fnx->in_dps[0]->value.boolean,
        fnx->in_dps[1]->value.boolean,
        &out)) != SPFG_ERROR_NO) {
        return err;
    }

    if ((err = _spfg_dp_set_bool(fnx->out_dps[0], out)) != SPFG_ERROR_NO) {
        return err;
    }

    return SPFG_ERROR_NO;
}

// ---


static spfg_err_t eval_fnx(spfg_fnx_t *fnx, spfg_ts_t ts) {

    switch (fnx->fn->type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            return spfg_fn_and_bool_bool_ret_bool(fnx, ts);
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

        if (fnx->fn->phase != grx->gr->ctl.curr_phase) {
            grx->gr->ctl.curr_phase += 1;
        }

        if ((err = spfg_run_fnx(grx, fnx, ts)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to run fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
            return SPFG_ERROR_CYCLE_FAILURE;
        }

        grx->gr->ctl.curr_fnx_idx += 1;
    }

    return SPFG_ERROR_NO;
}


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


// ----------------------------------------------------------------------------
// Import / Export API
// ----------------------------------------------------------------------------

spfg_err_t spfg_gr_export_schema(spfg_gr_id_t gr_id, void *outbuf, size_t outbuf_len)
{
    spfg_err_t err;

    if (!outbuf) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (outbuf_len < sizeof(spfg_grxp_t)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    unsigned int gr_idx;

    if ((err = find_global_gr(gr_id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_gr_t *gr = &global_grs[gr_idx];

    spfg_grxp_t *grxp = (spfg_grxp_t *)outbuf;

    for (int dp_idx = 0; dp_idx < SPFG_MAX_GRID_DPS; dp_idx++) {

        if (!gr->dps[dp_idx].name.chars[0]) {
            continue;
        }

        if ((err = spfg_dp_gr_create(&grxp->data, dp_idx,
                                   gr->dps[dp_idx].type,
                                   gr->dps[dp_idx].name.chars)) != SPFG_ERROR_NO) {
            return err;
        }
    }

    for (int fn_idx = 0; fn_idx < SPFG_MAX_GRID_FNS; fn_idx++) {

        if (!gr->fns[fn_idx].name.chars[0]) {
            continue;
        }

        if ((err = spfg_fn_gr_create(&grxp->data, fn_idx,
                                   gr->fns[fn_idx].type,
                                   gr->fns[fn_idx].phase,
                                   gr->fns[fn_idx].in_dp_ids,
                                   gr->fns[fn_idx].in_dp_ids_len,
                                   gr->fns[fn_idx].out_dp_ids,
                                   gr->fns[fn_idx].out_dp_ids_len,
                                   gr->fns[fn_idx].name.chars)) != SPFG_ERROR_NO) {
            return err;
        }
    }

    return SPFG_ERROR_NO;
}
