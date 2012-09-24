all: makeheaders_tool bstring

makeheaders_tool: 
	$(MAKE) -C makeheaders
	

bstring:
	$(MAKE) -C bstring
	
test: all
	
install: all test
	$(MAKE) install -C makeheaders
	
.PHONY: clean makeheaders_tool

clean:
	$(MAKE) clean -C makeheaders
	