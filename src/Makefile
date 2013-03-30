DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
MODULES := $(DIRS)$(DIRS:%/=%.a)
LIB = libinac.a

CFLAGS = -Wall -std=c89 -g -I../include -I../contribs/anet -I../ -DINA_LIB -DDEBUG -DTRACE_ENABLED


all: $(OBJ) 
	for i in $(DIRS); do $(MAKE) -C $$i; done
	ar crs $(LIB) $(OBJ)
	for i in $(DIRS); do ar crs $(LIB) $$i/*.o; done
	
%.o: %.c
	$(CC) $(CFLAGS) -v -c $<
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f *.o *.a

