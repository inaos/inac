CC     = /usr/bin/gcc
CFLAGS = -Wall -g -I $(shell$$PWD)/include -DDEBUG
DIRS = contribs doc include src tests

all: 
	for i in $(DIRS); do $(MAKE) -C $$i; done
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f ChangeLog

test: all
	$(MAKE) test -C tests

rebuild: clean all

dist: test
	$(shell git log --pretty=format:"%cd - %cn: %s" --date=short > ChangeLog)

install: dist NEWS README INSTALL COPYING

