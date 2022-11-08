/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/24 16:05:16 by cmarteau          #+#    #+#             */
/*   Updated: 2022/10/04 17:33:15 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Server/httpServer.hpp"
#include "./Config/parseConfig.hpp"

void    printMapConfig(ConfigFile cf) {
    std::map<std::string, std::vector<std::string> > mapConfig = cf.getMap();
    std::map<std::string, std::vector<std::string> >::iterator it = mapConfig.begin();

    for (; it != mapConfig.end(); it++) {
        std::cout << it->first << " = " << std::endl;
        std::vector<std::string>::iterator ite = it->second.begin();
        for (; ite != it->second.end(); ite++) {
            std::cout << "\t" << ite->c_str() << std::endl;
        }
    }
}

int main(int ac, char **av) {

    std::string path = "srcs/nginx.conf";

    if (ac == 2)
        std::string path = av[1];

    ConfigFile  cf(path);
    HDE::httpServer t(cf);

    std::cout << std::endl;
    return 0;
}