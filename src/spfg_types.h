#ifndef __SPFG_TYPES_H__
#define __SPFG_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spfg_block_name {
    char chars[SPFG_BLOCK_NAME_MAX_LENGTH];
} spfg_block_name_t;

typedef union {
    spfg_real_t real;
    spfg_int_t integer;
    spfg_boolean_t boolean;
} spfg_dp_value_t;

typedef struct spfg_dp {
    spfg_dp_id_t id;
    spfg_dp_type_t type;
    spfg_block_name_t name;
    spfg_boolean_t emitted;
    spfg_dp_value_t value;
} spfg_dp_t;

typedef struct spfg_fn {
    spfg_fn_id_t id;
    spfg_fn_type_t type;
    spfg_block_name_t name;
    spfg_phase_t phase;
    spfg_dp_id_t in_dp_ids[SPFG_MAX_FN_IN_DPS];
    spfg_fn_dp_in_cnt_t in_dp_ids_len;
    spfg_dp_id_t out_dp_ids[SPFG_MAX_FN_OUT_DPS];
    spfg_fn_dp_out_cnt_t out_dp_ids_len;
} spfg_fn_t;


typedef struct spfg_gr_ctl {
    spfg_phase_t curr_phase;
    spfg_gr_fn_cnt_t curr_fn_idx;
}  spfg_gr_ctl_t;

typedef struct spfg_gr {
    spfg_gr_id_t id;
    spfg_block_name_t name;
    spfg_dp_t dps[SPFG_MAX_GRID_DPS];
    spfg_gr_dp_cnt_t dps_cnt;
    spfg_fn_t fns[SPFG_MAX_GRID_FNS];
    spfg_gr_fn_cnt_t fns_cnt;
    spfg_gr_ctl_t ctl;
} spfg_gr_t;


typedef struct spfg_fnx {
    spfg_fn_t *fn;
    spfg_dp_t *in_dps[SPFG_MAX_FN_IN_DPS];
    spfg_dp_t *out_dps[SPFG_MAX_FN_OUT_DPS];
} spfg_fnx_t;

typedef struct spfg_grx {
    spfg_gr_t *gr;
    spfg_fnx_t fnx[SPFG_MAX_GRID_FNS];
    bool is_valid;
} spfg_grx_t;


typedef struct spfg_gr_exph {
} spfg_gr_exph_t;

typedef struct spfg_gr_exp {
    spfg_gr_exph_t header;
    spfg_gr_t data;
} spfg_gr_exp_t;


#ifdef __cplusplus
}
#endif

#endif // __SPFG_TYPES_H__
