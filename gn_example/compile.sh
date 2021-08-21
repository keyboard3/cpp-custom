# ./bin/gn gen -C out
# ninja -C out
# ./out/hello

./bin/gn gen out
ninja -C out tutorial
out/tutorial