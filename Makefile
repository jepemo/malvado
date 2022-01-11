.PHONY: doc clean web

current_dir = $(shell pwd)

all:
	@echo "This package don't need to be compiled. Just install it: make install"

install:
	@echo "Installing..."

doc:
	ldoc -M -p Malvado -c  $(current_dir)/docs/config.ld -l  $(current_dir)/docs/ .

web:
	cd docs/website && npm run build
	cp -R docs/website/build/malvado/* docs/

clean:
	rm -f doc/index.html
