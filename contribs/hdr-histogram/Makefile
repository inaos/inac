SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
THIS_LIB = hdr-histogram.a


all: $(OBJS)
	@ar rcs $(THIS_LIB) $(OBJS)

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -I../../include -I../.. -c $<

.PHONY: clean

clean:
	@-rm -f *.o

