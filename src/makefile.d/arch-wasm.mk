JS_LIB = lib$(NAME).js
BC_LIB = lib$(NAME).bc

WFLAGS = --pre-js ${TMP_PREJS} --post-js ${TMP_POSTJS} -s RESERVED_FUNCTION_POINTERS=10 -s WASM=1 -s EXPORTED_FUNCTIONS="['_spfg_rt_init', '_spfg_rt_finish', '_spfg_rt_dp_create', '_spfg_rt_dp_remove', '_spfg_rt_fn_create', '_spfg_rt_fn_remove', '_spfg_rt_run_cycle', '_spfg_rt_reset_cycle', '_spfg_rt_export_bin', '_spfg_rt_export_json', '_spfg_rt_import_json', '_spfg_rt_dp_set_bool', '_spfg_rt_dp_get_bool', '_spfg_runtime_size']" -s EXTRA_EXPORTED_RUNTIME_METHODS="['stringToUTF8', 'UTF8ToString', 'setValue', 'getValue', 'addFunction', 'addOnPostRun']"

TMP_PREJS = _prejs.js
TMP_POSTJS = _postjs.js

WASMS = *.wasm
WASTS = *.wast
BCS = *.bc
JSS = *.js
ARTIFACTS += $(JSS) $(BCS) $(WASMS) $(WASTS)

$(JS_LIB): $(SOURCES)
	echo 'var ModuleSPFG = function(Module) {Module = Module || {}; var Module = Module;' > ${TMP_PREJS}
	echo ';return Module;};' > ${TMP_POSTJS}
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^
	rm -f ${TMP_PREJS} ${TMP_POSTJS}

$(BC_LIB): $(SOURCES)
	echo 'var ModuleSPFG = function(Module) {Module = Module || {}; var Module = Module;' > ${TMP_PREJS}
	echo ';return Module;};' > ${TMP_POSTJS}
	$(CC) ${CFLAGS} ${WFLAGS} -o $@ $^
	rm -f ${TMP_PREJS} ${TMP_POSTJS}
