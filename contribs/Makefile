all: makeheaders_tool bstring

makeheaders_tool: 
	$(MAKE) -C makeheaders
	

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
	