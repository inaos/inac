SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o) $(DIRS:%=%.o)
CFLAGS = -Wall -std=c89 -g
LIB = bstring.a


all: $(OBJ)
	ar rcs $(LIB) $(OBJ)
%.o: %.c
	$(CC) $(CFLAGS) -v -c -DBSTRLIB_NOVSNP $<
	
.PHONY: clean

clean:
	-rm -f *.o
