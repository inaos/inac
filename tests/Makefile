DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
SRC += $(wildcard helper*.c)
OBJ = $(SRC:%.c=%.o)
FN  = $(SRC:%.c=%)
BIN = test
MONITOR_BIN = ullc_monitor

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


all: $(OBJ)  main.o ullc_monitor
	@$(CC) $(CFLAGS) -o $(BIN) main.o $(OBJ)  $(LDFLAGS) 

ullc_monitor: ullc_monitor.o
	@$(CC) $(CFLAGS) -o $(MONITOR_BIN) ullc_monitor.o $(LDFLAGS) 
	
%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  -c $<

.PHONY: clean

debug: all

release: all

clean:
	@echo cleaning tests...
	@-rm -f *.o test

test: all
	./test
