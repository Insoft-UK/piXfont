all: arm64 x86_64

arm64:
	g++ -arch arm64 -std=c++20 src/*.cpp src/arm64/libpng.a src/arm64/libz.a -o build/pxfnt_arm -Os -fno-ident -fno-asynchronous-unwind-tables

x86_64:
	g++ -arch x86_64 -std=c++20 src/*.cpp src/x86_64/libpng.a src/x86_64/libz.a -o build/pxfnt_x86_64 -Os -fno-ident -fno-asynchronous-unwind-tables

clean:
	rm -rf build/*

