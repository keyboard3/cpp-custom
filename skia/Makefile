clean:
	rm -rf ../../skia/jsGUI out include tools modules
	rm -rf jsGUI/htmlParser.cpp 
	
ready:
	cd jsGUI && ln -s ../../parser/htmlParser.cpp htmlParser.cpp 
	ln -s ../../skia/out/Clang out
	ln -s ../../skia/include include
	ln -s ../../skia/modules modules
	ln -s ../../skia/tools tools
	cd ../../skia && ln -s ../cpp-custom/skia/jsGUI jsGUI 

donwloadSkia:
	cd ../../ && git clone https://skia.googlesource.com/skia.git && cd skia && python2 tools/git-sync-deps

outclang:
	cd ../../skia && bin/gn gen out/Clang  --args='cc="clang" cxx="clang++"'

browser:
	cat skia.gn BUILD.gn  > ../../skia/BUILD.gn
	cd ../../skia && ninja -C out/Clang :jsGUI