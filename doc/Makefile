MD2HTMLCMD = 
SRC = $(wildcard *.md)	
DOCS = $(SRC:%.md=%.html)

all:

%.html: %.md
	$(MD2HTMLCMD) $< > $@

install: all

debug: install

release: install

.PHONY: clean
	
clean:
	@echo cleaning docs.. 
	@-rm -f *.html

