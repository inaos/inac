DIRS = $(shell ls -p |grep /)
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
TOOLS  = $(SRC:%.c=%)

# ****************************************************************************
# Linker settings
# ****************************************************************************
LDFLAGS += ../src/$(INAC_LIB) $(INAC_LIBS)

ifeq (,$(findstring Windows,$(OS)))
	ifneq (Darwin,$(shell uname -s))
		LDFLAGS += -rdynamic -Wl,-E -lm -lrt -ldl
	else
		LDFLAGS += -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000 
	endif
endif


all: $(OBJ) 
	echo Building tools
	echo $(TOOLS)
	@for i in $(TOOLS); do $(CC) $(CFLAGS) -o $$i $$i.o $(LDFLAGS); done;
	
%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  -c $<

.PHONY: clean

debug: all

release: all

clean:
	@echo cleaning tests...
	@-rm -f *.o $(TOOLS) *.gcno *.gcda *.gcov* *.tap

test: all

