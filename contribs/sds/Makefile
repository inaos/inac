SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o) $(DIRS:%=%.o)
CFLAGS = -Wall -std=c89 -g -DDEBUG
THIS_LIB = sds.a


all: $(OBJ)
	ar rcs $(THIS_LIB) $(OBJ)
%.o: %.c
	$(CC) $(CFLAGS) -v -c  $<
	
.PHONY: clean

clean:
	@-rm -f *.o
