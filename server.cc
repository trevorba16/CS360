#include "server.h"

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::close_socket() {
    close(server_);
}

void
Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
	// loop to handle all requests
	cache = "";
	while (1) {
		// get a request
		string request = get_request(client);
		cache = "";
		// break if client is done or an error occurred
		if (request.empty())
			break;
		// parse request
		Message message = parse_request(request);
		// get more characters if needed
		if (message.needed())
			get_value(client, message);
		// do something
		bool success = handle_message(client, message);
		// break if an error occurred
		if (not success)
			break;
	}
	close(client);
}

string
Server::get_request(int client) {
    string request = cache;
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
		}
		else if (nread == 0) {
			// the socket is closed
			return "";
		}
        request.append(buf_,nread);
    }
    return request;
}

Message
Server::parse_request(string request) {
    int i = 0;
	Message message;
    string command; 
    while (request.at(i) != ' ') {
        command.append(request.at(i));
        i++;
    }
    message.command = command;
    i++;
	string name;
    while (request.at(i) != ' ') {
		name.append(request.at(i));
		i++;
    }
	message.name = name;
	i++;
	string length;
	while (request.at(i) != "\n") {
		length.append(request.at(i));
		i++;
	}
	std::string::size_type sz;
	message.length = std::stoi(length, &sz);
	if (request.at(i) != "\n") {
		//error;
		cout << "Error";
	}
	else {
		int position = request.find("\n");
		position++;
		for (int i = position; i < request.length; i++) {
			cache.append(request.at(i));
		}
	}
	message.value = cache;
}

void Server::get_value()
{
	while (cache.length != message.length) {
		int nread = recv(client, buf_, 1024, 0);
		if (nread < 0) {
			if (errno == EINTR)
				// the socket call was interrupted -- try again
				continue;
			else
				// an error occurred, so break out
				return "";
		}
		else if (nread == 0) {
			// the socket is closed
			return "";
		}
		cache.append(buf_, nread);
	}
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}
