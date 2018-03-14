#include <string.h>
#include <stdio.h>
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"
#include "spfg_build.h"
#include "spfg_eval.h"
#include "spfg_export.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


// -------------------------------------------------------------------------------------------------
// Public Grid Composition API (refactoring)
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_rt_init(spfg_runtime_t *runtime, const char *name)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(runtime, 0, sizeof(spfg_runtime_t));

    return _spfg_rt_init((spfg_runtime_pvt_t *)runtime, name);
}


extern spfg_err_t spfg_rt_finish(spfg_runtime_t *runtime)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(runtime, 0, sizeof(spfg_runtime_t));

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_dp_create(spfg_runtime_t *runtime, spfg_dp_type_t dp_type, const char * name, spfg_dp_id_t *dp_id)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_dp_create((spfg_runtime_pvt_t *)runtime, dp_type, name, dp_id);
}


extern spfg_err_t spfg_dp_remove(spfg_runtime_t *runtime, spfg_dp_id_t dp_id)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_dp_remove((spfg_runtime_pvt_t *)runtime, dp_id);
}


extern spfg_err_t spfg_fn_create(spfg_runtime_t *runtime,
                                    spfg_fn_type_t type,
                                    spfg_phase_t phase,
                                    spfg_dp_id_t in_dp_ids[], uint8_t in_dp_ids_len,
                                    spfg_dp_id_t out_dp_ids[], uint8_t out_dp_ids_len,
                                    const char *name,
                                    spfg_fn_id_t *fn_id)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!name) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (in_dp_ids_len > SPFG_MAX_FN_IN_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    if (out_dp_ids_len > SPFG_MAX_FN_OUT_DPS) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    return _spfg_fn_create((spfg_runtime_pvt_t *)runtime, type, phase,
                              in_dp_ids, in_dp_ids_len,
                              out_dp_ids, out_dp_ids_len,
                              name, fn_id);
}


extern spfg_err_t spfg_fn_remove(spfg_runtime_t *runtime, spfg_fn_id_t fn_id)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_fn_remove((spfg_runtime_pvt_t *)runtime, fn_id);
}

// -------------------------------------------------------------------------------------------------
// Public Grid Evaluation API (refactoring)
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_dp_set_int(spfg_runtime_t *runtime, spfg_dp_id_t dp_id, spfg_int_t value)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return SPFG_ERROR_UNIMPLEMENTED;
}


extern spfg_err_t spfg_dp_set_real(spfg_runtime_t *runtime, spfg_dp_id_t dp_id, spfg_real_t value)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return SPFG_ERROR_UNIMPLEMENTED;
}


extern spfg_err_t spfg_dp_set_bool(spfg_runtime_t *runtime, spfg_dp_id_t dp_id, spfg_boolean_t value)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_dp_set_bool((spfg_runtime_pvt_t *)runtime, dp_id, value);
}


extern spfg_err_t spfg_dp_get_bool(spfg_runtime_t *runtime, spfg_dp_id_t dp_id, spfg_boolean_t *value, spfg_boolean_t *emitted)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!value) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_dp_get_bool((spfg_runtime_pvt_t *)runtime, dp_id, value, emitted);
}


extern spfg_err_t spfg_dp_set_word(spfg_runtime_t *runtime, spfg_dp_id_t dp_id, spfg_word_t word)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return SPFG_ERROR_UNIMPLEMENTED;
}

// ---

extern spfg_err_t spfg_rt_reset_cycle(spfg_runtime_t *runtime)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_rt_reset_cycle((spfg_runtime_pvt_t *)runtime);
}


extern spfg_err_t spfg_rt_run_cycle(spfg_runtime_t *runtime, spfg_ts_t ts, spfg_run_cb_t cb, void *udata)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_rt_run_cycle((spfg_runtime_pvt_t *)runtime, ts, cb, udata);
}

// -------------------------------------------------------------------------------------------------
// Import / Export API (refactoring)
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_rt_import_bin(spfg_runtime_t *runtime, void *data, uint32_t data_len)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!data) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (data_len < sizeof(spfg_gr_exp_t)) {
        return SPFG_ERROR_BAD_PARAM_INVALID_VALUE;
    }

    return _spfg_rt_import_bin((spfg_runtime_pvt_t *)runtime, data, data_len);
}


extern spfg_err_t spfg_rt_export_bin(spfg_runtime_t *runtime, void *outbuf, uint32_t outbuf_len)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!outbuf) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (outbuf_len < sizeof(spfg_gr_exp_t)) {
        return SPFG_ERROR_BUFFER_OVERFLOW;
    }

    return _spfg_rt_export_bin((spfg_runtime_pvt_t *)runtime, outbuf, outbuf_len);
}


extern spfg_err_t spfg_rt_import_json(spfg_runtime_t *runtime, char *json_str, uint32_t len)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!json_str) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(runtime, 0, sizeof(spfg_runtime_t));

    return _spfg_rt_import_json((spfg_runtime_pvt_t *)runtime, json_str, len);
}


extern spfg_err_t spfg_rt_export_json(spfg_runtime_t *runtime, char *output, uint32_t output_len, uint32_t *slen)
{
    if (!runtime) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    if (!output) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    return _spfg_rt_export_json((spfg_runtime_pvt_t *)runtime, output, output_len, slen);
}


// -------------------------------------------------------------------------------------------------
// Inspection API
// -------------------------------------------------------------------------------------------------

extern spfg_err_t spfg_runtime_size(uint32_t *size)
{
    if (!size) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    *size = sizeof(spfg_runtime_t);

    return SPFG_ERROR_NO;
}


extern spfg_err_t spfg_info(spfg_info_t *info)
{
    if (!info) {
        return SPFG_ERROR_BAD_PARAM_NULL_POINTER;
    }

    memset(info, 0, sizeof(spfg_info_t));

    info->version_major = VERSION_MAJOR;
    info->version_minor = VERSION_MINOR;
    info->version_patch = VERSION_PATCH;
    info->max_grid_cnt = SPFG_MAX_GRID_CNT;
    info->max_fn_in_dps = SPFG_MAX_FN_IN_DPS;
    info->max_fn_out_dps = SPFG_MAX_FN_OUT_DPS;
    info->max_grid_fns = SPFG_MAX_GRID_FNS;
    info->max_grid_dps = SPFG_MAX_GRID_DPS;
    info->max_phases = SPFG_MAX_PHASES;

    return SPFG_ERROR_NO;
}
