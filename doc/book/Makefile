
all: book

convert-images:
	cd images ; make

book: book_pdf book_html

# Full build: multiple runs for correct references...
book_pdf: convert-images
	pdflatex book.tex
	pdflatex book.tex
	pdflatex book.tex

book_html: convert-images
	hevea book.tex
	hevea book.tex
	hevea book.tex
	#latexml --path=/opt/local/share/texmf-texlive-dist/tex/latex/base/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/mdwtools/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/geometry/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/ucs/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/fancyhdr/ --path=/opt/local/share/texmf-texlive-dist/tex/latex/footmisc --destination book.xml book.tex
	#latexmlpost --destination=book.html book.xml

# Only one run for development (faster)
book-dev: convert-images
	pdflatex book.tex

clean:
	rm -f *.log *.aux *.toc *.out *.pdf *.lo[flt]
	rm -f *.haux *.htoc book.image.tex book.html

