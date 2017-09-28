WASM_RUNNER = $(NAME).html

WFLAGS = -s WASM=1 -s EXPORT_ALL=1

WASMS = $(SOURCES:%.c=%.wasm)
WASTS = $(SOURCES:%.c=%.wast)
HTMLS = $(SOURCES:%.c=%.html)
JSS = $(SOURCES:%.c=%.js)
ARTIFACTS += $(WASMS) $(WASTS) $(HTMLS) $(JSS)

$(WASM_RUNNER): $(SOURCES) ${ROOT_PATH}/${STAGE_LIB}/libspfg.bc
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^ --shell-file skel.html
