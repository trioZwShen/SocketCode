all:
	g++ -g common_line.cpp -c -o common_line.o
	g++ -g 05_echo_client.cpp -c -o echo_client.o
	g++ -g 05_echo_server.cpp -c -o echo_server.o
	g++ -g common_line.o echo_client.o -o echo_client.exe
	g++ -g common_line.o echo_server.o -o echo_server.exe

clean:
	rm -rf *.o
	rm -rf *.exe
