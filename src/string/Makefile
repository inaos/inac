DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o) $(DIRS:%=%.o)
CFLAGS = -Wall -std=c89 -g -I../../include -I../../contribs -DINA_LIB -DDEBUG
<<<<<<< HEAD
MODULE = string.a

all: $(OBJ)
	for i in $(DIRS); do $(MAKE) -C $$i; done
	ar rcs $(MODULE) $(OBJ) 
=======
BIN = string.a

all: $(OBJ)
	for i in $(DIRS); do $(MAKE) -C $$i; done
	ar rcs $(BIN) $(OBJ) 
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
		
%.o: %.c
	$(CC) $(CFLAGS) -v -c $<
	
.PHONY: clean

clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
<<<<<<< HEAD
	-rm -f *.o *.a
=======
	-rm -f *.o
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
	

