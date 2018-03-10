#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"

extern spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
extern spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];


spfg_err_t _spfg_rt_index_clear(spfg_rt_t *rt)
{
    memset(&rt->grx, 0, sizeof(spfg_grx_t));

    return SPFG_ERROR_NO;
}

static int fnx_cmp(const void *p1, const void *p2)
{
    spfg_fn_t *fn1 = ((spfg_fnx_t *)p1)->fn;
    spfg_fn_t *fn2 = ((spfg_fnx_t *)p2)->fn;

    if (!fn1) {
        return +1;
    }

    if (!fn2) {
        return -1;
    }

    int phase_cmp = fn1->phase - fn2->phase;

    if (phase_cmp == 0) {
        return fn1->id - fn2->id;
    }

    return phase_cmp;
}

spfg_err_t _spfg_rt_index_rebuild(spfg_rt_t *rt)
{
    // Capture scheme information into an array optimized for evaluation.
    // The resulting array may be sparse, thus not safe for plain sequential evaluation.
    for (uint32_t i = 0; i < SPFG_MAX_GRID_FNS; i++) {

        if (!rt->gr.fns[i].name.chars[0]) {
            continue;
        }

        spfg_fnx_t *fnx = &rt->grx.fnx[i];

        fnx->fn = &rt->gr.fns[i];

        memset(fnx->in_dps, 0, sizeof(fnx->in_dps));

        for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->fn->in_dp_ids[i]; i++) {
            if (_spfg_resolve_gr_dp(&rt->gr, fnx->fn->in_dp_ids[i], &fnx->in_dps[i]) != SPFG_ERROR_NO) {
                return SPFG_ERROR_INVALID_DP_ID;
            }
        }

        memset(fnx->out_dps, 0, sizeof(fnx->out_dps));

        for (int i = 0; i < SPFG_MAX_FN_OUT_DPS && fnx->fn->out_dp_ids[i]; i++) {
            if (_spfg_resolve_gr_dp(&rt->gr, fnx->fn->out_dp_ids[i], &fnx->out_dps[i]) != SPFG_ERROR_NO) {
                return SPFG_ERROR_INVALID_DP_ID;
            }
        }
    }

    // Sorts the captured information so it becomes safe for plain sequential evaluation.
    // The resulting sorted array is not sparse, and the first zeroed element works as a sentinel
    // to determine there are no more functions to evaluate point-forward.
    qsort(rt->grx.fnx, SPFG_MAX_GRID_FNS, sizeof(spfg_fnx_t), fnx_cmp);

    rt->grx.is_valid = true;

    return SPFG_ERROR_NO;
}
