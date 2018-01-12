#ifndef __SPFG_INDEX_H__
#define __SPFG_INDEX_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_gr_index_clear(spfg_gr_t *gr);
spfg_err_t _spfg_grx_index_rebuild(spfg_grx_t *grx);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_INDEX_H__
