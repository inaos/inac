DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
SRC += $(wildcard helper*.c)
OBJ = $(SRC:%.c=%.o)
FN  = $(SRC:%.c=%)
BIN = test
TOOLS = ullc_monitor service_mgr ilt iml bench_timer
TOOLS_OBJS = $(TOOLS:=.o)

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


all: $(OBJ)  main.o $(TOOLS)
	@$(CC) $(CFLAGS) -o $(BIN) main.o $(OBJ)  $(LDFLAGS) 

$(TOOLS): $(TOOLS_OBJS)
	@for i in $(TOOLS); do $(CC) $(CFLAGS) -o $$i $$i.o $(LDFLAGS); done;
	
%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  -c $<

.PHONY: clean

debug: all

release: all

clean:
	@echo cleaning tests...
	@-rm -f *.o $(BIN) $(TOOLS) *.gcno *.gcda *.gcov* *.tap

test: all
	./test --tap
