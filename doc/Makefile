<<<<<<< HEAD
MD2HTMLCMD = 
=======
MARKDOWN = Markdown.pl --html4tags
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
SRC = $(wildcard *.md)	
DOCS = $(SRC:%.md=%.html)

all: $(DOCS)

%.html: %.md
<<<<<<< HEAD
	$(MD2HTMLCMD) $< > $@
=======
	perl ./$(MARKDOWN) $< > $@
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

install: all
    
.PHONY: clean
	
clean: 
<<<<<<< HEAD
	-rm -f *.html
=======
	-rm -f *.html *.pdf *.man
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

