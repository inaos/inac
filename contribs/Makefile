LUA_VERSION=5.2.1
LUA_PLATFORM=$(shell uname)
LUA_PLATFORM_Darwin = macosx

all: makeheaders_tool lua bstring

makeheaders_tool: 
	$(MAKE) -C makeheaders
	
lua:
	$(MAKE) $(LUA_PLATFORM_$(LUA_PLATFORM)) -C lua-$(LUA_VERSION)

bstring:
	$(MAKE) -C bstring
	
test: all
	$(MAKE) test -C lua-$(LUA_VERSION)
	
install: all test
	$(MAKE) install -C lua-$(LUA_VERSION)
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool lua

clean:
	$(MAKE) clean -C lua-$(LUA_VERSION)
	$(MAKE) clean -C makeheaders
	