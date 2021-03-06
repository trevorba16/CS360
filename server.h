#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

using namespace std;

class Server {
public:
    Server(int port);
    ~Server();

    void run();
    
private:
    void create();
	void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    Message parse_request(string);
    void get_value();

    bool send_response(int, string);

    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string cache;


	class Message {
		bool needed() {
			if (value.length() == length) {
				return false;
			}
			else return true;
		};

		string command = "";
		string name = "";
		int length = 0;
		string value = "";
	}
};
