
binding_js:
	(cd contrib/binding-js; npm install && npm test)

binding_elm:
	(cd contrib/binding-elm; elm-test)

binding_py:
	(cd contrib/binding-py && LD_LIBRARY_PATH=lib/spfg python3 setup.py test)
