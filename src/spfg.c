#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_builder.h"

// -------------------------------------------------------------------------------------------------
// Private Library Data
// -------------------------------------------------------------------------------------------------

spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

static char initialized;

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
        if (grx->gr->ctl.curr_fn_idx >= SPFG_MAX_GRID_FNS) {
            break;
        }

        fnx = &grx->fnx[grx->gr->ctl.curr_fn_idx];

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

        grx->gr->ctl.curr_fn_idx += 1;
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
    grx->gr->ctl.curr_fn_idx = 0;
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

// -------------------------------------------------------------------------------------------------
// JSON Import/Export API
// -------------------------------------------------------------------------------------------------

#include "azjson/azjson.h"

azjson_err_t spfg_dp_value_cp(void *target, azjson_token_t *token) {
    switch (token->type) {
        case TOKEN_VI: return azjson_strtol(token->chars, &((spfg_dp_value_t *)target)->integer);
        case TOKEN_VR: return azjson_strtod(token->chars, &((spfg_dp_value_t *)target)->real);
        case TOKEN_VT: (((spfg_dp_value_t *)target)->boolean) = true; break;
        case TOKEN_VF: (((spfg_dp_value_t *)target)->boolean) = false; break;
        default: return AZJSON_ERROR_FAIL;
    }
    return AZJSON_ERROR_NO;
}


static azjson_spec_t int_spec[] = {
    {
        .vtype = JSON_INTEGER,
    },
    {.boundary = true}
};

static azjson_spec_t gr_fns_fn_spec[] = {
    {
        .key = "id",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, id),
    },
    {
        .key = "type",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, type),
    },
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_fn_t, name),
        .vsize = sizeof(((spfg_fn_t *)0)->name)
    },
    {
        .key = "phase",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, phase),
    },
    {
        .key = "in_dp_ids",
        .vtype = JSON_ARRAY,
        .vspec = int_spec,
        .voffset = offsetof(spfg_fn_t, in_dp_ids),
        .noffset = offsetof(spfg_fn_t, in_dp_ids_len),
        .maxitems = sizeof(((spfg_fn_t *)0)->in_dp_ids) / sizeof(spfg_dp_id_t)
    },
    {
        .key = "out_dp_ids",
        .vtype = JSON_ARRAY,
        .vspec = int_spec,
        .voffset = offsetof(spfg_fn_t, out_dp_ids),
        .noffset = offsetof(spfg_fn_t, out_dp_ids_len),
        .maxitems = sizeof(((spfg_fn_t *)0)->out_dp_ids) / sizeof(spfg_dp_id_t)
    },
    {.boundary = true}
};

static azjson_spec_t gr_fns_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_fns_fn_spec,
        .vsize = sizeof(spfg_fn_t),
    },
    {.boundary = true}
};

static azjson_spec_t gr_dps_dp_spec[] = {
    {
        .key = "id",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, id),
    },
    {
        .key = "type",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, type),
    },
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_dp_t, name),
        .vsize = sizeof(((spfg_dp_t *)0)->name)
    },
    {
        .key = "emitted",
        .vtype = JSON_BOOL,
        .voffset = offsetof(spfg_dp_t, emitted),
    },
    {
        .key = "value",
        .vtype = JSON_BOOL,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {
        .key = "value",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {
        .key = "value",
        .vtype = JSON_REAL,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {.boundary = true}
};

static azjson_spec_t gr_dps_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_dps_dp_spec,
        .vsize = sizeof(spfg_dp_t),
    },
    {.boundary = true}
};

static azjson_spec_t gr_ctl_spec[] = {
    {
        .key = "curr_phase",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_gr_ctl_t, curr_phase)
    },
    {
        .key = "curr_fn_idx",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_gr_ctl_t, curr_fn_idx)
    },
    {.boundary = true}
};

static azjson_spec_t gr_spec[] = {
    {
        .key = "id",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_gr_t, id)
    },
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_gr_t, name),
        .vsize = sizeof(((spfg_gr_t *)0)->name)
    },
    {
        .key = "fns",
        .vtype = JSON_ARRAY,
        .vspec = gr_fns_spec,
        .voffset = offsetof(spfg_gr_t, fns),
        .noffset = offsetof(spfg_gr_t, fns_cnt),
        .maxitems = sizeof(((spfg_gr_t *)0)->fns) / sizeof(spfg_fn_t)
    },
    {
        .key = "dps",
        .vtype = JSON_ARRAY,
        .vspec = gr_dps_spec,
        .voffset = offsetof(spfg_gr_t, dps),
        .noffset = offsetof(spfg_gr_t, dps_cnt),
        .maxitems = sizeof(((spfg_gr_t *)0)->dps) / sizeof(spfg_dp_t)
    },
    {
        .key = "ctl",
        .vtype = JSON_OBJECT,
        .vspec = gr_ctl_spec,
        .voffset = offsetof(spfg_gr_t, ctl),
    },
    {.boundary = true}
};

static azjson_spec_t root_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_spec,
    },
    {.boundary = true}
};

static spfg_gr_t json_gr;

spfg_err_t spfg_gr_import_json(char *json_str, uint32_t len, spfg_gr_id_t *out_gr_id) {

    spfg_err_t err;
    spfg_gr_t *gr;

    if (azjson_import(json_str, len, root_spec, &json_gr) != AZJSON_ERROR_NO) {
        return SPFG_ERROR_FAIL;
    }

    if ((err = resolve_global_gr(json_gr.id, &gr)) != SPFG_ERROR_NO) {

        // Existing gr not found, attempt to create one from imported data.
        if (err == SPFG_ERROR_NOT_FOUND) {

            spfg_gr_id_t gr_id;

            if ((err = spfg_gr_create(&gr_id, json_gr.name.chars)) != SPFG_ERROR_NO) {
                return err;
            }

            (void) resolve_global_gr(gr_id, &gr);

            gr->id = json_gr.id;
        }

        // Unexpected error detected during gr resolution, bail out.
        else {
            return err;
        }
    }

    // Existin gr found, cleanup target memory.
    else {
        memset(gr, 0, sizeof(spfg_gr_t));
        memcpy(&gr->name, &json_gr.name, sizeof(json_gr.name));
        gr->id = json_gr.id;
    }

    memcpy(&gr->dps, &json_gr.dps, sizeof(json_gr.dps));
    memcpy(&gr->dps, &json_gr.dps, sizeof(json_gr.dps));
    memcpy(&gr->fns, &json_gr.fns, sizeof(json_gr.fns));
    memcpy(&gr->ctl, &json_gr.ctl, sizeof(spfg_gr_ctl_t));

    spfg_gr_cnt_t gr_idx;

    if ((err = find_global_gr(gr->id, &gr_idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_grx_t *grx = &global_grxs[gr_idx];
    memset(grx, 0, sizeof(spfg_grx_t));
    grx->gr = gr;  // TODO: refactor into gr_reindex function

    spfg_fnx_t *fnx = &grx->fnx[gr->ctl.curr_fn_idx];
    memset(fnx, 0, sizeof(spfg_fnx_t));

    if ((err = resolve_gr_fn(gr, gr->fns[gr->ctl.curr_fn_idx].id, &fnx->fn)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_FAIL;
    }

    if ((err = spfg_fn_reindex(grx, fnx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_REINDEX_FN;
    }

    if (out_gr_id) {
        *out_gr_id = gr->id;
    }

    return SPFG_ERROR_NO;
}



static spfg_err_t sappend(char *output, size_t output_len, uint32_t *vcnt, uint32_t *rcnt, const char *fmt, ...)
{
    int result;
    va_list ap;
    va_start(ap, fmt);

    result = vsnprintf(&output[*rcnt], output_len - *rcnt, fmt, ap);

    if (result < 0) {
        // TODO: properly handle error
        return SPFG_ERROR_FAIL;
    }

    *vcnt += result;
    *rcnt = *vcnt > output_len - 1 ? output_len - 1 : *vcnt;

    va_end (ap);
    return SPFG_ERROR_NO;
}

spfg_err_t spfg_gr_export_json(spfg_gr_id_t gr_id, char *output, uint32_t output_len, uint32_t *slen)
{
    spfg_gr_t *gr;
    spfg_err_t err;
    uint32_t rcnt = 0;
    uint32_t vcnt = 0;

    if (!output) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(output, 0, output_len);

    if ((err = resolve_global_gr(gr_id, &gr)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    sappend(output, output_len, &vcnt, &rcnt, "{");
    sappend(output, output_len, &vcnt, &rcnt, "\"id\": %d, ", gr->id);
    sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->name.chars);

    sappend(output, output_len, &vcnt, &rcnt, "\"fns\": [");
    for (uint32_t idx = 0; idx < SPFG_MAX_GRID_FNS; idx++) {
        if (!gr->fns[idx].name.chars[0]) {
            continue;
        }
        sappend(output, output_len, &vcnt, &rcnt, "{");
        sappend(output, output_len, &vcnt, &rcnt, "\"id\": %d, ", gr->fns[idx].id);
        sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->fns[idx].name.chars);
        sappend(output, output_len, &vcnt, &rcnt, "\"type\": %d, ", gr->fns[idx].type);
        sappend(output, output_len, &vcnt, &rcnt, "\"phase\": %d, ", gr->fns[idx].phase);
        sappend(output, output_len, &vcnt, &rcnt, "\"in_dp_ids\": [");

        for (uint32_t _idx = 0; _idx < gr->fns[idx].in_dp_ids_len; _idx++) {
            sappend(output, output_len, &vcnt, &rcnt, "%d", gr->fns[idx].in_dp_ids[_idx]);
            if (_idx + 1 < gr->fns[idx].in_dp_ids_len) {
                sappend(output, output_len, &vcnt, &rcnt, ", ");
            }
        }
        sappend(output, output_len, &vcnt, &rcnt, "], ");

        sappend(output, output_len, &vcnt, &rcnt, "\"out_dp_ids\": [");
        for (uint32_t _idx = 0; _idx < gr->fns[idx].out_dp_ids_len; _idx++) {
            sappend(output, output_len, &vcnt, &rcnt, "%d", gr->fns[idx].out_dp_ids[_idx]);
            if (_idx + 1 < gr->fns[idx].out_dp_ids_len) {
                sappend(output, output_len, &vcnt, &rcnt, ", ");
            }
        }
        sappend(output, output_len, &vcnt, &rcnt, "]");

        sappend(output, output_len, &vcnt, &rcnt, "}");
        if (idx < SPFG_MAX_GRID_FNS && gr->fns[idx + 1].name.chars[0]) {
            sappend(output, output_len, &vcnt, &rcnt, ", ");
        }
    }

    sappend(output, output_len, &vcnt, &rcnt, "], ");
    sappend(output, output_len, &vcnt, &rcnt, "\"dps\": [");

    for (uint32_t idx = 0; idx < SPFG_MAX_GRID_DPS; idx++) {
        if (!gr->dps[idx].name.chars[0]) {
            continue;
        }

        sappend(output, output_len, &vcnt, &rcnt, "{");
        sappend(output, output_len, &vcnt, &rcnt, "\"id\": %d, ", gr->dps[idx].id);
        sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->dps[idx].name.chars);
        sappend(output, output_len, &vcnt, &rcnt, "\"type\": %d, ", gr->dps[idx].type);

        switch (gr->dps[idx].type) {
            case SPFG_DP_INT: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %d, ", gr->dps[idx].value.integer);
                break;
            }
            case SPFG_DP_REAL: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %.5G, ", gr->dps[idx].value.real);
                break;
            }
            case SPFG_DP_BOOL: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %s, ", gr->dps[idx].value.boolean ? "true" : "false");
                break;
            }
            default: {
                return SPFG_ERROR_FAIL;
            }
        }

        sappend(output, output_len, &vcnt, &rcnt, "\"emitted\": %s", gr->dps[idx].emitted ? "true" : "false");
        sappend(output, output_len, &vcnt, &rcnt, "}");

        if (idx < SPFG_MAX_GRID_DPS && gr->dps[idx + 1].name.chars[0]) {
            sappend(output, output_len, &vcnt, &rcnt, ", ");
        }
    }

    sappend(output, output_len, &vcnt, &rcnt, "], ");

    sappend(output, output_len, &vcnt, &rcnt, "\"ctl\": {");
    sappend(output, output_len, &vcnt, &rcnt, "\"curr_phase\": %d, ", gr->ctl.curr_phase);
    sappend(output, output_len, &vcnt, &rcnt, "\"curr_fn_idx\": %d", gr->ctl.curr_fn_idx);
    sappend(output, output_len, &vcnt, &rcnt, "}");

    sappend(output, output_len, &vcnt, &rcnt, "}");

    if (slen) {
        *slen = rcnt;
    }

    return SPFG_ERROR_NO;
}
