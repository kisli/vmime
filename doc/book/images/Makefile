# 'inkscape' is needed for converting from SVG to PNG

build-makefile:
	@echo "# Auto-generated file" > Makefile2
	@echo ".SUFFIXES: .png .svg" >> Makefile2
	@echo -n "foo: " >> Makefile2
	@for f in *.svg ; do (echo -n " $$f" | sed 's/svg/png/g') >> Makefile2 ; done
	@echo >> Makefile2
	@echo "	@echo -n" >> Makefile2
	@echo ".svg.png:" >> Makefile2
	@echo '	echo Converting $$<' >> Makefile2
	@echo '	inkscape -b "#ffffff" -w 1000 -e $$@ $$<' >> Makefile2
	@make -f Makefile2

