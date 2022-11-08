/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@studend.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 00:53:05 by cmarteau          #+#    #+#             */
/*   Updated: 2022/09/27 00:30:32 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef parseConfig_HPP
# define parseConfig_HPP

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

class ConfigFile {

    private:

        /* A map that will contain the directive (ex. port, location, root, server_name) 
        and a vector containing the possible values (ex. if there are several server_names)*/

        std::map<std::string, std::vector<std::string> >    _content;
        std::string                                         _directive; // A string containing the directive
        std::vector<std::string>                            _valuesVec; // A vector containing the possible values
        std::string                                         _inSection; // A string containing the section 
        int                                                 _nbVirtualHosts; // The nb of server blocks

        std::string     getSection(std::string const & host, std::string url, std::string const & directive); // A function that returns the Section path where the value is
        std::string     findServer(std::string const & host); // A function to find the first server with the port corresponding to the request

        std::string     defineHost(std::string str);
        bool            isLocalIP(std::string const & listen, std::string const & host); // A function to check if the IP address of the request matches our local IP
        bool            sameServerName(std::vector<std::string> server_name, std::string const & host); // A function to check if the server_name corresponds to the host of the request
        bool            isCandidateServer(std::vector<std::string> servers, std::string const & testServ); // A function to create a list of candidate servers to serve the request

        bool            checkDirective(std::string); // A function to check errors in directives
        void            checkErrorConfig(void); // A function to check errors in the config file 
        std::string     checkIndex(std::string const & host, std::string const & url, std::string const & root); // A function to check which index to pick from the INDEX directive
        bool            checkRoot(void); // A function to check if listen and root directives are present in each virtual host

        //parsing utils
        std::string                 trim(std::string const & source, char const *delims = "\t\r\n");
        std::vector<std::string>    fillVector(std::string const & value);

    public:
            //Constructors
            ConfigFile(std::string const & configFile);
            ~ConfigFile();
         	ConfigFile & operator=(ConfigFile const & rhs);
           
            //Getters
            std::map<std::string, std::vector<std::string> > const & getMap() const;
            std::vector<std::string> const &                         getValue(std::string const & host, std::string const & url, std::string const & directive);
            void                                                     getPorts();
            std::string                                              getErrorPage(std::string const & host, std::string const & error);

            //Useful member functions 
            std::string     findPath(std::string const & host, std::string const & url); // A function to check if the given URL exists, and if yes return the associated root            
            std::string     checkRedirection(int *statusCode, std::string const & host, std::string const & url); // A function to check if there is a redirection, and returns the updated url
            bool            isMethodAllowed(std::string const & host, std::string const & url, std::string const & method); // A function to check if the required method is allowed

            //ports to open in listen mode
            std::vector<int>    portsToOpen;

            //Exception classes 
            class   ValueNotFoundException : public std::exception {

                public:
                    virtual const char *what() const throw() {
                        return ("Error: Value not found");
                    }
            };

            class  ServerNotFoundException : public std::exception {

                public:
                    virtual const char *what() const throw() {
                        return ("Error: Server not found");
                    }
            };
};

#endif
