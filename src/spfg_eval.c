#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_index.h"


spfg_err_t _dp_bool_set(spfg_dp_t *dp, spfg_boolean_t value)
{
    dp->emitted = dp->value.boolean != value;
    dp->value.boolean = value;
    return SPFG_ERROR_NO;
}

spfg_err_t _exec_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

spfg_err_t fnx_and_bool_bool_ret_bool_eval(spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_boolean_t out;
    spfg_err_t err;

    if ((err = _exec_and_bool_bool_ret_bool(
        fnx->in_dps[0]->value.boolean,
        fnx->in_dps[1]->value.boolean,
        &out)) != SPFG_ERROR_NO) {
        return err;
    }

    if ((err = _dp_bool_set(fnx->out_dps[0], out)) != SPFG_ERROR_NO) {
        return err;
    }

    return SPFG_ERROR_NO;
}

spfg_err_t _fnx_eval(spfg_fnx_t *fnx, spfg_ts_t ts) {

    switch (fnx->fn->type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            return fnx_and_bool_bool_ret_bool_eval(fnx, ts);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

spfg_err_t _fnx_changed_dps_clear(spfg_fnx_t *fnx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {
        fnx->in_dps[i]->emitted = false;
    }

    return SPFG_ERROR_NO;
}


spfg_err_t _fnx_run(spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = _spfg_find_changed_fnx_in_dp(fnx, NULL)) != SPFG_ERROR_NO) {
        if (err == SPFG_ERROR_NOT_FOUND) {
            return SPFG_ERROR_NO;
        }
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    if ((err = _fnx_eval(fnx, ts)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    if ((err = _fnx_changed_dps_clear(fnx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    return SPFG_ERROR_NO;
}

// ---

spfg_err_t _spfg_dp_set_bool(spfg_rt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t value)
{
    spfg_dp_t *dp;

    if (_spfg_resolve_gr_dp(&rt->gr, dp_id, &dp) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    return _dp_bool_set(dp, value);
}

spfg_err_t _spfg_dp_get_bool(spfg_rt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted)
{
    spfg_dp_t *dp;

    if (_spfg_resolve_gr_dp(&rt->gr, dp_id, &dp) != SPFG_ERROR_NO) {
        return SPFG_ERROR_INVALID_DP_ID;
    }

    *value = dp->value.boolean;

    if (emitted) {
        *emitted = dp->emitted;
    }

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_rt_reset_cycle(spfg_rt_t *rt)
{
    rt->gr.ctl.curr_fn_idx = 0;

    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_rt_run_cycle(spfg_rt_t *rt, spfg_ts_t ts, spfg_run_cb_t cb, void *udata)
{
    spfg_err_t err;
    spfg_fnx_t *fnx;

    // Ensures index sanity.
    if (!rt->grx.is_valid) {
        if ((err = _spfg_index_rebuild(rt)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_REINDEX;
        }
    }

    for (;;) {

        // Stop condition: array boundary protection.
        if (rt->gr.ctl.curr_fn_idx >= SPFG_MAX_GRID_FNS) {
            break;
        }

        fnx = &rt->grx.fnx[rt->gr.ctl.curr_fn_idx];

        // Stop condition: no more pending functions to evaluate.
        if (!fnx->fn) {
            break;
        }

        // Stop condition: no more pending functions to evaluate.
        if (!fnx->fn->name.chars[0]) {
            break;
        }

        if (cb) {

            // Hands control back to caller passing progress info.
            err = cb((spfg_runtime_t *)rt, fnx->fn->id, fnx->fn->phase, udata);

            // Stop condition: callback explicit stop.
            if (err == SPFG_LOOP_CONTROL_STOP) {
                break;
            }

            // Stop condition: callback failure.
            if (err != SPFG_ERROR_NO) {
                return SPFG_ERROR_EVAL_CB_FAILURE;
            }
        }

        // Stop condition: evaluation failure.
        if ((err = _fnx_run(fnx, ts)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_EVAL_FN_FAILURE;
        }

        rt->gr.ctl.curr_fn_idx += 1;
    }

    return SPFG_ERROR_NO;
}
