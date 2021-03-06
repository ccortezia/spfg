#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "azjson/azjson.h"
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_build.h"
#include "spfg_utils.h"
#include "spfg_index.h"


// -------------------------------------------------------------------------------------------------
// Binary Import / Export API
// -------------------------------------------------------------------------------------------------

spfg_err_t _spfg_rt_import_bin(spfg_rt_t *rt, void *data, uint32_t data_len)
{
    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)data;

    memcpy(&rt->gr.dps, &grxp->data.dps, sizeof(grxp->data.dps));

    memcpy(&rt->gr.fns, &grxp->data.fns, sizeof(grxp->data.fns));

    memcpy(&rt->gr.ctl, &grxp->data.ctl, sizeof(spfg_gr_ctl_t));

    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_rt_export_bin(spfg_rt_t *rt, void *outbuf, uint32_t outbuf_len)
{
    spfg_gr_exp_t *grxp = (spfg_gr_exp_t *)outbuf;

    memset(grxp, 0, sizeof(spfg_gr_exp_t));

    memcpy(&grxp->data, &rt->gr, sizeof(spfg_gr_t));

    return SPFG_ERROR_NO;
}


// -------------------------------------------------------------------------------------------------
// JSON Import / Export API
// -------------------------------------------------------------------------------------------------

static azjson_err_t spfg_dp_value_cp(void *target, azjson_token_t *token) {
    switch (token->type) {
        case TOKEN_VI: return azjson_strtol(token->chars, &((spfg_dp_value_t *)target)->integer);
        case TOKEN_VR: return azjson_strtod(token->chars, &((spfg_dp_value_t *)target)->real);
        case TOKEN_VT: (((spfg_dp_value_t *)target)->boolean) = true; break;
        case TOKEN_VF: (((spfg_dp_value_t *)target)->boolean) = false; break;
        default: return AZJSON_ERROR_FAIL;
    }
    return AZJSON_ERROR_NO;
}


static azjson_spec_t int_spec[] = {
    {
        .vtype = JSON_INTEGER,
    },
    {.boundary = true}
};

static azjson_spec_t gr_fns_fn_spec[] = {
    {
        .key = "id",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, id),
    },
    {
        .key = "type",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, type),
    },
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_fn_t, name),
        .vsize = sizeof(((spfg_fn_t *)0)->name)
    },
    {
        .key = "phase",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_fn_t, phase),
    },
    {
        .key = "in_dp_ids",
        .vtype = JSON_ARRAY,
        .vspec = int_spec,
        .voffset = offsetof(spfg_fn_t, in_dp_ids),
        .noffset = offsetof(spfg_fn_t, in_dp_ids_len),
        .maxitems = sizeof(((spfg_fn_t *)0)->in_dp_ids) / sizeof(spfg_dp_id_t)
    },
    {
        .key = "out_dp_ids",
        .vtype = JSON_ARRAY,
        .vspec = int_spec,
        .voffset = offsetof(spfg_fn_t, out_dp_ids),
        .noffset = offsetof(spfg_fn_t, out_dp_ids_len),
        .maxitems = sizeof(((spfg_fn_t *)0)->out_dp_ids) / sizeof(spfg_dp_id_t)
    },
    {.boundary = true}
};

static azjson_spec_t gr_fns_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_fns_fn_spec,
        .vsize = sizeof(spfg_fn_t),
    },
    {.boundary = true}
};

static azjson_spec_t gr_dps_dp_spec[] = {
    {
        .key = "id",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, id),
    },
    {
        .key = "type",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, type),
    },
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_dp_t, name),
        .vsize = sizeof(((spfg_dp_t *)0)->name)
    },
    {
        .key = "emitted",
        .vtype = JSON_BOOL,
        .voffset = offsetof(spfg_dp_t, emitted),
    },
    {
        .key = "value",
        .vtype = JSON_BOOL,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {
        .key = "value",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {
        .key = "value",
        .vtype = JSON_REAL,
        .voffset = offsetof(spfg_dp_t, value),
        .cp = spfg_dp_value_cp
    },
    {.boundary = true}
};

static azjson_spec_t gr_dps_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_dps_dp_spec,
        .vsize = sizeof(spfg_dp_t),
    },
    {.boundary = true}
};

static azjson_spec_t gr_ctl_spec[] = {
    {
        .key = "curr_fn_idx",
        .vtype = JSON_INTEGER,
        .voffset = offsetof(spfg_gr_ctl_t, curr_fn_idx)
    },
    {.boundary = true}
};

static azjson_spec_t gr_spec[] = {
    {
        .key = "name",
        .vtype = JSON_STRING,
        .voffset = offsetof(spfg_gr_t, name),
        .vsize = sizeof(((spfg_gr_t *)0)->name)
    },
    {
        .key = "fns",
        .vtype = JSON_ARRAY,
        .vspec = gr_fns_spec,
        .voffset = offsetof(spfg_gr_t, fns),
        .noffset = offsetof(spfg_gr_t, fns_cnt),
        .maxitems = sizeof(((spfg_gr_t *)0)->fns) / sizeof(spfg_fn_t)
    },
    {
        .key = "dps",
        .vtype = JSON_ARRAY,
        .vspec = gr_dps_spec,
        .voffset = offsetof(spfg_gr_t, dps),
        .noffset = offsetof(spfg_gr_t, dps_cnt),
        .maxitems = sizeof(((spfg_gr_t *)0)->dps) / sizeof(spfg_dp_t)
    },
    {
        .key = "ctl",
        .vtype = JSON_OBJECT,
        .vspec = gr_ctl_spec,
        .voffset = offsetof(spfg_gr_t, ctl),
    },
    {.boundary = true}
};

static azjson_spec_t root_spec[] = {
    {
        .vtype = JSON_OBJECT,
        .vspec = gr_spec,
    },
    {.boundary = true}
};


spfg_err_t _spfg_rt_import_json(spfg_rt_t *rt, char *json_str, uint32_t len)
{
    memset(rt, 0, sizeof(spfg_rt_t));

    memset(&rt->gr, 0, sizeof(spfg_gr_t));

    if (azjson_import(json_str, len, root_spec, &rt->gr) != AZJSON_ERROR_NO) {
        return SPFG_ERROR_FAIL;
    }

    (void) _spfg_index_clear(rt);

    return SPFG_ERROR_NO;
}


static spfg_err_t sappend(char *output, size_t output_len, uint32_t *vcnt, uint32_t *rcnt, const char *fmt, ...)
{
    int result;
    va_list ap;
    va_start(ap, fmt);

    result = vsnprintf(&output[*rcnt], output_len - *rcnt, fmt, ap);

    if (result < 0) {
        // TODO: properly handle error
        return SPFG_ERROR_FAIL;
    }

    *vcnt += result;
    *rcnt = *vcnt > output_len - 1 ? output_len - 1 : *vcnt;

    va_end (ap);
    return SPFG_ERROR_NO;
}


spfg_err_t _spfg_rt_export_json(spfg_rt_t *rt, char *output, uint32_t output_len, uint32_t *slen)
{
    uint32_t rcnt = 0;
    uint32_t vcnt = 0;

    spfg_gr_t *gr = &rt->gr;

    memset(output, 0, output_len);

    sappend(output, output_len, &vcnt, &rcnt, "{");
    sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->name.chars);

    sappend(output, output_len, &vcnt, &rcnt, "\"fns\": [");
    for (uint32_t idx = 0; idx < SPFG_MAX_GRID_FNS; idx++) {
        if (!gr->fns[idx].name.chars[0]) {
            continue;
        }
        sappend(output, output_len, &vcnt, &rcnt, "{");
        sappend(output, output_len, &vcnt, &rcnt, "\"id\": %d, ", gr->fns[idx].id);
        sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->fns[idx].name.chars);
        sappend(output, output_len, &vcnt, &rcnt, "\"type\": %d, ", gr->fns[idx].type);
        sappend(output, output_len, &vcnt, &rcnt, "\"phase\": %d, ", gr->fns[idx].phase);
        sappend(output, output_len, &vcnt, &rcnt, "\"in_dp_ids\": [");

        for (uint32_t _idx = 0; _idx < gr->fns[idx].in_dp_ids_len; _idx++) {
            sappend(output, output_len, &vcnt, &rcnt, "%d", gr->fns[idx].in_dp_ids[_idx]);
            if (_idx + 1 < gr->fns[idx].in_dp_ids_len) {
                sappend(output, output_len, &vcnt, &rcnt, ", ");
            }
        }
        sappend(output, output_len, &vcnt, &rcnt, "], ");

        sappend(output, output_len, &vcnt, &rcnt, "\"out_dp_ids\": [");
        for (uint32_t _idx = 0; _idx < gr->fns[idx].out_dp_ids_len; _idx++) {
            sappend(output, output_len, &vcnt, &rcnt, "%d", gr->fns[idx].out_dp_ids[_idx]);
            if (_idx + 1 < gr->fns[idx].out_dp_ids_len) {
                sappend(output, output_len, &vcnt, &rcnt, ", ");
            }
        }
        sappend(output, output_len, &vcnt, &rcnt, "]");

        sappend(output, output_len, &vcnt, &rcnt, "}");
        if (idx < SPFG_MAX_GRID_FNS && gr->fns[idx + 1].name.chars[0]) {
            sappend(output, output_len, &vcnt, &rcnt, ", ");
        }
    }

    sappend(output, output_len, &vcnt, &rcnt, "], ");
    sappend(output, output_len, &vcnt, &rcnt, "\"dps\": [");

    for (uint32_t idx = 0; idx < SPFG_MAX_GRID_DPS; idx++) {
        if (!gr->dps[idx].name.chars[0]) {
            continue;
        }

        sappend(output, output_len, &vcnt, &rcnt, "{");
        sappend(output, output_len, &vcnt, &rcnt, "\"id\": %d, ", gr->dps[idx].id);
        sappend(output, output_len, &vcnt, &rcnt, "\"name\": \"%s\", ", gr->dps[idx].name.chars);
        sappend(output, output_len, &vcnt, &rcnt, "\"type\": %d, ", gr->dps[idx].type);

        switch (gr->dps[idx].type) {
            case SPFG_DP_INT: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %d, ", gr->dps[idx].value.integer);
                break;
            }
            case SPFG_DP_REAL: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %.5G, ", gr->dps[idx].value.real);
                break;
            }
            case SPFG_DP_BOOL: {
                sappend(output, output_len, &vcnt, &rcnt, "\"value\": %s, ", gr->dps[idx].value.boolean ? "true" : "false");
                break;
            }
            default: {
                return SPFG_ERROR_FAIL;
            }
        }

        sappend(output, output_len, &vcnt, &rcnt, "\"emitted\": %s", gr->dps[idx].emitted ? "true" : "false");
        sappend(output, output_len, &vcnt, &rcnt, "}");

        if (idx < SPFG_MAX_GRID_DPS && gr->dps[idx + 1].name.chars[0]) {
            sappend(output, output_len, &vcnt, &rcnt, ", ");
        }
    }

    sappend(output, output_len, &vcnt, &rcnt, "], ");

    sappend(output, output_len, &vcnt, &rcnt, "\"ctl\": {");
    sappend(output, output_len, &vcnt, &rcnt, "\"curr_fn_idx\": %d", gr->ctl.curr_fn_idx);
    sappend(output, output_len, &vcnt, &rcnt, "}");

    sappend(output, output_len, &vcnt, &rcnt, "}");

    if (slen) {
        *slen = rcnt;
    }

    return SPFG_ERROR_NO;
}
