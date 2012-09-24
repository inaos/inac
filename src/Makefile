DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
MODULES := $(DIRS)/$(DIRS:%/=%.a)
LIB = libinac.a

CFLAGS = -Wall -std=c89 -g -I../include -DINA_LIB -DDEBUG


all: $(OBJ) 
	for i in $(DIRS); do $(MAKE) -C $$i; done
	$(CC) $(CFLAGS) -v -o $(LIB) $(OBJ) $(MODULES) $(LDFLAGS) 
	
%.o: %.c
	$(CC) $(CFLAGS) -v -c $<
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f *.o
	

