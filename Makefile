NAME := pxfnt

all: arm64 x86_64

arm64:
	g++ -arch arm64 -std=c++20 src/*.cpp src/arm64/libpng.a src/arm64/libz.a -o build/arm64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	zip build/$(NAME)_arm64.zip build/arm64/$(NAME) -x "*/.DS_Store"
	
x86_64:
	g++ -arch x86_64 -std=c++20 src/*.cpp src/x86_64/libpng.a src/x86_64/libz.a -o build/x86_64/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	zip build/$(NAME)_x86_64.zip build/x86_64/$(NAME) -x "*/.DS_Store"
	
clean:
	rm -rf build/*

