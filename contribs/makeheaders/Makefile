THIS_BIN = makeheaders
OBJ = makeheaders.o

all: makeheaders.o
	@$(CC) $(CFLAGS) -o $(THIS_BIN) $(OBJ) $(LDFLAGS)

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	@-rm -f $(OBJ) $(BIN)
