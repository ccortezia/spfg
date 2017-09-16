#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "azjson/azjson.h"

// -------------------------
// JSON PARSING
// -------------------------

/**
 * JSON parsing code adapted from https://github.com/douglascrockford/JSON-c
 **/

#define SPFG_JSON_GOOD 0xBABAB00E
#define __ -1

/*
    These modes can be pushed on the stack.
*/
typedef enum azjson_parser_mode {
    MODE_ARRAY  = 0,
    MODE_DONE   = 1,
    MODE_KEY    = 2,
    MODE_OBJECT = 3
} azjson_parser_mode_t;

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/
typedef enum azjson_parser_char_class {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_ZERO ,  /* 0 */
    C_DIGIT,  /* 123456789 */
    C_LOW_A,  /* a */
    C_LOW_B,  /* b */
    C_LOW_C,  /* c */
    C_LOW_D,  /* d */
    C_LOW_E,  /* e */
    C_LOW_F,  /* f */
    C_LOW_L,  /* l */
    C_LOW_N,  /* n */
    C_LOW_R,  /* r */
    C_LOW_S,  /* s */
    C_LOW_T,  /* t */
    C_LOW_U,  /* u */
    C_ABCDF,  /* ABCDF */
    C_E,      /* E */
    C_ETC,    /* everything else */
    NR_CLASSES
} azjson_parser_char_class_t;

/*
    The state codes.
*/
typedef enum azjson_parser_state {
    GO = 0,  /* start      */
    OK = 1,  /* ok         */
    OB = 2,  /* object     */
    KE = 3,  /* key        */
    CO = 4,  /* colon      */
    VA = 5,  /* value      */
    AR = 6,  /* array      */
    ST = 7,  /* string     */
    ES = 8,  /* escape     */
    U1 = 9,  /* u1         */
    U2 = 10,  /* u2        */
    U3 = 11,  /* u3        */
    U4 = 12,  /* u4        */
    MI = 13,  /* minus     */
    ZE = 14,  /* zero      */
    IN = 15,  /* integer   */
    FR = 16,  /* fraction  */
    FS = 17,  /* fraction  */
    E1 = 18,  /* e         */
    E2 = 19,  /* ex        */
    E3 = 20,  /* exp       */
    T1 = 21,  /* tr        */
    T2 = 22,  /* tru       */
    T3 = 23,  /* true      */
    F1 = 24,  /* fa        */
    F2 = 25,  /* fal       */
    F3 = 26,  /* fals      */
    F4 = 27,  /* false     */
    N1 = 28,  /* nu        */
    N2 = 29,  /* nul       */
    N3 = 30,  /* null      */
    NR_STATES
} azjson_parser_state_t;

/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
static const int8_t ascii_class[128] = {
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      C_WHITE, C_WHITE, __,      __,      C_WHITE, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_ZERO,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E,     C_ABCDF, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_LOW_L, C_ETC,   C_LOW_N, C_ETC,
    C_ETC,   C_ETC,   C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   C_ETC
};

/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.
*/
static const int8_t state_transition_table[NR_STATES][NR_CLASSES] = {
/*
                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,-9,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,-6,__,-5,__,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,-6,__,-5,-7,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*int    IN*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,FS,FS,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fracs  FS*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FS,FS,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__}
};

typedef struct azjson_parser {
    azjson_parser_state_t state;
    azjson_parser_mode_t stack[AZJSON_DEPTH];
    int8_t top;
    char *token_p0;
    char *token_p1;
    char *head;
    uint32_t valid;
} azjson_parser_t;

static azjson_err_t azjson_reject(azjson_parser_t *jc)
{
    jc->valid = 0;
    return AZJSON_ERROR_FAIL;
}

static azjson_err_t azjson_parser_push(azjson_parser_t *jc, azjson_parser_mode_t mode)
{
    if (jc->top + 1 >= AZJSON_DEPTH) {
        return AZJSON_ERROR_FAIL;
    }
    jc->top += 1;
    jc->stack[jc->top] = mode;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_parser_pop(azjson_parser_t *parser, azjson_parser_mode_t mode)
{
    if (parser->top < 0) {
        return AZJSON_ERROR_FAIL;
    }

    if (parser->stack[parser->top] != mode) {
        return AZJSON_ERROR_FAIL;
    }

    parser->top -= 1;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_parser_init(azjson_parser_t *jc)
{
    memset(jc, 0, sizeof(azjson_parser_t));
    jc->valid = SPFG_JSON_GOOD;
    jc->state = GO;
    jc->top = -1;
    azjson_parser_push(jc, MODE_DONE);
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_parser_finish(azjson_parser_t *parser)
{
    if (parser->valid != SPFG_JSON_GOOD) {
        return AZJSON_ERROR_FAIL;
    }

    if (parser->state != OK) {
        return AZJSON_ERROR_FAIL;
    }

    if (azjson_parser_pop(parser, MODE_DONE) != AZJSON_ERROR_NO) {
        return AZJSON_ERROR_FAIL;
    }

    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_parse_char(azjson_parser_t *jc, int32_t next_char)
{
    int next_class, next_state;

    // Determine the character's class.
    if (jc->valid != SPFG_JSON_GOOD) {
        return AZJSON_ERROR_FAIL;
    }

    if (next_char < 0) {
        return azjson_reject(jc);
    }

    if (next_char >= 128) {
        next_class = C_ETC;
    }
    else {
        next_class = ascii_class[next_char];
        if (next_class <= __) {
            return azjson_reject(jc);
        }
    }

    // Get the next state from the state transition table.
    next_state = state_transition_table[jc->state][next_class];

    if (next_state >= 0) {
        // Change the state.
        jc->state = next_state;
        return AZJSON_ERROR_NO;
    }

    // Or perform one of the actions.
    switch (next_state) {

        // empty }
        case -9: {
            if (azjson_parser_pop(jc, MODE_KEY) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = OK;
            break;
        }

        // }
        case -8: {
            if (azjson_parser_pop(jc, MODE_OBJECT) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = OK;
            break;
        }

        // ]
        case -7: {
            if (azjson_parser_pop(jc, MODE_ARRAY) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = OK;
            break;
        }

        // {
        case -6: {
            if (azjson_parser_push(jc, MODE_KEY) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = OB;
            break;
        }

        // [
        case -5: {
            if (azjson_parser_push(jc, MODE_ARRAY) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = AR;
            break;
        }

        // "
        case -4: {
            switch (jc->stack[jc->top]) {
                case MODE_KEY:
                    jc->state = CO;
                    break;
                case MODE_ARRAY:
                case MODE_OBJECT:
                    jc->state = OK;
                    break;
                default:
                    return azjson_reject(jc);
                }
            break;
        }

        // ,
        case -3: {
            switch (jc->stack[jc->top]) {
                case MODE_OBJECT:
                    if (azjson_parser_pop(jc, MODE_OBJECT) != AZJSON_ERROR_NO) {
                        return azjson_reject(jc);
                    }
                    if (azjson_parser_push(jc, MODE_KEY) != AZJSON_ERROR_NO) {
                        return azjson_reject(jc);
                    }
                    jc->state = KE;
                    break;
                case MODE_ARRAY:
                    jc->state = VA;
                    break;
                default:
                    return azjson_reject(jc);
                }
            break;
        }

        // :
        case -2: {
            if (azjson_parser_pop(jc, MODE_KEY) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            if (azjson_parser_push(jc, MODE_OBJECT) != AZJSON_ERROR_NO) {
                return azjson_reject(jc);
            }
            jc->state = VA;
            break;
        }

        // Bad action
        default: {
            return azjson_reject(jc);
        }
    }
    return AZJSON_ERROR_NO;
}

static inline azjson_err_t init_json_token(azjson_token_t *token, azjson_token_type_t type, azjson_parser_t *parser)
{
    if (token) {
        memset(token, 0, sizeof(azjson_token_t));
        token->chars = parser->token_p0;
        token->length = parser->token_p1 - parser->token_p0;
        token->type = type;
    }
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_track_token(azjson_parser_t *prev, azjson_parser_t *curr, char *json_str, uint32_t pos, azjson_token_t *token)
{
    memset(token, 0, sizeof(azjson_token_t));

    // Advance head tracker.
    curr->head = &json_str[pos];

    // Detect lower token boundary for json key.
    if ((prev->state == OB || prev->state == KE) && curr->state == ST) {
        curr->token_p0 = curr->head + 1;
    }

    // Detect lower token boundary for json string value.
    else if ((prev->state == AR || prev->state == VA) && curr->state == ST) {
        curr->token_p0 = curr->head + 1;
    }

    // Detect lower token boundary for json non-string value.
    else if ((prev->state == AR || prev->state == VA) &&
        (curr->state == MI || curr->state == ZE || curr->state == IN || curr->state == F1 || curr->state == T1 || curr->state == N1)) {
        curr->token_p0 = curr->head;
    }

    // Detect upper token boundary for json key.
    else if (prev->state == ST && curr->state == CO) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_KS, curr);
    }

    // Detect upper token boundary for json string value.
    else if (prev->state == ST && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VS, curr);
    }

    // Detect upper token boundary for json integer value.
    else if ((prev->state == IN || prev->state == ZE) && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VI, curr);
    }

    // Detect upper token boundary for json integer value.
    else if (prev->state == FS && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VR, curr);
    }

    // Detect upper token boundary for json true value.
    else if (prev->state == T3 && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VT, curr);
    }

    // Detect upper token boundary for json false value.
    else if (prev->state == F4 && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VF, curr);
    }

    // Detect upper token boundary for json null value.
    else if ((prev->state == N3) && (curr->state == KE || curr->state == VA || curr->state == OK)) {
        curr->token_p1 = curr->head;
        (void) init_json_token(token, TOKEN_VN, curr);
    }

    return AZJSON_ERROR_NO;
}


// -------------------------
// JSON TRAIL
// -------------------------

typedef enum json_trail_type {
    JSON_TRAIL_ROOT   = 0,
    JSON_TRAIL_OBJECT = 1,
    JSON_TRAIL_ARRAY  = 2,
    JSON_TRAIL_LEAF   = 3,
} azjson_trail_type_t;

typedef struct json_trail_item {
    azjson_trail_type_t type;
    azjson_token_t parent_key;
    uint32_t parent_idx;
    uint32_t child_count;
    bool is_array_item;
} azjson_trail_item_t;

typedef struct json_trail {
    azjson_trail_item_t stack[AZJSON_DEPTH];
    int8_t top;
    azjson_token_t last_key;
} azjson_trail_t;


static azjson_err_t azjson_trail_append_obj_value(azjson_trail_t *trail, azjson_trail_type_t type, azjson_token_t *key)
{
    if (trail->top + 1 >= AZJSON_DEPTH) {
        return AZJSON_ERROR_FAIL;
    }
    trail->top += 1;
    memcpy(&trail->stack[trail->top].parent_key, key, sizeof(azjson_token_t));
    trail->stack[trail->top].type = type;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_trail_append_array_child(azjson_trail_t *trail, azjson_trail_type_t type, uint32_t idx)
{
    if (trail->top + 1 >= AZJSON_DEPTH) {
        return AZJSON_ERROR_FAIL;
    }
    trail->top += 1;
    trail->stack[trail->top].is_array_item = true;
    trail->stack[trail->top].parent_idx = idx;
    trail->stack[trail->top].type = type;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_trail_set_root(azjson_trail_t *trail, azjson_trail_type_t type)
{
    if (trail->top + 1 >= AZJSON_DEPTH) {
        return AZJSON_ERROR_FAIL;
    }
    trail->top += 1;
    trail->stack[trail->top].is_array_item = false;
    trail->stack[trail->top].parent_idx = 0;
    trail->stack[trail->top].type = type;
    return AZJSON_ERROR_NO;
}


static azjson_err_t azjson_trail_pop(azjson_trail_t *trail)
{
    if (trail->top - 1 < 0) {
        return AZJSON_ERROR_FAIL;
    }

    memset(&trail->stack[trail->top], 0, sizeof(azjson_trail_item_t));

    trail->top -= 1;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_trail_init(azjson_trail_t *trail)
{
    memset(trail, 0, sizeof(azjson_trail_t));
    trail->stack[0].type = JSON_TRAIL_ROOT;
    return AZJSON_ERROR_NO;
}

static inline void json_trail_clean_last_key(azjson_trail_t *trail)
{
    memset(&trail->last_key, 0, sizeof(azjson_token_t));
}

static azjson_err_t azjson_track_trail(azjson_parser_t *prev, azjson_parser_t *curr, azjson_token_t *token, azjson_trail_t *trail)
{

    azjson_parser_mode_t prev_mode = prev->stack[prev->top];
    azjson_parser_mode_t curr_mode = curr->stack[curr->top];

    // Memorize last trail key.
    if (token->type == TOKEN_KS) {
        memcpy(&trail->last_key, token, sizeof(azjson_token_t));
    }

    // If a leaf trail item is at the top, pop it to make room for a potential sibbling.
    if (trail->stack[trail->top].type == JSON_TRAIL_LEAF) {
        if (azjson_trail_pop(trail) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
    }

    // Detect transition from root to object
    if (prev_mode == MODE_DONE && curr->state == OB) {
        if (trail->last_key.type != TOKEN_NO) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_set_root(trail, JSON_TRAIL_OBJECT) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
    }

    // Detect transition from parent object to child object
    else if (prev_mode == MODE_OBJECT && curr_mode == MODE_KEY && prev->state == VA) {
        if (trail->last_key.type != TOKEN_KS) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_obj_value(trail, JSON_TRAIL_OBJECT, &trail->last_key) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        json_trail_clean_last_key(trail);
    }

    // Detect leaf in object
    else if (prev_mode == MODE_OBJECT && token->type != TOKEN_KS && token->type != TOKEN_NO) {
        if (trail->last_key.type != TOKEN_KS) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_obj_value(trail, JSON_TRAIL_LEAF, &trail->last_key) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        json_trail_clean_last_key(trail);
    }

    // Detect transition from parent object to child array
    else if (prev_mode == MODE_OBJECT && curr_mode == MODE_ARRAY && curr->state == AR) {
        if (trail->last_key.type != TOKEN_KS) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_obj_value(trail, JSON_TRAIL_ARRAY, &trail->last_key) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        json_trail_clean_last_key(trail);
    }

    // Detect transition from root to array
    else if (prev_mode == MODE_DONE && curr->state == AR) {
        if (trail->last_key.type != TOKEN_NO) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_set_root(trail, JSON_TRAIL_ARRAY) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
    }

    // Detect leaf in array
    else if (prev_mode == MODE_ARRAY && token->type != TOKEN_KS && token->type != TOKEN_NO) {
        if (trail->last_key.type != TOKEN_NO) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_array_child(trail, JSON_TRAIL_LEAF, trail->stack[trail->top].child_count) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        // Update the container's leaf counter.
        trail->stack[trail->top - 1].child_count += 1;
    }

    // Detect transition from parent array to child object
    else if (prev_mode == MODE_ARRAY && (prev->state == AR || prev->state == VA) && curr->state == OB && (prev->top == curr->top - 1)) {
        if (trail->last_key.type != TOKEN_NO) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_array_child(trail, JSON_TRAIL_OBJECT, trail->stack[trail->top].child_count) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        // Update the container's leaf counter.
        trail->stack[trail->top - 1].child_count += 1;
    }

    // Detect transition from parent array to child array
    else if (prev_mode == MODE_ARRAY && curr_mode == MODE_ARRAY && (prev->top == curr->top - 1)) {
        if (trail->last_key.type != TOKEN_NO) {
            return AZJSON_ERROR_FAIL;
        }
        if (azjson_trail_append_array_child(trail, JSON_TRAIL_ARRAY, trail->stack[trail->top].child_count) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
        // Update the container's leaf counter.
        trail->stack[trail->top - 1].child_count += 1;
    }

    // Stack down to follow a parser stack down, unless the top holds a leaf.
    if (trail->stack[trail->top].type != JSON_TRAIL_LEAF) {
        while (trail->top > curr->top) {
            if (azjson_trail_pop(trail) != AZJSON_ERROR_NO) {
                return AZJSON_ERROR_FAIL;
            }
        }
    }

    return AZJSON_ERROR_NO;
}


// ------------------------
// JSON LOADING
// ------------------------

typedef struct azjson_loader_item {
    azjson_trail_item_t *trail_item;
    azjson_spec_t *node_spec;
    void *vaddr;
    void *naddr;
} azjson_loader_item_t;

typedef struct azjson_loader {
    azjson_loader_item_t stack[AZJSON_DEPTH];
    int8_t top;
    azjson_trail_t *trail;
    azjson_spec_t *root_spec;
    void *storage;
} azjson_loader_t;

static azjson_err_t azjson_loader_push(azjson_loader_t *loader, azjson_trail_item_t *trail_item, azjson_spec_t *node_spec)
{
    if (loader->top + 1 >= AZJSON_DEPTH) {
        return AZJSON_ERROR_FAIL;
    }

    // Calculates current base address.
    void *vaddr = loader->stack[loader->top].vaddr ? loader->stack[loader->top].vaddr : loader->storage;

    loader->top += 1;
    loader->stack[loader->top].node_spec = node_spec;
    loader->stack[loader->top].trail_item = trail_item;

    // Calculates address increment for array members.
    if (trail_item->is_array_item) {

        uint32_t mul = trail_item->parent_idx;
        uint32_t inc = 0;

        // Determines address increment from element size.
        switch (node_spec->vtype) {
            case JSON_ARRAY: {
                inc = node_spec->vsize;
                break;
            }
            case JSON_OBJECT: {
                inc = node_spec->vsize;
                break;
            }
            case JSON_STRING: {
                inc = node_spec->vsize;
                break;
            }
            case JSON_INTEGER: {
                inc = sizeof(uint32_t);
                break;
            }
            case JSON_REAL: {
                inc = sizeof(double);
                break;
            }
            case JSON_BOOL: {
                inc = sizeof(bool);
                break;
            }
            default: {
                return AZJSON_ERROR_FAIL;
            }
        }

        loader->stack[loader->top].vaddr = vaddr + (mul * inc);
    }

    // Calculates address increment for object members.
    else {
        loader->stack[loader->top].vaddr = vaddr + node_spec->voffset;
    }

    // Updates the address for the array counter storage.
    if (loader->stack[loader->top].node_spec->vtype == JSON_ARRAY) {
        loader->stack[loader->top].naddr = vaddr + node_spec->noffset;
    }
    else {
        loader->stack[loader->top].naddr = NULL;
    }

    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_loader_pop(azjson_loader_t *loader)
{
    if (loader->top - 1 < 0) {
        return AZJSON_ERROR_FAIL;
    }

    loader->stack[loader->top].trail_item = NULL;
    loader->stack[loader->top].node_spec = NULL;
    loader->stack[loader->top].vaddr = NULL;
    loader->stack[loader->top].naddr = NULL;
    loader->top -= 1;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_loader_init(azjson_loader_t *loader, azjson_trail_t *trail, azjson_spec_t *root_spec, void *storage)
{
    memset(loader, 0, sizeof(azjson_loader_t));
    loader->storage = storage;
    loader->trail = trail;
    loader->root_spec = root_spec;
    loader->stack[0].trail_item = NULL;
    loader->stack[0].node_spec = NULL;
    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_loader_sync(azjson_loader_t *loader, azjson_token_t *token)
{
    // Stack up.
    while (loader->top < loader->trail->top) {

        azjson_spec_t *top_spec = loader->stack[loader->top].node_spec;
        azjson_trail_item_t *next_trail_item = &loader->trail->stack[loader->top + 1];

        if (loader->top == 0) {
            if (azjson_loader_push(loader, next_trail_item, &loader->root_spec[0]) != AZJSON_ERROR_NO) {
                return AZJSON_ERROR_FAIL;
            }
            continue;
        }

        switch (top_spec->vtype) {
            case JSON_OBJECT: {
                if (next_trail_item->parent_key.type != TOKEN_KS) {
                    return AZJSON_ERROR_FAIL;
                }
                bool found = false;
                for (int ki = 0; !top_spec->vspec[ki].boundary; ki++) {
                    if (!strncmp(top_spec->vspec[ki].key, next_trail_item->parent_key.chars, next_trail_item->parent_key.length)) {

                        azjson_vtype_t expected_type = top_spec->vspec[ki].vtype;

                        switch (token->type) {
                            case TOKEN_VF: {
                                if (expected_type != JSON_BOOL) {
                                    continue;
                                }
                                break;
                            }
                            case TOKEN_VT: {
                                if (expected_type != JSON_BOOL) {
                                    continue;
                                }
                                break;
                            }
                            case TOKEN_VI: {
                                if (expected_type != JSON_INTEGER) {
                                    continue;
                                }
                                break;
                            }
                            case TOKEN_VR: {
                                if (expected_type != JSON_REAL) {
                                    continue;
                                }
                                break;
                            }
                            case TOKEN_VN: {
                                // Currently, a null token is rejected regardless of the expected type.
                                // TODO: add an explicit 'nullable' property to the spec to check here.
                                continue;
                            }
                            case TOKEN_VS: {
                                if (expected_type != JSON_STRING) {
                                    continue;
                                }
                                break;
                            }
                            default: {
                                break;
                            }
                        }

                        if (azjson_loader_push(loader, next_trail_item, &top_spec->vspec[ki]) != AZJSON_ERROR_NO) {
                            return AZJSON_ERROR_FAIL;
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return AZJSON_ERROR_FAIL;
                }
                break;
            }
            case JSON_ARRAY: {
                if (azjson_loader_push(loader, next_trail_item, &top_spec->vspec[0]) != AZJSON_ERROR_NO) {
                    return AZJSON_ERROR_FAIL;
                }
                break;
            }
            default:
                return AZJSON_ERROR_NO;
        }
    }

    // Stack down.
    while (loader->top > loader->trail->top) {
        if (azjson_loader_pop(loader) != AZJSON_ERROR_NO) {
            return AZJSON_ERROR_FAIL;
        }
    }


    return AZJSON_ERROR_NO;
}

azjson_err_t azjson_strtod(const char *nptr, double *output)
{
    char *endptr = NULL;

    errno = 0;

    *output = strtod(nptr, &endptr);

    if (endptr == nptr) {
        return AZJSON_ERROR_FAIL;
    }

    if (errno < 0) {
        return AZJSON_ERROR_FAIL;
    }

    return AZJSON_ERROR_NO;
}

azjson_err_t azjson_strtol(const char *nptr, int32_t *output)
{
    char *endptr = NULL;

    errno = 0;

    *output = strtol(nptr, &endptr, 10);

    if (endptr == nptr) {
        return AZJSON_ERROR_FAIL;
    }

    if (errno < 0) {
        return AZJSON_ERROR_FAIL;
    }

    return AZJSON_ERROR_NO;
}

static azjson_err_t azjson_loader_extract(azjson_loader_t *loader, azjson_token_t *token)
{
    if (token->type == TOKEN_NO) {
        return AZJSON_ERROR_NO;
    }

    // Verify specified array boundaries to prevent overflow.
    if (loader->stack[loader->top].trail_item->is_array_item) {
        void *count_p = loader->stack[loader->top - 1].naddr;
        if (*(uint32_t *)count_p + 1 > loader->stack[loader->top - 1].node_spec->maxitems) {
            return AZJSON_ERROR_FAIL;
        }
    }

    void *value_p = loader->stack[loader->top].vaddr;
    azjson_vtype_t expected_type = loader->stack[loader->top].node_spec->vtype;

    switch (token->type) {
        case TOKEN_VF: {
            if (expected_type != JSON_BOOL) {
                return AZJSON_ERROR_FAIL;
            }
            break;
        }
        case TOKEN_VT: {
            if (expected_type != JSON_BOOL) {
                return AZJSON_ERROR_FAIL;
            }
            break;
        }
        case TOKEN_VI: {
            if (expected_type != JSON_INTEGER) {
                return AZJSON_ERROR_FAIL;
            }
            break;
        }
        case TOKEN_VR: {
            if (expected_type != JSON_REAL) {
                return AZJSON_ERROR_FAIL;
            }
            break;
        }
        case TOKEN_VN: {
            // Currently, a null token is rejected regardless of the expected type.
            // TODO: add an explicit 'nullable' property to the spec to check here.
            return AZJSON_ERROR_FAIL;
        }
        case TOKEN_VS: {
            if (expected_type != JSON_STRING) {
                return AZJSON_ERROR_FAIL;
            }
            break;
        }
        default: {
            break;
        }
    }

    azjson_tokcp_fn_t custom_copy_fn = loader->stack[loader->top].node_spec->cp;

    // Copy value data to dereferenced destination using custom injected copy strategies.
    if (custom_copy_fn && (custom_copy_fn(value_p, token) != AZJSON_ERROR_NO)) {
        return AZJSON_ERROR_FAIL;
    }

    // Copy value data to dereferenced destination using standard copy strategies.
    else if (custom_copy_fn == NULL) {

        switch (token->type) {
            case TOKEN_VF: {
                *((bool *)value_p) = false;
                break;
            }
            case TOKEN_VT: {
                *((bool *)value_p) = true;
                break;
            }
            case TOKEN_VI: {
                if (azjson_strtol(token->chars, (int32_t *)value_p) != AZJSON_ERROR_NO) {
                    return AZJSON_ERROR_FAIL;
                }
                break;
            }
            case TOKEN_VR: {
                if (azjson_strtod(token->chars, (double *)value_p) != AZJSON_ERROR_NO) {
                    return AZJSON_ERROR_FAIL;
                }
                break;
            }
            case TOKEN_VN: {
                // Unreachable condition.
                // Currently unsupported.
                return AZJSON_ERROR_FAIL;
            }
            case TOKEN_VS: {
                uint32_t vsize = loader->stack[loader->top].node_spec->vsize;
                if (token->length > vsize) {
                    return AZJSON_ERROR_FAIL;
                }
                memset(value_p, 0, vsize);
                strncpy((char *)value_p, token->chars, token->length);
                ((char *)value_p)[vsize - 1] = '\0';
                break;
            }
            default: {
                break;
            }
        }
    }

    // Update array extraction counter.
    if (loader->stack[loader->top].trail_item->is_array_item) {
        void *count_p = loader->stack[loader->top - 1].naddr;
        *(uint32_t *)count_p = loader->stack[loader->top].trail_item->parent_idx + 1;
    }

    return AZJSON_ERROR_NO;
}


extern azjson_err_t azjson_import(char *json_str, uint32_t len, azjson_spec_t *spec, void *storage)
{
    azjson_token_t token;
    azjson_trail_t trail;
    azjson_loader_t loader;
    azjson_parser_t curr_parser;
    azjson_parser_t prev_parser;

    if (azjson_trail_init(&trail) != AZJSON_ERROR_NO) {
        fprintf(stderr, "azjson_trail_init failure\n");
        return AZJSON_ERROR_FAIL;
    }

    if (azjson_loader_init(&loader, &trail, spec, storage) != AZJSON_ERROR_NO) {
        fprintf(stderr, "azjson_loader_init failure\n");
        return AZJSON_ERROR_FAIL;
    }

    if (azjson_parser_init(&curr_parser) != AZJSON_ERROR_NO) {
        fprintf(stderr, "azjson_parser_init failure\n");
        return AZJSON_ERROR_FAIL;
    }

    for (uint32_t idx = 0; idx < strnlen(json_str, len); idx++) {

        memcpy(&prev_parser, &curr_parser, sizeof(azjson_parser_t));

        if (azjson_parse_char(&curr_parser, json_str[idx]) != AZJSON_ERROR_NO) {
            fprintf(stderr, "azjson_parse_char failure [char:%d]\n", idx);
            return AZJSON_ERROR_FAIL;
        }

        if (azjson_track_token(&prev_parser, &curr_parser, json_str, idx, &token) != AZJSON_ERROR_NO) {
            fprintf(stderr, "azjson_track_token failure [char:%d]\n", idx);
            return AZJSON_ERROR_FAIL;
        }

        if (azjson_track_trail(&prev_parser, &curr_parser, &token, &trail) != AZJSON_ERROR_NO) {
            fprintf(stderr, "azjson_track_trail failure [char:%d]\n", idx);
            return AZJSON_ERROR_FAIL;
        }

        if (azjson_loader_sync(&loader, &token) != AZJSON_ERROR_NO) {
            fprintf(stderr, "azjson_loader_sync failure [char:%d]\n", idx);
            return AZJSON_ERROR_FAIL;
        }

        if (azjson_loader_extract(&loader, &token) != AZJSON_ERROR_NO) {
            fprintf(stderr, "azjson_loader_extract failure [char:%d]\n", idx);
            return AZJSON_ERROR_FAIL;
        }
    }

    if (azjson_parser_finish(&curr_parser) != AZJSON_ERROR_NO) {
        fprintf(stderr, "azjson_parser_finish failure\n");
        return AZJSON_ERROR_FAIL;
    }

    return AZJSON_ERROR_NO;
}
