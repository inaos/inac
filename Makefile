CC     = /usr/bin/gcc
CFLAGS = -Wall -g -I $(shell$$PWD)/include -DDEBUG
<<<<<<< HEAD
DIRS = contribs doc include src tests
=======
LDFLAGS=
DIRS = contribs doc etc include src scripts tests 

export BIN = $(shell basename $$PWD)
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

all: 
	for i in $(DIRS); do $(MAKE) -C $$i; done
	
.PHONY: clean
<<<<<<< HEAD

=======
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f ChangeLog

test: all
	$(MAKE) test -C tests
<<<<<<< HEAD

rebuild: clean all

dist: test
	$(shell git log --pretty=format:"%cd - %cn: %s" --date=short > ChangeLog)

install: dist NEWS README INSTALL COPYING

=======
	
rebuild: clean all
	
dist: test
	$(shell git log --pretty=format:"%cd - %cn: %s" --date=short > ChangeLog)
	
install: dist NEWS README INSTALL COPYING
	
	
	
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
