
all: convert-images book

convert-images:
	cd images ; make

# Full build: multiple runs for correct references...
book:
	pdflatex book.tex
	pdflatex book.tex
	pdflatex book.tex

# Only one run for development (faster)
book-dev:
	pdflatex book.tex

clean:
	rm -f *.toc *.aux *.lof *.log *.lol *.pdf

