BIN = makeheaders
OBJ = makeheaders.o

all: makeheaders.o
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<
	
.PHONY: clean

clean:
	-rm -f $(OBJ) $(BIN)

	
	
