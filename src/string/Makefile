DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o) $(DIRS:%=%.o)
CFLAGS = -Wall -std=c89 -g -I../../include -I../../contribs -DINA_LIB -DDEBUG -DTRACE_ENABLED
MODULE = string.a

all: $(OBJ)
	for i in $(DIRS); do $(MAKE) -C $$i; done
	ar rcs $(MODULE) $(OBJ) 
		
%.o: %.c
	$(CC) $(CFLAGS) -v -c $<
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f *.o *.a
	

