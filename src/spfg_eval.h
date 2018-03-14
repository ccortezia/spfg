#ifndef __SPFG_EVAL_H__
#define __SPFG_EVAL_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_rt_reset_cycle(spfg_runtime_pvt_t *rt);
spfg_err_t _spfg_rt_run_cycle(spfg_runtime_pvt_t *rt, spfg_ts_t ts, spfg_run_cb_t cb, void *udata);
spfg_err_t _spfg_dp_set_bool(spfg_runtime_pvt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t value);
spfg_err_t _spfg_dp_get_bool(spfg_runtime_pvt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted);


#ifdef __cplusplus
}
#endif

#endif // __SPFG_EVAL_H__
