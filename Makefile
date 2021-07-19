tcp-client:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-client.cpp -o .dist/tcp-socket-client.out && .dist/tcp-socket-client.out
tcp-server:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server.cpp -o .dist/tcp-socket-server.out && .dist/tcp-socket-server.out