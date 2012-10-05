MD2HTMLCMD = 
SRC = $(wildcard *.md)	
DOCS = $(SRC:%.md=%.html)

all: $(DOCS)

%.html: %.md
	$(MD2HTMLCMD) $< > $@

install: all
    
.PHONY: clean
	
clean: 
	-rm -f *.html

