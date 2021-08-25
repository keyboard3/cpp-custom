CFLAGS=-std=c++17 -stdlib=libc++
CC=g++
PRE=mkdir -p .dist &&
xmlParser:
	${PRE} ${CC} $(CFLAGS) -g $@.cpp -o .dist/$@ && .dist/$@ < index.xml