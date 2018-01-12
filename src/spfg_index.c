#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"

extern spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
extern spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];


spfg_err_t _spfg_gr_index_clear(spfg_gr_t *gr)
{
    spfg_grx_t *grx;

    (void) _spfg_resolve_grx(gr->id, &grx);

    memset(&grx->fnx, 0, sizeof(grx->fnx));

    grx->is_valid = false;

    return SPFG_ERROR_NO;
}

int fnx_cmp(const void *p1, const void *p2)
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

spfg_err_t _spfg_grx_index_rebuild(spfg_grx_t *grx)
{
    // Capture scheme information into an array optimized for evaluation.
    // The resulting array may be sparse, thus not safe for naive sequential evaluation.
    for (uint32_t i = 0; i < SPFG_MAX_GRID_FNS; i++) {

        if (!grx->gr->fns[i].name.chars[0]) {
            continue;
        }

        spfg_fnx_t *fnx = &grx->fnx[i];
        fnx->fn = &grx->gr->fns[i];

        memset(fnx->in_dps, 0, SPFG_MAX_FN_IN_DPS * sizeof(spfg_dp_t *));

        for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->fn->in_dp_ids[i]; i++) {
            if (_spfg_resolve_gr_dp(grx->gr, fnx->fn->in_dp_ids[i], &fnx->in_dps[i]) != SPFG_ERROR_NO) {
                return SPFG_ERROR_INVALID_DP_ID;
            }
        }

        memset(fnx->out_dps, 0, SPFG_MAX_FN_OUT_DPS * sizeof(spfg_dp_t *));

        for (int i = 0; i < SPFG_MAX_FN_OUT_DPS && fnx->fn->out_dp_ids[i]; i++) {
            if (_spfg_resolve_gr_dp(grx->gr, fnx->fn->out_dp_ids[i], &fnx->out_dps[i]) != SPFG_ERROR_NO) {
                return SPFG_ERROR_INVALID_DP_ID;
            }
        }
    }

    // Sorts the captured information so it becomes safe for naive sequential evaluation.
    // The resulting sorted array is not sparse, and the first zeroed element works as a sentinel
    // and can be used to determine there are no more function to evaluate point-forward.
    qsort(grx->fnx, SPFG_MAX_GRID_FNS, sizeof(spfg_fnx_t), fnx_cmp);

    grx->is_valid = true;

    return SPFG_ERROR_NO;
}
