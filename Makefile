NAME := pxfnt
SRC := src
LIB := $(SRC)/lib

all: arm64 x86_64

arm64:
	g++ -arch arm64 -std=c++20 $(SRC)/*.cpp $(LIB)/arm64/libpng.a $(LIB)/arm64/libz.a -o build/arm64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
x86_64:
	g++ -arch x86_64 -std=c++20 $(SRC)/*.cpp $(LIB)/x86_64/libpng.a $(LIB)/x86_64/libz.a -o build/x86_64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
clean:
	rm -rf build/*

