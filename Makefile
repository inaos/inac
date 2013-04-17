CC     = /usr/bin/gcc
CFLAGS = -Wall -g -I $(shell$$PWD)/include -DDEBUG
DIRS = contribs doc include src tests
ARCH := $(shell arch)
ARCH2 = $(shell arch)

all: 
	@echo $(ARCH2)
	@for i in $(DIRS); do $(MAKE) -C $$i; done

.PHONY: clean

clean:
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@rm -f ChangeLog

test: all
	$(MAKE) test -C tests

rebuild: clean all

install: dist NEWS README.md INSTALL COPYING

