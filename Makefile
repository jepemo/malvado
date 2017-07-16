.PHONY: doc clean

all:
	@echo "This package don't need to be compiled. Just install it: make install"

install:
	@echo "Installing..."

doc:
	@lua /home/jere/libs/LDoc/ldoc.lua -M -p Malvado -c doc/config.ld .

clean:
	rm -f doc/index.html
