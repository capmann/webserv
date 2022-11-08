/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimpleServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@studend.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 16:02:46 by cmarteau          #+#    #+#             */
/*   Updated: 2022/08/30 09:33:43 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SimpleServer_HPP
# define SimpleServer_HPP

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "../Sockets/SimpleSocket.hpp"
#include "../Sockets/BindingSocket.hpp"
#include "../Sockets/ListeningSocket.hpp"
#include "../Sockets/ConnectingSocket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>

namespace HDE {

    class SimpleServer {

        private:
            std::vector<ListeningSocket *> socket;
            virtual void accepter() = 0;
            // virtual void handler() = 0;
            // virtual void responder(std::string content, std::string contentType);

        public:
            SimpleServer(int domain, int service, int protocol, std::vector<int> port, u_long interface, int bklog);
            ~SimpleServer();
            virtual void launch() = 0;
            std::vector<ListeningSocket *> getSocket();
    };
}

#endif
