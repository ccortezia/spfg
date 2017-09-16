#ifndef __AZJSON_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Error and Info return codes
// ----------------------------------------------------------------------------

#define AZJSON_ERROR_NO                       0
#define AZJSON_ERROR_FAIL                    -1

// ----------------------------------------------------------------------------
// Limits
// ----------------------------------------------------------------------------

#define AZJSON_DEPTH 10

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------

typedef int8_t azjson_err_t;

// ---

typedef enum azjson_token_type {
    TOKEN_NO = 0,
    TOKEN_KS = 1,
    TOKEN_VS = 2,
    TOKEN_VI = 3,
    TOKEN_VT = 4,
    TOKEN_VF = 5,
    TOKEN_VR = 6,
    TOKEN_VN = 7,
} azjson_token_type_t;

typedef struct json_token {
    char *chars;
    uint32_t length;
    azjson_token_type_t type;
} azjson_token_t;

typedef azjson_err_t (*azjson_tokcp_fn_t)(void *, azjson_token_t *);

typedef enum azjson_vtype_e {
    JSON_OBJECT  = 1,
    JSON_ARRAY   = 2,
    JSON_BOOL    = 3,
    JSON_REAL    = 4,
    JSON_STRING  = 5,
    JSON_INTEGER = 6,
} azjson_vtype_t;


typedef struct azjson_spec {
    const char *key;
    uint8_t vtype;
    uint32_t vsize;
    uint32_t voffset;
    uint32_t noffset;
    uint32_t maxitems;
    azjson_tokcp_fn_t cp;
    struct azjson_spec *vspec;
    const bool boundary;
} azjson_spec_t;


// ---

azjson_err_t azjson_import(char *, uint32_t, azjson_spec_t *, void *);
azjson_err_t azjson_strtol(const char *, int32_t *);
azjson_err_t azjson_strtod(const char *, double *);

#ifdef __cplusplus
}
#endif

#endif // __AZJSON_H__
