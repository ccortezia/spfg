#ifndef __SPFG_COMPOSITION_H__
#define __SPFG_COMPOSITION_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_gr_create(const char *name, spfg_gr_id_t *gr_id);
spfg_err_t _spfg_gr_remove(spfg_gr_t *gr);
spfg_err_t _spfg_dp_create(spfg_gr_t *gr, spfg_dp_type_t dp_type, const char *name, spfg_dp_id_t *dp_id);
spfg_err_t _spfg_dp_remove(spfg_gr_t *gr, spfg_dp_t *dp);
spfg_err_t _spfg_fn_create(spfg_gr_t *gr,
                           spfg_fn_type_t type,
                           spfg_phase_t phase,
                           spfg_dp_id_t in_dp_ids[], uint8_t in_dp_ids_len,
                           spfg_dp_id_t out_dp_ids[], uint8_t out_dp_ids_len,
                           const char *name,
                           spfg_fn_id_t *fn_id);
spfg_err_t _spfg_fn_remove(spfg_gr_t *gr, spfg_fn_t *fn);

spfg_err_t fn_signature_validate(spfg_dp_t *in_dps[], uint8_t in_dps_len,
                                 spfg_dp_t *out_dps[], uint8_t out_dps_len,
                                 spfg_dp_type_t *in_dp_types, uint8_t in_dp_types_len,
                                 spfg_dp_type_t *out_dp_types, uint8_t out_dp_types_len,
                                 const char *fn_name);

spfg_err_t fn_validate(spfg_fn_type_t type,
                       spfg_dp_t *in_dps[], uint8_t in_dps_len,
                       spfg_dp_t *out_dps[], uint8_t out_dps_len,
                       const char *fn_name);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_COMPOSITION_H__
