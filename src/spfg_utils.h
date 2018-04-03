#ifndef __SPFG_UTILS_H__
#define __SPFG_UTILS_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

// -------------------------------------------------------------------------------------------------
// Local macros
// -------------------------------------------------------------------------------------------------

#define SPFG_GR_DP_ID0 1
#define SPFG_GR_FN_ID0 1
#define SPFG_DP_ID(idx) (SPFG_GR_DP_ID0 + idx)
#define SPFG_FN_ID(idx) (SPFG_GR_FN_ID0 + idx)
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


#define ESTIMATED_SIZE_BLOCK_NAME \
    SPFG_BLOCK_NAME_MAX_LENGTH

#define ESTIMATED_SIZE_DP_VALUE \
    sizeof(spfg_real_t)

#define ESTIMATED_SIZE_DP \
    ESTIMATED_SIZE_BLOCK_NAME + \
    ESTIMATED_SIZE_DP_VALUE + \
    sizeof(spfg_dp_id_t) + \
    sizeof(spfg_dp_type_t) + \
    sizeof(spfg_boolean_t) + \
    5 // padding

#define ESTIMATED_SIZE_FN \
    ESTIMATED_SIZE_BLOCK_NAME + \
    sizeof(spfg_fn_id_t) + \
    sizeof(spfg_fn_type_t) + \
    sizeof(spfg_phase_t) + \
    ((sizeof(spfg_dp_id_t)) * (SPFG_MAX_FN_IN_DPS)) + \
    sizeof(spfg_fn_dp_in_cnt_t) + \
    ((sizeof(spfg_dp_id_t)) * (SPFG_MAX_FN_OUT_DPS)) + \
    sizeof(spfg_fn_dp_out_cnt_t) + \
    8 // padding

#define ESTIMATED_SIZE_GR_CTL \
    sizeof(spfg_gr_fn_cnt_t)

#define ESTIMATED_SIZE_GR \
    ESTIMATED_SIZE_GR_CTL + \
    ESTIMATED_SIZE_BLOCK_NAME + \
    ((ESTIMATED_SIZE_DP) * (SPFG_MAX_GRID_DPS)) + \
    ((ESTIMATED_SIZE_FN) * (SPFG_MAX_GRID_FNS)) + \
    sizeof(spfg_gr_dp_cnt_t) + \
    sizeof(spfg_gr_fn_cnt_t) + \
    10 // padding

#define ESTIMATED_SIZE_FNX \
    sizeof(void *) + \
    ((sizeof(void *)) * (SPFG_MAX_FN_IN_DPS)) + \
    ((sizeof(void *)) * (SPFG_MAX_FN_OUT_DPS))

#define ESTIMATED_SIZE_GRX \
    ((ESTIMATED_SIZE_FNX) * (SPFG_MAX_GRID_FNS)) + \
    sizeof(void *) + \
    sizeof(bool) + \
    7 // padding

#define ESTIMATED_RUNTIME_MEMORY \
    ESTIMATED_SIZE_GR + \
    ESTIMATED_SIZE_GRX


#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_block_name_set(spfg_block_name_t *name, const char *ascii);

spfg_err_t _spfg_find_free_gr_dp(spfg_gr_t *gr, uint32_t *idx, spfg_dp_t **dp);
spfg_err_t _spfg_find_free_gr_fn(spfg_gr_t *gr, uint32_t *idx, spfg_fn_t **fn);
spfg_err_t _spfg_find_changed_fnx_in_dp(spfg_fnx_t *fnx, uint32_t *idx);

spfg_err_t _spfg_resolve_gr_fn(spfg_gr_t *gr, spfg_fn_id_t fn_id, spfg_fn_t **fn);
spfg_err_t _spfg_resolve_gr_dp(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp);
spfg_err_t _spfg_resolve_gr_dps(spfg_gr_t *gr, spfg_dp_id_t *dp_ids, spfg_dp_t *dps[], uint8_t length);

spfg_err_t _spfg_ints_have_intersection(int lset[], uint32_t lset_len,
                                        int rset[], uint32_t rset_len,
                                        bool *result);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_UTILS_H__
