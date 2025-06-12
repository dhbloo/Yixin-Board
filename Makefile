all: main.c yixin.h
	gcc -Wall -O2 main.c -o Yixin -lm `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` `pkg-config --libs --cflags cairo`

build-windows: main.c yixin.h Yixin.rc
	gcc main.c -O3 -c -o Yixin.o -lm `pkg-config --cflags gtk+-3.0` `pkg-config --cflags cairo` -Wno-incompatible-pointer-types -Wno-int-conversion
	windres -i Yixin.rc -o resource.o
	gcc -s -o Yixin.exe Yixin.o resource.o -Wl,--subsystem,windows -lm `pkg-config --libs gtk+-3.0` `pkg-config --libs cairo`

clean:
	-@rm Yixin Yixin.exe resource.o Yixin.o