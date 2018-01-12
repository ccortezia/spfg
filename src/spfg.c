#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_build.h"
#include "spfg_eval.h"

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

extern spfg_err_t spfg_gr_create(const char *name, spfg_gr_id_t *gr_id)
{
    if (!gr_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_gr_create(name, gr_id);
}

extern spfg_err_t spfg_gr_remove(spfg_gr_id_t gr_id)
{
    spfg_gr_t *gr;

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return _spfg_gr_remove(gr);
}

extern spfg_err_t spfg_gr_get_ids(spfg_gr_id_t *output, spfg_gr_cnt_t maxlen, spfg_gr_cnt_t *count)
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
    spfg_gr_t *gr;

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!dp_id) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return _spfg_dp_create(gr, dp_type, name, dp_id);
}

extern spfg_err_t spfg_dp_remove(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id)
{
    spfg_gr_t *gr;
    spfg_dp_t *dp;

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if (_spfg_resolve_gr_dp(gr, dp_id, &dp) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    // Lookup fn associated to dp
    // TODO: extract into separate function.
    for (int i = 0; i < SPFG_MAX_GRID_FNS; i++) {
        if (!gr->fns[i].name.chars[0]) {
            continue;
        }
        for (int j = 0; j < SPFG_MAX_FN_IN_DPS; j++) {
            if (gr->fns[i].in_dp_ids[j] == dp->id) {
                return SPFG_ERROR_FN_INTEGRITY;
            }
        }
        for (int j = 0; j < SPFG_MAX_FN_OUT_DPS; j++) {
            if (gr->fns[i].out_dp_ids[j] == dp->id) {
                return SPFG_ERROR_FN_INTEGRITY;
            }
        }
    }

    return _spfg_dp_remove(gr, dp);
}

extern spfg_err_t spfg_fn_create(spfg_gr_id_t gr_id,
                                 spfg_fn_type_t type,
                                 spfg_phase_t phase,
                                 spfg_dp_id_t *in_dp_ids, uint8_t in_dp_ids_len,
                                 spfg_dp_id_t *out_dp_ids, uint8_t out_dp_ids_len,
                                 const char *name,
                                 spfg_fn_id_t *fn_id)
{
    spfg_gr_t *gr;

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (in_dp_ids_len > SPFG_MAX_FN_IN_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    if (out_dp_ids_len > SPFG_MAX_FN_OUT_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return _spfg_fn_create(gr, type, phase, in_dp_ids, in_dp_ids_len, out_dp_ids, out_dp_ids_len, name, fn_id);
}

extern spfg_err_t spfg_fn_remove(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id)
{
    spfg_gr_t *gr;
    spfg_fn_t *fn;

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if (_spfg_resolve_gr_fn(gr, fn_id, &fn) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_FN_ID;
    }

    return _spfg_fn_remove(gr, fn);
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
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if (_spfg_resolve_gr_dp(gr, dp_id, &dp) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    return dp_bool_set(dp, value);
}

extern spfg_err_t spfg_dp_get_bool(spfg_gr_id_t gr_id, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted)
{
    spfg_dp_t *dp;
    spfg_gr_t *gr;

    if (!value) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    if (_spfg_resolve_gr_dp(gr, dp_id, &dp) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    *value = dp->value.boolean;

    if (emitted) {
        *emitted = dp->emitted;
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
    spfg_grx_t *grx;

    if (_spfg_resolve_grx(gr_id, &grx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return _spfg_reset_cycle(grx);
}

extern spfg_err_t spfg_run_cycle(spfg_gr_id_t gr_id, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata)
{
    spfg_grx_t *grx;

    if (_spfg_resolve_grx(gr_id, &grx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    return _spfg_run_cycle(grx, ts, cb, udata);
}


// -------------------------------------------------------------------------------------------------
// Import / Export API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_gr_export_bin(spfg_gr_id_t gr_id, void *outbuf, uint32_t outbuf_len)
{
    if (!outbuf) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (outbuf_len < sizeof(spfg_gr_exp_t)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    spfg_gr_t *gr;

    if (_spfg_resolve_gr(gr_id, &gr) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_GR_ID;
    }

    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)outbuf;
    memset(grxp, 0, sizeof(spfg_gr_exp_t));
    memcpy(&grxp->data, gr, sizeof(spfg_gr_t));

    return SPFG_ERROR_NO;
}

extern spfg_err_t spfg_gr_import_bin(void *data, uint32_t data_len, spfg_gr_id_t *gr_id, const char *name)
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

    if ((err = spfg_gr_create(name, gr_id)) != SPFG_ERROR_NO) {
        // TODO: review unbounded error situation.
        return err;
    }

    spfg_gr_t *gr;

    (void) _spfg_resolve_gr(*gr_id, &gr);

    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)data;

    memcpy(&gr->dps, &grxp->data.dps, sizeof(grxp->data.dps));
    memcpy(&gr->fns, &grxp->data.fns, sizeof(grxp->data.fns));
    memcpy(&gr->ctl, &grxp->data.ctl, sizeof(spfg_gr_ctl_t));

    return SPFG_ERROR_NO;
}

// -------------------------------------------------------------------------------------------------
// Inspection API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_info(spfg_info_t *info)
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
