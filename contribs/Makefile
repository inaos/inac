LUAJIT_VERSION=2.0.0
LUAJIT_PLATFORM=$(shell uname)
LUAJIT_PLATFORM_Darwin = macosx

SQLITE_VERSION=3.7.14.1
SQLITE_PLATFORM=$(shell uname)
SQLITE_PLATFORM_Darwin = macosx

all: makeheaders_tool bstring sqlite luajit anet

makeheaders_tool:
	$(MAKE) -C makeheaders

luajit:
	$(MAKE) -C luajit-$(LUAJIT_VERSION)

sqlite:
	$(MAKE) -C sqlite-$(SQLITE_VERSION)

bstring:
	$(MAKE) -C bstring

anet:
	$(MAKE) -C anet

sds:
	$(MAKE) -C sds

test: all
	
install: all test
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool bstring sqlite luajit sds

clean:
	$(MAKE) clean -C makeheaders
	$(MAKE) clean -C bstring
	$(MAKE) clean -C anet
	$(MAKE) clean -C sds
	$(MAKE) clean -C luajit-$(LUAJIT_VERSION)
	$(MAKE) clean -C sqlite-$(SQLITE_VERSION)