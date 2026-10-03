
PDFLATEX = pdflatex -interaction=nonstopmode -halt-on-error

all: book

convert-images:
	cd images ; make

book: book_pdf book_html

# Full build: multiple runs for correct references...
book_pdf: convert-images
	$(PDFLATEX) book.tex
	$(PDFLATEX) book.tex
	$(PDFLATEX) book.tex

# Full build, and fail if some references are undefined (used by CI)
check: book_pdf
	@if grep -E 'undefined|Rerun to get' book.log ; then \
		echo "Error: undefined references (see book.log)" ; \
		exit 1 ; \
	fi

book_html: convert-images
	hevea book.tex
	hevea book.tex
	hevea book.tex
	#latexml --path=/opt/local/share/texmf-texlive-dist/tex/latex/base/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/mdwtools/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/geometry/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/ucs/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/fancyhdr/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/footmisc --destination book.xml book.tex
	#latexmlpost --destination=book.html book.xml

# Only one run for development (faster)
book-dev: convert-images
	$(PDFLATEX) book.tex

clean:
	rm -f *.log *.aux *.toc *.out *.pdf *.lo[flt]
	rm -f *.haux *.htoc book.image.tex book.html

