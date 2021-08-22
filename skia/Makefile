clean:
	rm -rf ../../skia/jsGUI out include tools
	
ready:
	cd ../../skia && ln -s ../cpp-custom/skia/jsGUI jsGUI 
	ln -s ../../skia/out/Clang out
	ln -s ../../skia/include include
	ln -s ../../skia/tools tools

donwloadSkia:
	cd ../../ && git clone https://skia.googlesource.com/skia.git && cd skia && python2 tools/git-sync-deps

outclang:
	cd ../../skia && bin/gn gen out/Clang  --args='cc="clang" cxx="clang++"'

browser:
	cat skia.gn BUILD.gn  > ../../skia/BUILD.gn
	cd ../../skia && ninja -C out/Clang :jsGUI