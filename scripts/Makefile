DIRS = $(shell ls -p |grep /)

all:
	for i in $(DIRS); do $(MAKE) -C $$i; done
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C  $$i; done
	

