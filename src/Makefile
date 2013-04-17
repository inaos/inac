DIRS = $(shell ls -p |grep /)

SRC = $(wildcard *.c)
SRC_LUA = $(wildcard *.lua)

OBJ = $(SRC:%.c=%.o)
OBJ += $(SRC_LUA:%.lua=%.o)

MODULES := $(DIRS)$(DIRS:%/=%.a)

LIB = libinac.a

INAC_LUAJIT= ../contribs/luajit/src/luajit -b
INAC_LUAJIT_DIR= ../contribs/luajit/src/jit
LUA_PATH=../contribs/luajit/src/?.lua

CFLAGS = -Wall -std=c89 -g -I../include -I../contribs/anet -I../ -DINA_LIB -DDEBUG -DTRACE_ENABLED


all: $(OBJ) 
	@echo compiling sources...
	@for i in $(DIRS); do $(MAKE) -C $$i; done
	@echo Linking...
	@ar crs $(LIB) $(OBJ)
	@for i in $(DIRS); do ar crs $(LIB) $$i/*.o; done

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

%.o: %.lua
	@echo compile ljit $<
	@$(INAC_LUAJIT) $<  $(basename $<).o

.PHONY: clean

clean:
	@echo cleaning...
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@rm -f *.o *.a

