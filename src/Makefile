DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
<<<<<<< HEAD
MODULES := $(DIRS)$(DIRS:%/=%.a)
LIB = libinac.a
=======
MODULES := $(DIRS)/$(DIRS:%/=%.a)
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

CFLAGS = -Wall -std=c89 -g -I../include -DINA_LIB -DDEBUG


all: $(OBJ) 
	for i in $(DIRS); do $(MAKE) -C $$i; done
<<<<<<< HEAD
	ar rcs $(LIB) $(OBJ) $(MODULES)
=======
	$(CC) $(CFLAGS) -v -o $(BIN) $(OBJ) $(MODULES) $(LDFLAGS) 
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

