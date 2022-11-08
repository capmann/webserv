#ifndef BindingSocket_HPP
# define BindingSocket_HPP

#include "SimpleSocket.hpp"

namespace HDE
{
    class BindingSocket : public SimpleSocket {
        
        public:
            BindingSocket(int domain, int service, int protocol, int port, u_long interface);
            int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif 