
all:
	elm make Main.elm --output app.js

.PHONY: clean serve

serve:
	python -m SimpleHTTPServer 8000

clean:
	$(RM) app.js
