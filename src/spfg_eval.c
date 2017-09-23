#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"

spfg_err_t impl_dp_set_bool(spfg_dp_t *dp, spfg_boolean_t value)
{
    dp->emitted = dp->value.boolean != value;
    dp->value.boolean = value;
    return SPFG_ERROR_NO;
}

spfg_err_t impl_fn_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result)
{
    *result = p0 && p1;
    return SPFG_ERROR_NO;
}

spfg_err_t eval_fn_and_bool_bool_ret_bool(spfg_fnx_t *fnx, spfg_ts_t ts)
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

spfg_err_t eval_fnx(spfg_fnx_t *fnx, spfg_ts_t ts) {

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

spfg_err_t spfg_run_fnx(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts)
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

spfg_err_t spfg_resume_cycle_grx(spfg_grx_t *grx, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata)
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
