#ifndef __SPFG_EVAL_H__
#define __SPFG_EVAL_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// spfg_err_t dp_bool_set(spfg_dp_t *dp, spfg_boolean_t value);
// spfg_err_t fn_and_bool_bool_ret_bool_exec(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result);
// spfg_err_t fn_and_bool_bool_ret_bool_eval(spfg_fnx_t *fnx, spfg_ts_t ts);
// spfg_err_t fnx_eval(spfg_fnx_t *fnx, spfg_ts_t ts);
// spfg_err_t grx_fnx_run(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts);
// spfg_err_t fnx_changed_dps_clear(spfg_fnx_t *fnx);

spfg_err_t _spfg_rt_reset_cycle(spfg_runtime_pvt_t *rt);
spfg_err_t _spfg_rt_run_cycle(spfg_runtime_pvt_t *rt, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata);
spfg_err_t _spfg_dp_set_bool(spfg_runtime_pvt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t value);
spfg_err_t _spfg_dp_get_bool(spfg_runtime_pvt_t *rt, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted);


#ifdef __cplusplus
}
#endif

#endif // __SPFG_EVAL_H__
