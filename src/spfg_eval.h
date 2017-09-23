#ifndef __SPFG_EVALUATION_H__
#define __SPFG_EVALUATION_H__

#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t impl_dp_set_bool(spfg_dp_t *dp, spfg_boolean_t value);
spfg_err_t impl_fn_and_bool_bool_ret_bool(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result);
spfg_err_t eval_fn_and_bool_bool_ret_bool(spfg_fnx_t *fnx, spfg_ts_t ts);
spfg_err_t eval_fnx(spfg_fnx_t *fnx, spfg_ts_t ts);
spfg_err_t spfg_run_fnx(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts);
spfg_err_t spfg_resume_cycle_grx(spfg_grx_t *grx, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_EVALUATION_H__
