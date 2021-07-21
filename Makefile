tcp-client:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-client.cpp -o .dist/tcp-socket-client.out && .dist/tcp-socket-client.out
tcp-server:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server.cpp -o .dist/tcp-socket-server.out && .dist/tcp-socket-server.out
tcp-server-process:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server-process.cpp -o .dist/tcp-socket-server-process.out && .dist/tcp-socket-server-process.out
tcp-server-thread:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server-thread.cpp -o .dist/tcp-socket-server-thread.out && .dist/tcp-socket-server-thread.out
tcp-server-select:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server-select.cpp -o .dist/tcp-socket-server-select.out && .dist/tcp-socket-server-select.out
tcp-server-kqueue:
	mkdir -p .dist && g++ -std=c++17 ./tcp-socket-server-kqueue.cpp -o .dist/tcp-socket-server-kqueue.out && .dist/tcp-socket-server-kqueue.out
udp-client:
	mkdir -p .dist && g++ -std=c++17 ./udp-socket-client.cpp -o .dist/udp-socket-client.out && .dist/udp-socket-client.out
udp-server:
	mkdir -p .dist && g++ -std=c++17 ./udp-socket-server.cpp -o .dist/udp-socket-server.out && .dist/udp-socket-server.out