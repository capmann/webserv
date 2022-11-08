#include "parseConfig.hpp"

int main(int ac, char **av) {

    if (ac != 2) {
        std::cout << "ERROR: Wrong number of arguments\n" << std::endl; 
        return (1); 
    }

    std::cout << std::endl;

    std::string path = av[1]; 
    ConfigFile  cf(path);

    std::cout << std::endl;

    try {

        std::cout << "Path to files: " << cf.findPath("example.org:80", "/app") << std::endl;
        std::cout << "Path to files: " << cf.findPath("example.org:80", "/uploads") << std::endl;

        std::cout << "Is GET allowed for localhost:80/uploads ? " << cf.isMethodAllowed("localhost:80", "/uploads", "GET") << std::endl;

        std::vector<std::string> host = cf.getValue("localhost:80", "", "listen");
        std::cout << "host = " << host[0] << std::endl;
    
        std::vector<std::string> server_name = cf.getValue("localhost:80", "", "server_name");
        std:: cout << "server_name = " << server_name[1] << std::endl;

        std::vector<std::string> root = cf.getValue("localhost:80", "/uploads", "root");
        std:: cout << "root = " << root[0] << std::endl;

        std::vector<std::string> error_page = cf.getValue("localhost:80", "" , "error_page");
        std::cout << "error_page = " << error_page[0] << std::endl;

        std::vector<std::string> methods = cf.getValue("localhost:80", "/app", "authorized_methods");
        std::cout << "authorized_methods = " << methods[0] << std::endl;

        std::vector<std::string> methods1 = cf.getValue("localhost:80", "/", "authorized_methods");
        std::cout << "authorized_methods = " << methods1[0] << std::endl;

        std::cout << "Path to files: " << cf.findPath("localhost:443", "/uploads") << std::endl;

        std::cout << "Is DELETE allowed for localhost:443/uploads ? " << cf.isMethodAllowed("localhost:443", "/uploads", "DELETE") << std::endl;
        std::cout << "Is POST allowed for localhost:443/uploads ? " << cf.isMethodAllowed("localhost:443", "/uploads", "POST") << std::endl;

        std::vector<std::string> root443 = cf.getValue("localhost:443", "/uploads", "root");
        std:: cout << "root = " << root443[0] << std::endl;

        std::vector<std::string> methods443 = cf.getValue("localhost:80", "/app", "authorized_methods");
        std::cout << "authorized_methods = " << methods443[1] << std::endl;

        std::vector<std::string> upload = cf.getValue("localhost:443", "", "upload_path");
        std::cout << "upload path = " << upload[0] << std::endl;

        std::vector<std::string> redir = cf.getValue("localhost:443", "/uploads", "redirection");
        std::cout << "redir = " << redir[0] << std::endl;

        std::vector<std::string> ghost = cf.getValue("localhost:80", "/appa", "authorized_methods");
        std::cout << "authorized_methods = " << ghost[0] << std::endl;
    }
    catch (ConfigFile::ServerNotFoundException &e) {
        std::cout << e.what() << std::endl;
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << std::endl;
    return (0);
}