/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@studend.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 16:01:48 by cmarteau          #+#    #+#             */
/*   Updated: 2022/08/30 07:26:37 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimpleServer.hpp"

HDE::SimpleServer::SimpleServer(int domain, int service, int protocol, std::vector<int> port, u_long interface, int bklog) {
    
    std::vector<int>::iterator  it = port.begin();

    for (; it != port.end(); it++)
        socket.push_back(new ListeningSocket(domain, service, protocol, *it, interface, bklog));
}

std::vector<HDE::ListeningSocket *> HDE::SimpleServer::getSocket() { return socket; }

HDE::SimpleServer::~SimpleServer() // Destructor function   
{
    for (std::vector<ListeningSocket *>::iterator ite = socket.begin(); ite != socket.end(); ite++)
    {
        delete *ite;
    }
}  