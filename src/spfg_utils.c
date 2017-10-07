#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"

extern spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
extern spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

spfg_err_t create_name(const char *ascii, spfg_block_name_t *name)
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

    if (strnlen(ascii, sizeof(name->chars)) >= sizeof(name->chars)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    memset(name->chars, 0, sizeof(name->chars));
    strcpy(name->chars, ascii);
    name->chars[sizeof(name->chars) - 1] = 0;
    return SPFG_ERROR_NO;
}

spfg_err_t find_gr(spfg_gr_id_t gr_id, uint32_t *idx)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (gr_id == 0) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (int i = 0; i < SPFG_MAX_GRID_CNT; i++) {
        if (global_grs[i].id == gr_id) {
            *idx = i;
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

spfg_err_t resolve_gr(spfg_gr_id_t gr_id, spfg_gr_t **gr)
{
    spfg_err_t err;
    unsigned int idx;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if ((err = find_gr(gr_id, &idx)) != SPFG_ERROR_NO) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *gr = &global_grs[idx];
    return SPFG_ERROR_NO;
}

spfg_err_t resolve_grx(spfg_gr_id_t gr_id, spfg_grx_t **grx)
{
    unsigned int gr_idx;

    if (find_gr(gr_id, &gr_idx) != SPFG_ERROR_NO) {
        return SPFG_ERROR_NOT_FOUND;
    }

    *grx = &global_grxs[gr_idx];

    return SPFG_ERROR_NO;
}

spfg_err_t resolve_gr_dp(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp)
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

spfg_err_t resolve_gr_fn(spfg_gr_t *gr, spfg_fn_id_t fn_id, spfg_fn_t **fn)
{
    if (!fn) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_FNS; i++) {
        if (gr->fns[i].id == fn_id) {
            *fn = &gr->fns[i];
            return SPFG_ERROR_NO;
        }
    }

    return SPFG_ERROR_NOT_FOUND;
}

spfg_err_t resolve_dps(spfg_gr_t *gr, spfg_dp_id_t *dp_ids, spfg_dp_t *dps[], uint8_t length)
{
    spfg_err_t err;

    for (int i = 0; dp_ids[i] && i < length; i++) {
        if ((err = resolve_gr_dp(gr, dp_ids[i], &dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    return SPFG_ERROR_NO;
}

spfg_err_t find_free_gr(uint32_t *idx, spfg_gr_t **gr)
{
    if (!idx) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    for (int i = 0; i < SPFG_MAX_GRID_CNT; i++) {

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

spfg_err_t find_free_gr_dp(spfg_gr_t *gr, uint32_t *idx, spfg_dp_t **dp)
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

spfg_err_t find_free_gr_fn(spfg_gr_t *gr, uint32_t *idx, spfg_fn_t **fn)
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

spfg_err_t find_changed_in_dp_for_fnx(spfg_fnx_t *fnx, uint32_t *idx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {

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

spfg_err_t clear_changed_fnx_inputs(spfg_fnx_t *fnx)
{
    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->in_dps[i]; i++) {
        fnx->in_dps[i]->emitted = 0;
    }

    return SPFG_ERROR_NO;
}
