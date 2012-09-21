MARKDOWN = Markdown.pl --html4tags
SRC = $(wildcard *.md)	
DOCS = $(SRC:%.md=%.html)

all: $(DOCS)

%.html: %.md
	perl ./$(MARKDOWN) $< > $@

install: all
    
.PHONY: clean
	
clean: 
	-rm -f *.html *.pdf *.man

