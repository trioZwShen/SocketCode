all:
	g++ -g common_line.cpp -c -o common_line.o
	g++ -g 07_echo_client_shutdown.cpp -c -o echo_client.o
	g++ -g 07_echo_server_shutdown.cpp -c -o echo_server.o
	g++ -g common_line.o echo_client.o -o echo_client.exe
	g++ -g common_line.o echo_server.o -o echo_server.exe -pthread

clean:
	rm -rf *.o
	rm -rf *.exe
