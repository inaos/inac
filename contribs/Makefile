LUAJIT_PLATFORM=$(shell uname)
LUAJIT_PLATFORM_Darwin = macosx

SQLITE_PLATFORM=$(shell uname)
SQLITE_PLATFORM_Darwin = macosx

all: makeheaders_tool bstring sqlite luajit anet skiplist

makeheaders_tool:
	@$(MAKE) -C makeheaders

luajit:
	@$(MAKE) -C luajit

sqlite:
	@$(MAKE) -C sqlite

bstring:
	@$(MAKE) -C bstring

anet:
	@$(MAKE) -C anet

skiplist:
	@$(MAKE) -C skiplist

sds:
	@$(MAKE) -C sds

test: all
	
install: all test
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool bstring sqlite luajit sds anet skiplist

clean:
	$(MAKE) clean -C makeheaders
	$(MAKE) clean -C bstring
	$(MAKE) clean -C skiplist
	$(MAKE) clean -C anet
	$(MAKE) clean -C sds
	$(MAKE) clean -C luajit
	$(MAKE) clean -C sqlite
