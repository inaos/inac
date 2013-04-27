MD2HTMLCMD = 
SRC = $(wildcard *.md)	
DOCS = $(SRC:%.md=%.html)

all:

%.html: %.md
	$(MD2HTMLCMD) $< > $@

install: all
    
.PHONY: clean
	
clean:
	@echo cleaning docs.. 
	@-rm -f *.html

