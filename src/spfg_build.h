#ifndef __SPFG_BUILD_H__
#define __SPFG_BUILD_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_rt_init(spfg_rt_t *rt, const char *name);
spfg_err_t _spfg_dp_create(spfg_rt_t *rt, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id);
spfg_err_t _spfg_dp_remove(spfg_rt_t *rt, spfg_dp_id_t dp_id);
spfg_err_t _spfg_fn_create(spfg_rt_t *rt,
                              spfg_fn_type_t type,
                              spfg_phase_t phase,
                              spfg_dp_id_t in_dp_ids[],
                              spfg_fn_dp_in_cnt_t in_dp_ids_len,
                              spfg_dp_id_t out_dp_ids[],
                              spfg_fn_dp_out_cnt_t out_dp_ids_len,
                              const char *name,
                              spfg_fn_id_t *fn_id);
spfg_err_t _spfg_fn_remove(spfg_rt_t *rt, spfg_fn_id_t fn_id);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_BUILD_H__
