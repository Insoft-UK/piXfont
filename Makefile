pixfont:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o build/pixfont -Os -fno-ident -fno-asynchronous-unwind-tables
	strip build/pixfont
	lipo -info build/pixfont
