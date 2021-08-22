clean:
	rm -rf ../../skia/jsGUI out include tools
	
ready:
	cd ../../skia && ln -s ../cpp-custom/skia/jsGUI jsGUI 
	ln -s ../../skia/out/Clang out
	ln -s ../../skia/include include
	ln -s ../../skia/tools tools

hello:
	cat skia.gn BUILD.gn  > ../../skia/BUILD.gn
	cd ../../skia && ninja -C out/Clang :jsGUI