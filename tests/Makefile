DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
SRC += $(wildcard helper*.c)
OBJ = $(SRC:%.c=%.o)
FN  = $(SRC:%.c=%)
BIN = test
# ****************************************************************************
# Linker settings
# ****************************************************************************
CFLAGS := $(filter-out -DINA_LIB=1,$(CFLAGS))
LDFLAGS += $(INAC_HOME_DIR)/src/$(INAC_LIB) $(INAC_LIBS)

ifeq (,$(findstring Windows,$(OS)))
	ifneq (Darwin,$(shell uname -s))
		LDFLAGS += -rdynamic -Wl,-E -lm -lrt -ldl
	else
		LDFLAGS += -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000 
	endif
endif


all: $(OBJ)  main.o
	@$(CC) $(CFLAGS) -o $(BIN) main.o $(OBJ)  $(LDFLAGS)
	@for i in $(DIRS); do $(MAKE) -C $$i $(INAC_BUILD_TYPE); done
	
%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  -c $<

.PHONY: clean

debug: all

release: all

clean:
	@echo cleaning tests...
	@-rm -f *.o $(BIN)  *.gcno *.gcda *.gcov* *.tap
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done


test: all
	./test --format tap

test-appveyor: all
	./test --format=junit
