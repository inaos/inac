<<<<<<< HEAD
all: makeheaders_tool bstring
=======
LUA_VERSION=5.2.1
LUA_PLATFORM=$(shell uname)
LUA_PLATFORM_Darwin = macosx

all: makeheaders_tool lua bstring
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

makeheaders_tool: 
	$(MAKE) -C makeheaders
	
<<<<<<< HEAD
=======
lua:
	$(MAKE) $(LUA_PLATFORM_$(LUA_PLATFORM)) -C lua-$(LUA_VERSION)
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a

bstring:
	$(MAKE) -C bstring
	
test: all
<<<<<<< HEAD
	
install: all test
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool

clean:
=======
	$(MAKE) test -C lua-$(LUA_VERSION)
	
install: all test
	$(MAKE) install -C lua-$(LUA_VERSION)
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool lua

clean:
	$(MAKE) clean -C lua-$(LUA_VERSION)
>>>>>>> 244ecc23bf8bc8d3f4c0def4823befca126b443a
	$(MAKE) clean -C makeheaders
	