.PHONY: doc clean

all:
	@echo "This package don't need to be compiled. Just install it: make install"

install:
	@echo "Installing..."

doc:
	@lua /home/jere/libs/LDoc/ldoc.lua -p Malvado malvado/core.lua malvado/draw.lua

clean:
	rm -f doc/index.html
