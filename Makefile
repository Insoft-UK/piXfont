all:
	g++ -arch arm64 -std=c++20 src/*.cpp src/libpng.a src/libz.a -o build/pxfnt -Os -fno-ident -fno-asynchronous-unwind-tables
