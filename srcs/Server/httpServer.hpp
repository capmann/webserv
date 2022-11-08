/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 16:02:31 by cmarteau          #+#    #+#             */
/*   Updated: 2022/10/04 16:35:56 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

#include <stdio.h>
#include "SimpleServer.hpp"
#include "../Request/httpRequest.hpp"
#include "../Response/httpResponse.hpp"
#include "../Config/parseConfig.hpp"

namespace HDE {

    class httpServer : public SimpleServer {

        private:
            char    buffer[3000001];
            std::vector<char>   _requestVec;
            int     newSocket;
            int     _ret;
            void    accepter();
            void    handler();
            void    responder();
            void    handleResponse(std::string content, std::string contentType, int connectListSocket);
            void    responder(std::string content, std::string contentType);

            int     connectList[10]; // array of connected sockets (clients)
            fd_set  socks; // socket fds we want to wake up for, using select()
            int     highSocket; // highest fd

            void    buildSelectList(void);
            void    handle_new_connections(ListeningSocket *socketToHandle);
            void    setNonBlocking(int);
            void    deal_with_data(int);
            int     endOfFile(std::string reqString);

            httpResponse        _response;


        public:
            httpServer(ConfigFile cf);
            ~httpServer() {}
            
            ConfigFile* _ConfigFile; // Configuration file defined in the main
            void        launch();
    };
}

#endif
