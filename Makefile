all: main.c yixin.h
	gcc main.c -O3 -o Yixin -lm `pkg-config --libs --cflags gtk+-3.0` -Wno-incompatible-pointer-types -Wno-int-conversion

debug: main.c yixin.h
	gcc main.c -g -o Yixin -lm `pkg-config --libs --cflags gtk+-3.0` -Wno-incompatible-pointer-types -Wno-int-conversion
	
build-windows: main.c yixin.h Yixin.rc
	gcc main.c -O3 -c -o Yixin.o -lm `pkg-config --cflags gtk+-3.0` -Wno-incompatible-pointer-types -Wno-int-conversion
	windres -i Yixin.rc -o resource.o
	gcc -s -o Yixin.exe Yixin.o resource.o -Wl,--subsystem,windows -lm `pkg-config --libs gtk+-3.0`

build-macos: main.c yixin.h
	@echo "Building Yixin.app..."
	gcc main.c -O3 -o Yixin -lm `pkg-config --libs --cflags gtk+-3.0` -Wno-incompatible-pointer-types -Wno-int-conversion
	
	@echo "Creating app bundle structure..."
	mkdir -p Yixin.app/Contents/{MacOS,Resources}
	cp Yixin Yixin.app/Contents/MacOS/
	
	@echo "Creating icon.icns if needed..."
	if [ ! -f icon.icns ]; then \
		mkdir -p icon.iconset; \
		sips -s format png icon.ico --out icon.png 2>/dev/null || true; \
		for size in 16 32 128 256 512; do \
			sips -z $$size $$size icon.png --out icon.iconset/icon_$${size}x$${size}.png 2>/dev/null || true; \
			if [ $$size != 512 ]; then \
				sips -z $$((size*2)) $$((size*2)) icon.png --out icon.iconset/icon_$${size}x$${size}@2x.png 2>/dev/null || true; \
			fi; \
		done; \
		iconutil -c icns icon.iconset 2>/dev/null || true; \
		rm -f icon.png; \
		rm -rf icon.iconset; \
	fi
	
	@echo "Copying icon to Resources..."
	if [ -f icon.icns ]; then \
		cp icon.icns Yixin.app/Contents/Resources/; \
	fi
	
	@echo "Creating Info.plist..."
	echo '<?xml version="1.0" encoding="UTF-8"?>\
		<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\
		<plist version="1.0">\
		<dict>\
			<key>CFBundleExecutable</key>\
			<string>Yixin</string>\
			<key>CFBundleIconFile</key>\
			<string>icon.icns</string>\
			<key>CFBundleIdentifier</key>\
			<string>com.yixin.board</string>\
			<key>CFBundleInfoDictionaryVersion</key>\
			<string>6.0</string>\
			<key>CFBundleName</key>\
			<string>Rapfi-Yixin</string>\
			<key>CFBundlePackageType</key>\
			<string>APPL</string>\
			<key>CFBundleVersion</key>\
			<string>1.0</string>\
			<key>CFBundleShortVersionString</key>\
			<string>1.0</string>\
			<key>LSMinimumSystemVersion</key>\
			<string>10.9</string>\
			<key>NSHighResolutionCapable</key>\
			<true/>\
			<key>NSPrincipalClass</key>\
			<string>NSApplication</string>\
			<key>NSMainNibFile</key>\
			<string></string>\
			<key>LSUIElement</key>\
			<false/>\
		</dict>\
		</plist>' > Yixin.app/Contents/Info.plist
	
	@echo "Creating PkgInfo..."
	echo "APPL????" > Yixin.app/Contents/PkgInfo
	
	@echo "Yixin.app has been built. Run with 'open Yixin.app'"

clean:
	-@rm -f Yixin Yixin.exe resource.o Yixin.o
	-@rm -rf Yixin.app Yixin.dSYM icon.icns