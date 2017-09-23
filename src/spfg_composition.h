#ifndef __SPFG_COMPOSITION_H__
#define __SPFG_COMPOSITION_H__

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t spfg_dp_gr_create(spfg_gr_t *gr, unsigned int dp_idx, spfg_dp_type_t dp_type, const char *name);
spfg_err_t spfg_fn_reindex(spfg_grx_t *grx, spfg_fnx_t *fnx);

spfg_err_t validate_fn_signature(spfg_dp_t *in_dps[], uint8_t in_dps_len,
    spfg_dp_t *out_dps[], uint8_t out_dps_len,
    spfg_dp_type_t *in_dp_types, uint8_t in_dp_types_len,
    spfg_dp_type_t *out_dp_types, uint8_t out_dp_types_len,
    const char *fn_name);

spfg_err_t spfg_fn_validate(spfg_fn_type_t type,
    spfg_dp_t *in_dps[], uint8_t in_dps_len,
    spfg_dp_t *out_dps[], uint8_t out_dps_len,
    const char *fn_name);

spfg_err_t spfg_fn_gr_create(spfg_gr_t *gr, int fn_idx,
    spfg_fn_type_t type,
    spfg_phase_t phase,
    spfg_dp_id_t *in_dp_ids, uint8_t in_dp_ids_len,
    spfg_dp_id_t *out_dp_ids, uint8_t out_dp_ids_len,
    const char *name);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_COMPOSITION_H__
