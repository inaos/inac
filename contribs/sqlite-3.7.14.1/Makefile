SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o) $(DIRS:%=%.o)
CFLAGS = -Wall -std=c89 -g -DDEBUG
LIB = sqlite.a


all: $(OBJ)
	ar rcs $(LIB) $(OBJ)
%.o: %.c
	$(CC) $(CFLAGS) -v -c  $<
	
.PHONY: clean

clean:
	-rm -f *.o
