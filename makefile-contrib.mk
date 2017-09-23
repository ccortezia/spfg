
binding_js: $(STAGE_LIB)/libspfg.wasm
	(cd contrib/binding-js; npm install && npm test)
