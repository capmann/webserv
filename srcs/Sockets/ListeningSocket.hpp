#ifndef ListeningSocket_HPP
# define ListeningSocket_HPP

#include "BindingSocket.hpp"

namespace HDE
{
    class ListeningSocket : public BindingSocket {
        
        private:
            int backlog;
            int listening;
    
        public:
            ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int backlog);
            ~ListeningSocket(){}
            void startListening();
    };
}

#endif 