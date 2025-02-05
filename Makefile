NAME := pxfnt
SRC := src
ARCH := $(shell arch)

all: arm64 x86_64

arm64:
	g++ -arch arm64 -std=c++20 -I$(SRC)/libpng/include -I$(SRC)/libz/include $(SRC)/*.cpp $(SRC)/libpng/lib/arm64/libpng.a $(SRC)/libz/lib/arm64/libz.a -o build/arm64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
x86_64:
	g++ -arch x86_64 -std=c++20 -I$(SRC)/libpng/include -I$(SRC)/libz/include $(SRC)/*.cpp $(SRC)/libpng/lib/x86_64/libpng.a $(SRC)/libz/lib/x86_64/libz.a -o build/x86_64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
clean:
	rm -rf build/*
	
install:
	cp build/$(ARCH)/$(NAME) /usr/local/bin/$(NAME)
	
uninstall:
	rm /usr/local/bin/$(NAME)

