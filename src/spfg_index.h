#ifndef __SPFG_INDEX_H__
#define __SPFG_INDEX_H__

#include "spfg/spfg.h"
#include "spfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spfg_err_t _spfg_index_clear(spfg_runtime_pvt_t *rt);
spfg_err_t _spfg_index_rebuild(spfg_runtime_pvt_t *rt);

#ifdef __cplusplus
}
#endif

#endif // __SPFG_INDEX_H__
