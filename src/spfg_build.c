#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"

extern spfg_gr_t global_grs[SPFG_MAX_GRID_CNT];
extern spfg_grx_t global_grxs[SPFG_MAX_GRID_CNT];

spfg_err_t gr_dp_create(spfg_gr_t *gr, unsigned int dp_idx, spfg_dp_type_t dp_type, const char *name)
{
    spfg_err_t err = SPFG_ERROR_NO;

    if (!gr) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    // TODO: dp type validation

    if ((err = create_name(name, &gr->dps[dp_idx].name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set datapoint name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    gr->dps[dp_idx].id = GEN_SPFG_DP_ID(gr->id, dp_idx);
    gr->dps[dp_idx].type = dp_type;

    return SPFG_ERROR_NO;
}

spfg_err_t grx_fnx_reindex(spfg_grx_t *grx, spfg_fnx_t *fnx)
{
    // TODO: ensure grx->fnx is still sorted by fnx->fn->phase + fnx->fn->id.

    spfg_err_t err;

    memset(fnx->in_dps, 0, SPFG_MAX_FN_IN_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; i < SPFG_MAX_FN_IN_DPS && fnx->fn->in_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->in_dp_ids[i], &fnx->in_dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    memset(fnx->out_dps, 0, SPFG_MAX_FN_OUT_DPS * sizeof(spfg_dp_t *));

    for (int i = 0; i < SPFG_MAX_FN_OUT_DPS && fnx->fn->out_dp_ids[i]; i++) {
        if ((err = resolve_gr_dp(grx->gr, fnx->fn->out_dp_ids[i], &fnx->out_dps[i])) != SPFG_ERROR_NO) {
            return SPFG_ERROR_INVALID_DP_ID;
        }
    }

    return SPFG_ERROR_NO;
}

spfg_err_t fn_signature_validate(spfg_dp_t *in_dps[], uint8_t in_dps_len,
                                        spfg_dp_t *out_dps[], uint8_t out_dps_len,
                                        spfg_dp_type_t *in_dp_types, uint8_t in_dp_types_len,
                                        spfg_dp_type_t *out_dp_types, uint8_t out_dp_types_len,
                                        const char *fn_name) {

    uint8_t cnt = 0;

    if (in_dps_len < in_dp_types_len) {
        fprintf(stderr, "%d missing input datapoints for function '%s' expecting %d input datapoints\n", in_dp_types_len - in_dps_len, fn_name, in_dp_types_len);
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (cnt = 0; cnt < in_dps_len && cnt < SPFG_MAX_FN_IN_DPS; cnt++) {

        if (cnt >= in_dp_types_len) {
            fprintf(stderr, "unexpected input datapoint '%s' (position %d) for function '%s' expecting only %d input datapoints\n", in_dps[cnt]->name.chars, cnt, fn_name, in_dp_types_len);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }

        if (in_dps[cnt]->type != in_dp_types[cnt]) {
            fprintf(stderr, "input datapoint '%s' type %d is incompatible with expected input type %d (position %d) for function '%s'\n", in_dps[cnt]->name.chars, in_dps[cnt]->type, in_dp_types[cnt], cnt, fn_name);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }

    if (out_dps_len < out_dp_types_len) {
        fprintf(stderr, "%d missing output datapoints for function '%s' expecting %d output datapoints\n", out_dp_types_len - out_dps_len, fn_name, out_dp_types_len);
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    for (cnt = 0; cnt < out_dps_len && cnt < SPFG_MAX_FN_OUT_DPS; cnt++) {

        if (cnt >= out_dp_types_len) {
            fprintf(stderr, "unexpected output datapoint '%s' (position %d) for function '%s' expecting only %d output datapoints\n", out_dps[cnt]->name.chars, cnt, fn_name, out_dp_types_len);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }

        if (out_dps[cnt]->type != out_dp_types[cnt]) {
            fprintf(stderr, "output datapoint '%s' type %d is incompatible with expected output type %d (position %d) for function '%s'\n", out_dps[cnt]->name.chars, out_dps[cnt]->type, out_dp_types[cnt], cnt, fn_name);
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }

    return SPFG_ERROR_NO;
}

spfg_err_t fn_validate(spfg_fn_type_t type,
                                   spfg_dp_t *in_dps[], uint8_t in_dps_len,
                                   spfg_dp_t *out_dps[], uint8_t out_dps_len,
                                   const char *fn_name) {

    switch (type) {

        case SPFG_FN_INVERT_BOOL_RET_BOOL:
        {
            return SPFG_ERROR_UNIMPLEMENTED;
        }

        case SPFG_FN_AND_BOOL_BOOL_RET_BOOL:
        {
            spfg_dp_type_t in_dp_types[] = {SPFG_DP_BOOL, SPFG_DP_BOOL};
            spfg_dp_type_t out_dp_types[] = {SPFG_DP_BOOL};
            return fn_signature_validate(
                in_dps, in_dps_len,
                out_dps, out_dps_len,
                in_dp_types, 2,
                out_dp_types, 1,
                fn_name);
        }

        default:
        {
            return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
        }
    }
}

spfg_err_t gr_fn_create(spfg_gr_t *gr,
                        int fn_idx, spfg_fn_type_t type, spfg_phase_t phase,
                        spfg_dp_id_t *in_dp_ids, uint8_t in_dp_ids_len,
                        spfg_dp_id_t *out_dp_ids, uint8_t out_dp_ids_len,
                        const char *name)
{
    spfg_err_t err;
    spfg_fn_t *fn = &gr->fns[fn_idx];

    if ((err = create_name(name, &fn->name)) != SPFG_ERROR_NO) {
        fprintf(stderr, "failed to set function name: err=[%d]\n", err);
        return SPFG_ERROR_BAD_BLOCK_NAME;
    }

    fn->type = type;
    fn->phase = phase;
    fn->id = GEN_SPFG_FN_ID(gr->id, fn_idx);
    memcpy(fn->in_dp_ids, in_dp_ids, in_dp_ids_len * sizeof(spfg_dp_id_t));
    fn->in_dp_ids_len = in_dp_ids_len;
    memcpy(fn->out_dp_ids, out_dp_ids, out_dp_ids_len * sizeof(spfg_dp_id_t));
    fn->out_dp_ids_len = out_dp_ids_len;

    return SPFG_ERROR_NO;
}

spfg_err_t spfg_gr_idx_clear(spfg_gr_t *gr)
{
    uint32_t gr_idx;

    (void) find_gr(gr->id, &gr_idx);

    memset(&global_grxs[gr_idx].fnx, 0, sizeof(global_grxs[gr_idx].fnx));

    return SPFG_ERROR_NO;
}

spfg_err_t spfg_grx_reindex(spfg_grx_t *grx)
{
    spfg_err_t err;

    for (uint32_t i = 0; i < SPFG_MAX_GRID_FNS; i++) {

        if (!grx->gr->fns[i].name.chars[0]) {
            continue;
        }

        spfg_fnx_t *fnx = &grx->fnx[i];
        fnx->fn = &grx->gr->fns[i];

        if ((err = grx_fnx_reindex(grx, fnx)) != SPFG_ERROR_NO) {
            fprintf(stderr, "failed to reindex function: err=[%d]\n", err);
            return SPFG_ERROR_FAIL;
        }
    }

    grx->is_valid = true;

    return SPFG_ERROR_NO;
}
