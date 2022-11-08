/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: threiss <threiss@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/20 00:28:11 by cmarteau          #+#    #+#             */
/*   Updated: 2022/10/04 17:35:03 by threiss          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parseConfig.hpp"
#include "colormod.hpp"
#include "../Response/httpResponse.hpp"

Color::Modifier		red(Color::FG_RED);
Color::Modifier		cyan(Color::FG_CYAN);
Color::Modifier		def(Color::FG_DEFAULT);
Color::Modifier		green(Color::FG_GREEN);
Color::Modifier		magenta(Color::FG_LIGHT_MAGENTA);

// ================================== CONSTRUCTOR: PARSE + STORE THE ELEMENTS IN A MAP ======================================

ConfigFile::ConfigFile(std::string const & configFile) {

    std::ifstream           file(configFile.c_str());
        
    std::string             line;
    std::string             value;
    
    int                     posEqual;
    int                     flag = 0;
    _nbVirtualHosts = 0;

    while (std::getline(file, line)) {

        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end()); // REMOVE WHITESPACES

        if (line.empty() || line[0] == '#' || line[0] == '}') // SKIP LINE IF IT IS EMPTY OR COMMENTED
            continue;

        // CHECK IF LINE DEFINES A SECTION AND ASSIGN PATH TO THE SECTION
        if (line[0] == '[') {

            _inSection = trim(line.substr(1, line.find(']') -1));

            if (!_inSection.compare("server")) {
                _inSection.assign("server" + to_string(++flag));
                _nbVirtualHosts++;
            }

            else if (!_inSection.compare("location/")) 
                _inSection.assign("server" + to_string(flag) + "/" + _inSection.substr(0, _inSection.find("/")));

            else if (!strncmp(_inSection.c_str(), "location/", 9))
                _inSection.assign("server" + to_string(flag) + "/" + _inSection);

            else {

                std::cout << red <<  "Config File Error: [" << _inSection << "] is not an acceptable block nomination" << def << std::endl;
                exit(0);
            }
            continue; 
        }

        // STORE THE PART BEFORE THE "=" IN _DIRECTIVE AND THE PART AFTER IN _VALUESVEC
        posEqual = line.find('=');
        _directive = trim(line.substr(0, posEqual));
        if (!checkDirective(_directive)) {

            std::cout << red << "Config File Error: [" << _directive << "] is not an acceptable directive" << def << std::endl;
            exit(0);
        }
        value = trim(line.substr(posEqual + 1, line.length() - _directive.length() - 2));
        _valuesVec = fillVector(value);

        _content[_inSection + '/' + _directive] = _valuesVec; // STORE THE PAIR OF _DIRECTIVE + _VALUESVEC IN THE MAP

    }
    checkErrorConfig(); // CHECK FOR SYNTAX ERRORS IN THE CONFIG FILE
    getPorts(); // GET THE LISTEN PORTS IN A VECTOR
}

ConfigFile::~ConfigFile() {}

ConfigFile & ConfigFile::operator=(ConfigFile const & rhs) {

	this->_content = rhs._content;
	this->_directive = rhs._directive;
	this->_valuesVec = rhs._valuesVec;
	this->_inSection = rhs._inSection;
	this->portsToOpen = rhs.portsToOpen;
	return (*this);
}

// ========================================================= GETTERS AND MEMBER FUNCTIONS TO INTERACT WITH =============================================================

// GET MAP
std::map<std::string, std::vector<std::string> > const & ConfigFile::getMap() const { 
    
    return _content; 
}

// GET BLOCK NAME
std::string ConfigFile::getSection(std::string const & host, std::string url, std::string const & directive) {

    std::string server = findServer(host);
    std::string str;

    if (url.find(".") != std::string::npos)
        url = url.substr(0, url.find_last_of("/"));
    if (!url.compare(""))
        str = server + directive; 
    else if (!url.compare("/"))
        str = server + "location" + url + directive; 
    else
        str = server + "location" + url + "/" + directive;
    return str;
}

// GET VALUE (SECOND PART OF THE PAIR IN THE MAP)
std::vector<std::string> const & ConfigFile::getValue(std::string const & port, std::string const & url, std::string const & directive) {

    std::string str = getSection(port, url, directive);
    std::map<std::string, std::vector<std::string> >::const_iterator it = _content.find(str);
    
    if (it == _content.end())
        throw ValueNotFoundException();
    return it->second;
}

// GET PORTS TO OPEN IN LISTEN MODE
void    ConfigFile::getPorts() {

    std::map<std::string, std::vector<std::string> >::iterator   it = _content.begin();

    for (; it != _content.end(); it++) {
        if (it->first.find("listen") != std::string::npos) {

            std::string stringPort = it->second[0].substr(it->second[0].find(":") + 1, it->second[0].length());
            int port = std::atoi(stringPort.c_str());

            if (std::find(portsToOpen.begin(), portsToOpen.end(), port) == portsToOpen.end())
                portsToOpen.push_back(port);
        }
    }
}

// FIND PATH TO THE FILES INSIDE THE SERVER 
std::string     ConfigFile::findPath(std::string const & port, std::string const & url) {

    if (url.find("http") != std::string::npos)
        return url;

    try {
        std::string root = getValue(port, url, "root")[0];

        if (url.find(".") == std::string::npos) { //if there is no extension, it is a directory, so go to the root directory if any to find the index.html
            std::cout << "\t\t~~~~~~~~~~~~~~~~~~~root in findPath: " << root << "\n";
            return (root + checkIndex(port, url, root));
        }
        else {
            std::string extension = url.substr(url.find_last_of("/"), url.length() - url.find_last_of("/"));
            std::cout << "\t\t~~~~~~~~~~~~~~~~~~~root + extension in findPath: " << root + extension << "\n";
            return (root + extension); //else we are looking for a file, so append the file to the root directory if any to find the file
        }
    }
    catch (ConfigFile::ValueNotFoundException &e) {

        std::string root = getValue(port, "", "root")[0];

        if (url.find(".") == std::string::npos) {
            if (url.size() == 1)
                return (root + checkIndex(port, url, root));
            if (url[0] == '/'){  // without double '//' in new url
                return (root + url + checkIndex(port, url, root));
            }
            return (root + "/" + url + checkIndex(port, url, root));
        }
        else {
            if ((url.find("/"+root)) != std::string::npos) {
                return url;
            }
            
            return (root + url); 
        }
    }
}

// DETERMINE IF THE REQUESTED METHOD IS ALLOWED 
bool            ConfigFile::isMethodAllowed(std::string const & host, std::string const & url, std::string const & method) {
// Check if methods authorizations are mentioned, else return true
    try {
         getValue(host, url, "authorized_methods");
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        
        return true;
    }

// If methods authorizations, check if the method is allowed
    std::string str = getSection(host, url, "authorized_methods");
    std::map<std::string, std::vector<std::string> >::reverse_iterator it = _content.rbegin();

    for (; it != _content.rend(); it++)
        if (it->first.find(str) != std::string::npos
            && std::find(it->second.begin(), it->second.end(), method) != it->second.end())
            return true;
    return false;
}

std::string     ConfigFile::checkRedirection(int *statusCode, std::string const & host, std::string const & url) {

    try {

        std::string redirection = getValue(host, url, "redirection")[0];
        *statusCode = 301;
        if (redirection.find("301") == std::string::npos) {

            std::cout << "ERROR: Redirection status code can only be 301" << std::endl;
            return url;
        }
        std::string updated_url = redirection.substr(redirection.find_first_of(":") + 1, redirection.size());
        return updated_url;
    }
    catch (ConfigFile::ValueNotFoundException &e) {

        return url;
    }
}

std::string     ConfigFile::getErrorPage(std::string const & host, std::string const & error) {
    std::cout << "getErrorPage error: " << error << "\n";

    try {

        std::vector<std::string> errorPage = getValue(host, "", "error_page");
        std::vector<std::string>::iterator it = errorPage.begin();
        for (;it != errorPage.end(); it++) {
            if (it->find(error) != std::string::npos){
                return ("srcs/Server/www/errorPages" + *it);
            }
        }

    }
    catch (ConfigFile::ServerNotFoundException &e) {
        std::cout << red << e.what() << def << std::endl;
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        std::cout << red << e.what() << def << std::endl;
    }
    return ("srcs/Server/www/errorPages/404notfound.html");
}

// ================================== FIND THE RIGHT SERVER TO SERVE THE REQUEST ======================================


// FIND THE SERVER THAT WILL SERVE THE REQUEST
std::string     ConfigFile::findServer(std::string const & host) {

    std::map<std::string, std::vector<std::string> >::iterator it = _content.begin();
    std::vector<std::string>    candidateServers;

    //  DETERMINE CANDIDATE SERVERS (SAME PORT, LOCAL IP)
    it  = _content.begin();
    for (; it != _content.end(); it++) {

        if (it->first.find("listen") != std::string::npos 
            && isLocalIP(it->second[0], host) == true)
                candidateServers.push_back(it->first.substr(0, it->first.find("/") + 1));
    }

    //  RETURN SERVER IF THERE IS ONLY ONE CANDIDATE 
    if (candidateServers.size() == 1)
        return candidateServers[0];

    // ELSE, COMPARE SERVER NAMES
    if (candidateServers.size() > 1) {

        it  = _content.begin();
        for (; it != _content.end(); it++) {

            if (it->first.find("server_name") != std::string::npos) {

                if (sameServerName(it->second, host) == true
                    && isCandidateServer(candidateServers, it->first) == true)
                        return it->first.substr(0, it->first.find("/") + 1);
            }
        }
        it = _content.begin();
        for (; it != _content.end(); it++) {

            if (it->first.find("server_name") != std::string::npos)
                return it->first.substr(0, it->first.find("/") + 1);
        }
    }
    throw ServerNotFoundException();
}

// DOES THE HOST IP CORRESPOND TO A LOCAL IP ?
bool    ConfigFile::isLocalIP(std::string const & listen, std::string const & host) {

    static int specificIP = 0;
    int lislen = listen.length();
    int hostlen = host.length();
    int portPos = listen.find(":") + 1;
    int portPosHost = host.find(":") + 1;
    std::string hostPort = host.substr(portPosHost, hostlen - portPosHost);
    hostPort = hostPort.substr(0, hostPort.find_first_not_of("0123456789"));
    std::string listenPort = listen.substr(portPos, lislen - portPos);

    if (!listen.compare(host)) {

        specificIP++;
        return true;
    }
    else if (!listenPort.compare(hostPort)
            && specificIP == 0)
                return true;
    return false;
}

// DOES THE HOST SPECIFY BOTH IP AND PORT ?
std::string     ConfigFile::defineHost(std::string str) {

    //  YES
    if (str.find(":") != std::string::npos) 
        return str;

    //  DOES NOT SPECIFY IP: SET THE GENERIC ADDRESS TO 0.0.0.0
    else if (str.find_first_not_of("0123456789") == std::string::npos)
        return ("0.0.0.0:" + str);

    //  DOES NOT SPECIFY THE PORT: SET THE DEFAULT PORT TO 8080
    else
        return (str + ":8080");
}

// DOES THE SERVER_NAME DIRECTIVE CORRESPOND TO THE HOST OF THE REQUEST ?
bool     ConfigFile::sameServerName(std::vector<std::string> server_name, std::string const & host) {

    std::vector<std::string>::reverse_iterator  rit = server_name.rbegin();

    for (; rit != server_name.rend(); rit++) {

        if (!host.compare(0, host.find(":"), *rit))
            return true;
    }
    return false;
}

// LIST OF CANDIDATE SERVERS THAT COULD SERVE THE REQUEST 
bool     ConfigFile::isCandidateServer(std::vector<std::string> servers, std::string const & testServ) {

    std::vector<std::string>::reverse_iterator  rit = servers.rbegin();
    std::string server = testServ.substr(0, testServ.find("/"));

    for (; rit != servers.rend(); rit++) {

        if (!server.compare(*rit))
            return true;
    }
    return false;
}

// ================================== ERROR CHECK: CONFIGURATION FILE ======================================

void     ConfigFile::checkErrorConfig(void) {

    std::map<std::string, std::vector<std::string> >::iterator  it = _content.begin();

    // CHECK POSSIBLE SYNTAX ERRORS IN THE LOCATION BLOCK
    if (_content.find("//") != _content.end()) {

        std::cout << red << "Config File Error: Wrong syntax in the location block" << def << std::endl;
        exit(0);
    }
    
    for (; it != _content.end(); it++)
    {
        // CHECK IF VIRTUAL HOST IS SPECIFIED
        if (it->first[6] == '0') {
            std::cout << red << "Config File Error: Virtual host is not specified" << def << std::endl;
            exit(0);
        }

        // CHECK NB OF VALUES IN DIRECTIVES
        if (it->second.empty() || (it->second.size() > 1 
            && it->first.substr(it->first.find_last_of("/") + 1, it->first.length()) != "authorized_methods"
            && it->first.substr(it->first.find_last_of("/") + 1, it->first.length()) != "index"
            && it->first.substr(it->first.find_last_of("/") + 1, it->first.length()) != "server_name"
            && it->first.substr(it->first.find_last_of("/") + 1, it->first.length()) != "error_page"
            && it->first.substr(it->first.find_last_of("/") + 1, it->first.length()) != "cgi")
            ) {
                std::cout << red << "Config File Error: [" << it->first << "] : Too many values for directive" << def << std::endl;
                exit(0);
            }
        
        // CHECK AUTOINDEX
        if (it->first.find("autoindex") != std::string::npos
            && it->second[0].compare("on") && it->second[0].compare("off")) {

                std::cout << red << "Config File Error: [" << it->first << "] : autoindex cannot be interpreted" << def << std::endl;
                exit(0);
            }

        // COMPLETE LISTEN DIRECTIVE IF INFORMATION IS MISSING 
        if (it->first.find("listen") != std::string::npos) 
            it->second[0] = defineHost(it->second[0]);

        // CHECK PORT CONFIGURATION
        if (it->first.find("listen") != std::string::npos) {
            if (std::count(it->second[0].begin(), it->second[0].end(), ':') != 1
                || it->second[0].find(":") == 0
                || it->second[0].find(":") == it->second[0].length() - 1) {

                std::cout << red << "Config File Error: [" << it->second[0] << "] is not an acceptable host syntax" << def << std::endl;
                exit(0);
            }
            std::string port = it->second[0].substr(it->second[0].find(":") + 1, it->second[0].length());
            std::string ip = it->second[0].substr(0, it->second[0].find(":"));
            if (port.find_first_not_of("0123456789") != std::string::npos) {
                std::cout << red << "Config File Error: [" << port << "] is not an acceptable port syntax" << def << std::endl;
                exit(0);
            }
            if (ip.compare("localhost") != 0 && ip.compare("127.0.0.1") != 0 && ip.compare("0.0.0.0") != 0) {
                std::cout << red << "Config File Error: [" << ip << "] is not an acceptable ip syntax" << def << std::endl;
                exit(0);                
            }
        }
    }
    // Check that all the mandatory directives are present in each server block
    if (checkRoot() == 0) {
        std::cout << red << "Config File Error: mandatory directives not present in all virtual hosts" << def << std::endl;
        exit(0);
    }
    // NO CONFIGURATION ISSUE
    std::cout << green << "Config File: ready to be used" << def << std::endl;
}

// CHECK IF DIRECTIVE IS ALLOWED
bool    ConfigFile::checkDirective(std::string dir) {

    if (dir.find("location") != std::string::npos)
        dir.substr(dir.find_last_of("/"), dir.length());

    std::vector<std::string>    listOfDir;
    listOfDir.reserve( 11 );

    listOfDir.push_back("listen");
    listOfDir.push_back("server_name");
    listOfDir.push_back("root");
    listOfDir.push_back("error_page");
    listOfDir.push_back("client_max_body_size");
    listOfDir.push_back("index");
    listOfDir.push_back("autoindex");
    listOfDir.push_back("cgi");
    listOfDir.push_back("authorized_methods");
    listOfDir.push_back("upload_path");
    listOfDir.push_back("redirection");

    if (std::find(listOfDir.begin(), listOfDir.end(), dir) != listOfDir.end())
        return true;
    return false;
}

// CHECK WHICH INDEX TO PICK FROM THE INDEX DIRECTIVE
std::string ConfigFile::checkIndex(std::string const & host, std::string const & url, std::string const & root) {
    
    std::vector<std::string>            indexList;
    std::vector<std::string>::iterator  it;
    std::ifstream                       data;

    try {
        indexList = getValue(host, url, "index"); // check if there is an 'index = xy.html' inside location
        for (it = indexList.begin(); it != indexList.end(); it++) {
            std::string openContent = root + "/" + *it;
            if (openContent.find("htm") != std::string::npos && openContent.find("html") == std::string::npos) {
                openContent += "l";
                *it += "l";
            }
            data.open(openContent.c_str());
            if (data) {
                data.close();
                return("/" + *it);
            }
        }
    }
    catch (ConfigFile::ServerNotFoundException &e) {
        std::cout << red << e.what() << def << std::endl;
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        std::cout << red << e.what() << def << std::endl;
    }
    try {
        std::string autoind = getValue(host, url, "autoindex")[0];
        std::cout << "autoind: " << autoind << std::endl;
        if (autoind == "on")
            return ""; // use autoindex in readContent/readDirectoryAutoindex
        return "/index.html";
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        std::ifstream                       data;

        return "/index.html";   // send index.html -> if no index-file in folder it will be handled later with 404
    }
}

// CHECK IF THERE IS A ROOT AT LEAST IN EACH SERVER BLOCK
bool    ConfigFile::checkRoot() {
    
    std::map<std::string, std::vector<std::string> >::iterator  it = _content.begin();
    int nbListen = 0;
    int nbRoot = 0;

    for (; it != _content.end(); it++) {
        
        if (it->first.find ("listen") != std::string::npos) {
            
            std::string prefix = it->first.substr(0, it->first.find("listen"));
            if (std::count(prefix.begin(), prefix.end(), '/') < 2)
                nbListen++;
        }
    }
    for (it = _content.begin(); it != _content.end(); it++) {
            
        if (it->first.find ("root") != std::string::npos) {
            
            std::string prefix = it->first.substr(0, it->first.find("root"));
            if (std::count(prefix.begin(), prefix.end(), '/') < 2)
                nbRoot++;
        }
    }
    if (nbListen >= _nbVirtualHosts && nbRoot >= _nbVirtualHosts)
        return 1;
    return 0;
}

// ===================================================== UTILS ===========================================================

//  TRIM 
std::string                 ConfigFile::trim(std::string const & source, char const *delims) {

    std::string result(source);
    std::string::size_type  index = result.find_last_of(delims);

    if (index != std::string::npos)
        result.erase(++index);

    index = result.find_first_not_of(delims);

    if (index != std::string::npos)
        result.erase(0, index);
    else
        result.erase();

    return result;
}

// TOKENIZE THE STRING BY | AND ;
std::vector<std::string>    ConfigFile::fillVector(std::string const & value) {

    std::vector<std::string> vector;
    char                     val[value.length() + 1];
    char *                   token;

    strcpy(val, value.c_str());
    token = std::strtok(val, "|;");

    while (token) {

            std::string str = token;
            vector.push_back(str);
            token = std::strtok(NULL, "|;");
    }

    return vector;
}