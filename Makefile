.PHONY: doc clean web

all:
	@echo "This package don't need to be compiled. Just install it: make install"

install:
	@echo "Installing..."

doc:
	@lua /home/jere/libs/LDoc/ldoc.lua -M -p Malvado -c docs/config.ld .

web:
	cd docs/website && npm run build
	cp -R docs/website/build/malvado/* docs/

clean:
	rm -f doc/index.html
