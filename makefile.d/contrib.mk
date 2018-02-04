
binding_js:
	(cd contrib/binding-js; npm install && npm test)

binding_elm:
	(cd contrib/binding-elm; elm-test)
