#include "SimpleSocket.hpp"

HDE::SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface) {
    
    //Define address structure 
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(interface);

    //Establish socket
    sock = socket(domain, service, protocol);
    testConnection(sock);
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    #ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
            perror("setsockopt(SO_REUSEPORT) failed");
    #endif
}

void    HDE::SimpleSocket::testConnection(int itemToTest) {
    
    if (itemToTest < 0) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);}
}

struct sockaddr_in HDE::SimpleSocket::getaddress() {
    return address;
}

int HDE::SimpleSocket::getsock() {
    return sock;
}

int HDE::SimpleSocket::getconnection() {
    return connection;
}

void HDE::SimpleSocket::setconnection(int con) {
    connection = con;
}