all:
	g++ common_line.cpp -c -o common_line.o
	g++ 04_echo_client.cpp -c -o echo_client.o
	g++ 04_echo_server.cpp -c -o echo_server.o
	g++ common_line.o echo_client.o -o echo_client
	g++ common_line.o echo_server.o -o echo_server

clean:
	rm -rf *.o
