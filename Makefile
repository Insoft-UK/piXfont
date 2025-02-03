NAME := pxfnt
SRC := src

all: arm64 x86_64

universal:
	g++ -arch arm64 -arch x86_64 -std=c++20 $(SRC)/*.cpp $(SRC)/*/libz.a src/*/libpng.a -o build/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	zip build/$(NAME).zip build/$(NAME) -x "*/.DS_Store"

arm64:
	$(eval TARGET := arm64)
	g++ -arch $(TARGET) -std=c++20 $(SRC)/*.cpp src/$(TARGET)/libpng.a $(SRC)/$(TARGET)/libz.a -o build/$(TARGET)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	zip build/$(NAME)_$(TARGET).zip build/$(TARGET)/$(NAME) -x "*/.DS_Store"
	
x86_64:
	$(eval TARGET := x86_64)
	g++ -arch $(TARGET) -std=c++20 $(SRC)/*.cpp src/$(TARGET)/libpng.a $(SRC)/$(TARGET)/libz.a -o build/$(TARGET)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	zip build/$(NAME)_$(TARGET).zip build/$(TARGET)/$(NAME) -x "*/.DS_Store"
	
clean:
	rm -rf build/*

