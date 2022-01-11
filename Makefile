.PHONY: doc

current_dir = $(shell pwd)

doc:
	ldoc -M -p Malvado -c  $(current_dir)/docs/config.ld -l  $(current_dir)/docs/ .
