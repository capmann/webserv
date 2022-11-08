#ifndef ConnectingSocket_HPP
# define ConnectingSocket_HPP

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "SimpleSocket.hpp"

namespace HDE {

    class ConnectingSocket : public SimpleSocket {
        
        public:
            ConnectingSocket(int domain, int service, int protocol, int port, u_long interface);
            int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif 