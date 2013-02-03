# Either Inkscape or ImageMagick is needed for converting from SVG to PNG.
# Please note that Inkscape gives better results, though.

SHELL=bash  # fix for 'echo -n' on MacOS X

build-makefile:
	@echo "# Auto-generated file" > Makefile2
	@echo ".SUFFIXES: .png .svg" >> Makefile2
	@echo -n "foo: " >> Makefile2
	@for f in *.svg ; do (echo -n `basename "$$f" .svg`'.png ') >> Makefile2 ; done
	@echo >> Makefile2
	@echo "	@echo -n" >> Makefile2
	@echo ".svg.png:" >> Makefile2
	@echo '	echo Converting $$<' >> Makefile2
	@echo '	( inkscape -b "#ffffff" -w 1000 -e $$@ $$< || convert -background none -density 300 -resize 1000x $$< $$@ )' >> Makefile2
	@make -f Makefile2
