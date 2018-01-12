#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_index.h"


spfg_err_t dp_bool_set(spfg_dp_t *dp, spfg_boolean_t value)
{
    dp->emitted = dp->value.boolean != value;
    dp->value.boolean = value;
    return SPFG_ERROR_NO;
}

spfg_err_t fn_and_bool_bool_ret_bool_exec(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

spfg_err_t fn_and_bool_bool_ret_bool_eval(spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_boolean_t out;
    spfg_err_t err;

    if ((err = fn_and_bool_bool_ret_bool_exec(
        fnx->in_dps[0]->value.boolean,
        fnx->in_dps[1]->value.boolean,
        &out)) != SPFG_ERROR_NO) {
        return err;
    }

    if ((err = dp_bool_set(fnx->out_dps[0], out)) != SPFG_ERROR_NO) {
        return err;
    }

    return SPFG_ERROR_NO;
}

spfg_err_t fnx_eval(spfg_fnx_t *fnx, spfg_ts_t ts) {

    switch (fnx->fn->type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            return fn_and_bool_bool_ret_bool_eval(fnx, ts);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

spfg_err_t fnx_changed_dps_clear(spfg_fnx_t *fnx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {
        fnx->in_dps[i]->emitted = 0;
    }

    return SPFG_ERROR_NO;
}


spfg_err_t grx_fnx_run(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if ((err = _spfg_find_changed_fnx_in_dp(fnx, NULL)) != SPFG_ERROR_NO) {
        if (err == SPFG_ERROR_NOT_FOUND) {
            return SPFG_ERROR_NO;
        }
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    if ((err = fnx_eval(fnx, ts)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to run fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    if ((err = fnx_changed_dps_clear(fnx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_EVAL_FN_FAILURE;
    }

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_reset_cycle(spfg_grx_t *grx)
{
    grx->gr->ctl.curr_phase = 0;
    grx->gr->ctl.curr_fn_idx = 0;
    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_run_cycle(spfg_grx_t *grx, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata)
{
    spfg_err_t err;
    spfg_fnx_t *fnx;

    if (!grx->is_valid) {
        if ((err = _spfg_grx_index_rebuild(grx)) != SPFG_ERROR_NO) {
            return SPFG_ERROR_REINDEX;
        }
    }

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
                return SPFG_ERROR_EVAL_CB_FAILURE;
            }
        }

        if ((err = grx_fnx_run(grx, fnx, ts)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to run fn %s on grid %d: err=[%d]\n", fnx->fn->name.chars, grx->gr->id, err);
            return SPFG_ERROR_EVAL_FN_FAILURE;
        }

        grx->gr->ctl.curr_fn_idx += 1;
    }

    return SPFG_ERROR_NO;
}
