#include "ConnectingSocket.hpp"

//Constructor
HDE::ConnectingSocket::ConnectingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface) {

    setconnection(connectToNetwork(getsock(), getaddress()));
    testConnection(getconnection());
}

//Definition of connect to network virtual function
int HDE::ConnectingSocket::connectToNetwork(int sock, struct sockaddr_in address) {
    
    return bind(sock, (struct sockaddr *)&address, sizeof(address));
}