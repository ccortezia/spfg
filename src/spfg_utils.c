#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"

extern spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
extern spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

spfg_err_t _spfg_block_name_set(spfg_block_name_t *name, const char *ascii)
{
    if (!*ascii) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if (strnlen(ascii, sizeof(name->chars)) >= sizeof(name->chars)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    memset(name->chars, 0, sizeof(name->chars));
    strcpy(name->chars, ascii);
    name->chars[sizeof(name->chars) - 1] = 0;
    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_calc_gr_idx(spfg_gr_id_t gr_id, uint32_t *idx)
{
    uint32_t gr_idx;

    if (gr_id == 0) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    gr_idx = gr_id - SPFG_GR_ID0;

    if (gr_idx >= SPFG_MAX_GRID_CNT) {
        return SPFG_ERROR_NOT_FOUND;
    }

    if (idx) {
        *idx = gr_idx;
    }

    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_find_gr(spfg_gr_id_t gr_id, uint32_t *idx)
{
    uint32_t gr_idx;

    if (_spfg_calc_gr_idx(gr_id, &gr_idx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if (!global_grs[gr_idx].name.chars[0]) {
        return SPFG_ERROR_NOT_FOUND;
    }

    if (idx) {
        *idx = gr_idx;
    }

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_resolve_gr(spfg_gr_id_t gr_id, spfg_gr_t **gr)
{
    unsigned int idx;

    if (gr_id == 0) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if (_spfg_find_gr(gr_id, &idx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *gr = &global_grs[idx];

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_resolve_grx(spfg_gr_id_t gr_id, spfg_grx_t **grx)
{
    unsigned int gr_idx;

    if (gr_id == 0) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    if (_spfg_find_gr(gr_id, &gr_idx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *grx = &global_grxs[gr_idx];

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_resolve_gr_dp(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp)
{
    spfg_gr_dp_cnt_t dp_idx = dp_id - SPFG_GR_DP_ID0(gr->id);

    if (dp_idx >= SPFG_MAX_GRID_DPS) {
        return SPFG_ERROR_NOT_FOUND;
    }

    if (!gr->dps[dp_idx].name.chars[0]) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *dp = &gr->dps[dp_idx];

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_resolve_gr_fn(spfg_gr_t *gr, spfg_fn_id_t fn_id, spfg_fn_t **fn)
{
    spfg_gr_fn_cnt_t fn_idx = fn_id - SPFG_GR_DP_ID0(gr->id);

    if (fn_idx >= SPFG_MAX_GRID_FNS) {
        return SPFG_ERROR_NOT_FOUND;
    }

    if (!gr->fns[fn_idx].name.chars[0]) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *fn = &gr->fns[fn_idx];

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_resolve_gr_dps(spfg_gr_t *gr, spfg_dp_id_t *dp_ids, spfg_dp_t *dps[], uint8_t length)
{
    for (spfg_gr_dp_cnt_t i = 0; dp_ids[i] && i < length; i++) {

        if (_spfg_resolve_gr_dp(gr, dp_ids[i], &dps[i]) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    return SPFG_ERROR_NO;
}

spfg_err_t _spfg_find_free_gr(uint32_t *idx, spfg_gr_t **gr)
{
    for (spfg_gr_cnt_t i = 0; i < SPFG_MAX_GRID_CNT; i++) {

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

spfg_err_t _spfg_find_free_gr_dp(spfg_gr_t *gr, uint32_t *idx, spfg_dp_t **dp)
{
    for (spfg_gr_dp_cnt_t i = 0; i < SPFG_MAX_GRID_DPS; i++) {

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

spfg_err_t _spfg_find_free_gr_fn(spfg_gr_t *gr, uint32_t *idx, spfg_fn_t **fn)
{
    for (spfg_gr_fn_cnt_t i = 0; i < SPFG_MAX_GRID_FNS; i++) {

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

spfg_err_t _spfg_find_changed_fnx_in_dp(spfg_fnx_t *fnx, uint32_t *idx)
{
    for (spfg_fn_dp_in_cnt_t i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {

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
