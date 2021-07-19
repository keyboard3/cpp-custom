tcp-client:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-client.cpp -o .dist/tcp-socket-client.out && .dist/tcp-socket-client.out
tcp-server:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server.cpp -o .dist/tcp-socket-server.out && .dist/tcp-socket-server.out
tcp-server-fork:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server-fork.cpp -o .dist/tcp-socket-server-fork.out && .dist/tcp-socket-server-fork.out
udp-client:
	mkdir -p .dist && g++ -std=c++17 ./udp-socket-client.cpp -o .dist/udp-socket-client.out && .dist/udp-socket-client.out
udp-server:
	mkdir -p .dist && g++ -std=c++17 ./udp-socket-server.cpp -o .dist/udp-socket-server.out && .dist/udp-socket-server.out