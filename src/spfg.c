#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_composition.h"
#include "spfg_evaluation.h"

// -------------------------------------------------------------------------------------------------
// Private Library Data
// -------------------------------------------------------------------------------------------------

spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

static char initialized;

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
