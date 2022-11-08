#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <iostream>
# include <map>
# include <vector>
#include "../Config/parseConfig.hpp"

class httpRequest
{
    private:
        std::string     _method;
        std::string     _url;
        std::string     _version;

	    std::vector<std::pair<std::string, std::string > >  _header;
        std::string     _host;      // -> host from body request
        std::string     _query;
        std::string     _body;
        
        ConfigFile*     _ConfigFile;

        int             _statusCode;

        std::string     _extension;
        std::string     _execArgExtension;

        std::vector<std::string>     chunksSplit( const std::string &str, const std::string &delimiter );
        unsigned int                 hextodec( const std::string &hex );
        unsigned int                 read_chunk_size( long socket );
        std::string                  read_line(long socket, bool incl_endl);
        std::string                  read_data(long socket, int size);
        std::string                  decodeChunks(long socket);

    public:
        httpRequest(std::string buffer, long socket);
        httpRequest();
        ~httpRequest();

        std::string     readContent();
        std::string     readFileContent();
        std::string     readDirectoryAutoindex();
        void            findContentType();
        void            parseRequest(std::string buffer, long socket);
        void            getFirstLine(std::string str, std::string deli);
        int             checkFirstLine();
        void            setQuery();
        void            uploadFile(std::string buffer);
        void            parseHeader(std::string buffer);
        void            parseBody(std::string *contentLength);
        int             isValid(ConfigFile & cf);
        void            handleURL(ConfigFile & cf);
        bool            autoI;
        bool            isCgi;

/* SETTERS - GETTERS */

        void        setUrl(std::string url);
        std::string getUrl();
        
        std::string getVersion();

        void        setHost(std::string buffer);
        std::string getHost();

        std::string getBody();
        std::string getExtension();
        std::string getExecArg();

        void        setMethod(std::string method);
        std::string getMethod();

        void        setStatusCode(int nbr);
        int         getStatusCode();

        std::string *getHeaderValue(std::string const &key);
        void        setHeaderValue(std::string key, std::string value);
        std::vector<std::pair<std::string, std::string > >  getHeader();

        ConfigFile* getConfigFile();
        
};

#endif