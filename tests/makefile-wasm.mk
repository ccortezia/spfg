WASM_RUNNER = $(NAME).html

WFLAGS = -s WASM=1 -s EXPORT_ALL=1

WASMS = $(SOURCES:%.c=%.wasm)
WASTS = $(SOURCES:%.c=%.wast)
HTMLS = $(SOURCES:%.c=%.html)
JSS = $(SOURCES:%.c=%.js)

$(WASM_RUNNER): $(SOURCES) ${ROOT_PATH}/${STAGE_LIB}/libspfg.bc
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^ --shell-file skel.html

.PHONY: wasm_clean
wasm_clean:
	$(RM) $(WASMS) $(WASTS) $(HTMLS) $(JSS)
