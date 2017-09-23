#ifndef __SPFG_UTILS_H__
#define __SPFG_UTILS_H__

#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t spfg_block_name_create(const char *ascii, spfg_block_name_t *name);
spfg_err_t find_global_gr(spfg_gr_id_t gr_id, unsigned int *idx);
spfg_err_t resolve_global_gr(spfg_gr_id_t gr_id, spfg_gr_t **gr);
spfg_err_t resolve_gr_dp(spfg_gr_t *gr, spfg_dp_id_t dp_id, spfg_dp_t **dp);
spfg_err_t resolve_gr_fn(spfg_gr_t *gr, spfg_fn_id_t fn_id, spfg_fn_t **fn);
spfg_err_t find_free_global_gr(unsigned int *idx, spfg_gr_t **gr);
spfg_err_t find_free_gr_dp(spfg_gr_t *gr, unsigned int *idx, spfg_dp_t **dp);
spfg_err_t find_free_gr_fn(spfg_gr_t *gr, unsigned int *idx, spfg_fn_t **fn);
spfg_err_t find_changed_input_for_fnx(spfg_fnx_t *fnx, unsigned int *idx);
spfg_err_t clear_changed_input_for_fnx(spfg_fnx_t *fnx);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_UTILS_H__
