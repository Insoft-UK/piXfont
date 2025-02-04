NAME := pxfnt
SRC := src

all: arm64 x86_64

arm64:
	g++ -arch arm64 -std=c++20 $(SRC)/*.cpp src/arm64/libpng.a $(SRC)/arm64/libz.a -o build/arm64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
x86_64:
	g++ -arch x86_64 -std=c++20 $(SRC)/*.cpp src/x86_64/libpng.a $(SRC)/x86_64/libz.a -o build/x86_64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
clean:
	rm -rf build/*

