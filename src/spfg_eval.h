#ifndef __SPFG_EVALUATION_H__
#define __SPFG_EVALUATION_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_reset_cycle(spfg_grx_t *grx);
spfg_err_t _spfg_run_cycle(spfg_grx_t *grx, spfg_ts_t ts, spfg_cycle_cb_t cb, void *udata);

spfg_err_t dp_bool_set(spfg_dp_t *dp, spfg_boolean_t value);
spfg_err_t fn_and_bool_bool_ret_bool_exec(spfg_boolean_t p0, spfg_boolean_t p1, spfg_boolean_t *result);
spfg_err_t fn_and_bool_bool_ret_bool_eval(spfg_fnx_t *fnx, spfg_ts_t ts);
spfg_err_t fnx_eval(spfg_fnx_t *fnx, spfg_ts_t ts);
spfg_err_t grx_fnx_run(spfg_grx_t *grx, spfg_fnx_t *fnx, spfg_ts_t ts);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_EVALUATION_H__
