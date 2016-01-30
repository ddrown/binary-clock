all: off.h on_i.h

.PHONY: all

# also: -ordered-dither o8x8,32,64,32
%.rgb: %.png
	convert $< -depth 8 -ordered-dither threshold,32,64,32 -gamma 3.0 $@

%.h: %.rgb rgb-to-c
	./rgb-to-c $* <$< >$@
