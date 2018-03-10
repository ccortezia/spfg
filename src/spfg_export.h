#ifndef __SPFG_EXPORT_H__
#define __SPFG_EXPORT_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_rt_import_bin(spfg_runtime_pvt_t *, const void *, uint32_t);
spfg_err_t _spfg_rt_export_bin(spfg_runtime_pvt_t *rt, void *outbuf, uint32_t outbuf_len);

spfg_err_t _spfg_rt_import_json(spfg_runtime_pvt_t *rt, char *json_str, uint32_t len);
spfg_err_t _spfg_rt_export_json(spfg_runtime_pvt_t *rt, char *output, uint32_t output_len, uint32_t *slen);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_EXPORT_H__

