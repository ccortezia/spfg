#ifndef __SPFG_JSON_H__
#define __SPFG_JSON_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t spfg_gr_import_json(char *json_str, uint32_t len, spfg_gr_id_t *out_gr_id);
spfg_err_t spfg_gr_export_json(spfg_gr_id_t gr_id, char *output, uint32_t output_len, uint32_t *slen);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_JSON_H__
