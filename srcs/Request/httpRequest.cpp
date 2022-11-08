#include "httpRequest.hpp"
#include <fstream>
#include <sstream>      // ostringstream
#include <dirent.h>     // DIR
#include <sys/stat.h>   // stat
#include "../Config/colormod.hpp"
#include <sys/socket.h>
#include <istream>

Color::Modifier		rouge(Color::FG_RED);
Color::Modifier		defi(Color::FG_DEFAULT);

httpRequest::httpRequest(std::string buffer, long socket): _method(""), _url(""), _version(""), _body(""), _statusCode(200), autoI(false), isCgi(false) {
    std::cout << "REQUEST [\n\n" << buffer << "]\n";
        isCgi = false;
        parseRequest(buffer, socket);
}

httpRequest::httpRequest(void) {}

httpRequest::~httpRequest() {}

/* ===================================== PARSE REQUEST ==========================================================*/

void    httpRequest::parseRequest(std::string buffer, long socket) {

    size_t start = buffer.find("\r\n\r\n");
    int end = buffer.size();

    if (start != std::string::npos) {    // check if "/r/n/r/n" present

        _body = buffer.substr(start + 4, end -1);

        getFirstLine(buffer, " ");
        
        setQuery();
        
        start = buffer.find("\r\n");
        
        if (start != std::string::npos)
            buffer = buffer.substr(start, end);

        parseHeader(buffer);        
        
        std::string *val = getHeaderValue("Transfer-Encoding:");
        std::string *contentLength = getHeaderValue("Content-Length:");

        if (val) {
            if (val->find("chunked") != std::string::npos)
                buffer = decodeChunks(socket); // -> first line is hexadecimal value that tells the extraction length of the second line
        }

        else if (contentLength) {     
            if ((_method == "GET" || _method == "DELETE") && contentLength->compare("0") != 0)
                std::cout << rouge << "ERROR: CONTENT LENGTH SHOULD BE 0" << defi << std::endl;

            else if (_method == "POST")
                parseBody(contentLength);
        }

        /*
        The Content-Length is optional in an HTTP request. For a GET or DELETE the length must be zero.
        For POST, if Content-Length is specified and it does not match the length of the message-line,
        the message is either truncated, or padded with nulls to the specified length.
        The Content-Length is always returned in the HTTP response even when there is no content, in which case the value is zero.
        */

        setHost(buffer);
    }
    else
        std::cout << "\t\telse: no rnrn in parseRequest!\n\n";
}

void    httpRequest::parseHeader(std::string buffer) {

    size_t      points;
    std::string key;
    std::string value;
    
    if (buffer.find("\r\n") == 0)       // erase first empty line
        buffer.erase(0, 1);
    
    std::string    line = buffer.substr(0, buffer.find("\r\n"));

    while (line != "")
    {
        points = line.find_first_of(":");
        key = line.substr(1, points);   // without \n
        value = line.substr(points + 1, line.length());
        if (key != "" && value != "")
            _header.push_back(std::make_pair(key, value));
        buffer.erase(0, line.length() + 1);     // delete first line from buffer
        line = buffer.substr(0, buffer.find("\r\n"));
    }
}

void    httpRequest::parseBody(std::string *contentLength) {  // already set in parseRequest
    
    size_t len = atoi((*contentLength).c_str());

    if (_body.length() != len) // if 'Content-Length' != body length, fill with NULLS or truncate to len
        _body.resize(len, '\0'); 
}

void    httpRequest::getFirstLine(std::string str, std::string deli = " ") {

    int start = 0;
    int end = str.find(deli);
    if (!str[0])    // -> check added for testing with telnet - otherwise _method[0] == '' and _method.compare("GET") not OK
        start++;

    if (end != -1)
        _method = str.substr(start, end - start);
    start = end + deli.size();
    end = str.find(deli, start);
    if (end != -1)
        _url = str.substr(start, end - start);
    start = end + deli.size();
    end = str.find("\r\n", start);
    if (end != -1)
        _version = str.substr(start, end - start);
}

int     httpRequest::checkFirstLine() {

    // check method:
    if (_method.compare("GET") != 0 && _method.compare("POST") != 0 && _method.compare("DELETE") != 0)
    {
        std::cerr << "Error: method not valid." << std::endl;
        _statusCode = 400; // bad request
        return -1;
    }
    
    // check url:
    if (_url[0] != '/' || _url.length() == 0) {
        _statusCode = 400; // bad request
        return -1;
    }
    
    if (_url.length() >= 256) {
        _statusCode = 414; // "Request-URI Too Large"
        return -1;
    }

    // check http version:
    if (_version.compare(0, 6, "HTTP/1") != 0) {
        std::cerr << "Error: HTTP Version not valid." << std::endl;
        _statusCode = 505;  // "HTTP Version not supported"
        return -1;
    }
    
    // check if numbers after 'HTTP/1':
    if (_version.substr(7, _version.length()).find_first_not_of("0123456789") != std::string::npos) {
        std::cerr << "Error: HTTP Version not valid." << std::endl;
        _statusCode = 505;  // "HTTP Version not supported"
        return -1;
    }
    return 1;
}

void    httpRequest::setQuery() {

    size_t  pos;

    if ((pos = _url.find_first_of('?')) != std::string::npos){
        _query = _url.substr(pos + 1, _url.size());
        _url = _url.substr(0, pos);
    }
}

/* ================================================== READ CONTENT ============================================================== */ 

std::string         httpRequest::readDirectoryAutoindex() {

    const char  *path = _url.c_str();
    std::string dirName(path);
    DIR         *dir = opendir(path);

    if (dir == NULL) {
        std::cerr << "Error: could not open [" << path << "]" << std::endl;
        return "";
    }
    
    std::string root;
    
    try {
        root = _ConfigFile->getValue(_host, "/", "root")[0];
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        root = "";
    }
    
    if (root == "") {
        try {
            root = _ConfigFile->getValue(_host, "", "root")[0];
        }
        catch (ConfigFile::ValueNotFoundException &e) {
           root = "";
        }
    }
    
    if (dirName[0] != '/')
        dirName = "/" + dirName;
    
    std::cout << "\tdirName: " << dirName << "\n\troot: " << root << "\n";
    
    if (dirName == ("/"+root))
        dirName = "";
    
    size_t  pos;
    
    if ((pos = dirName.find("/"+root)) != std::string::npos) {
        dirName = dirName.substr(root.size() + 1, dirName.size());
    }
    
    if (dirName[0] == '/' && dirName[1] == '/')
        dirName = dirName.substr(1, dirName.size());    // delete seccond '/' in dirName: //uploads
    
    std::cout << "\tdirName: " << dirName << "\n";

    std::string page ="<!DOCTYPE html>\n<html>\n<head>\n<title>" + dirName + "</title>\n</head>\n<body>\n<ul><h1>" + dirName + "</h1";
    
    for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir))
        page += "\t\t<li><a style=\"text-decoration: none;color:black;\" href=" + dirName + "/" + std::string(dirEntry->d_name)\
                + "><strong>" + std::string(dirEntry->d_name) + "</strong></a></li>\n";
    
    page +="</ul>\n</body>\n</html>\n";
    
    closedir(dir);
    
    autoI = true;
    
    return page;
}

std::string     httpRequest::readFileContent() {

    std::ifstream       data;
    std::ostringstream  buffer;

    data.open(_url.c_str());
    
    if (!data) {
        std::cout << "Error: " << _url << " could not be opened. Send tmp error page." << std::endl;
        _url = "srcs/Server/www/errorPages/404notfound.html";
        data.close();
        data.open(_url.c_str());
    }
    
    buffer << data.rdbuf();  // reading data
    
    data.close();
    
    return buffer.str();
}

//https://www.tutorialspoint.com/Read-whole-ASCII-file-into-Cplusplus-std-string
std::string     httpRequest::readContent() {

    struct stat         s;

    std::cout << "\t\t\t_url beginn of readContent(): " << _url << std::endl;

    if (_url[0] == '/' && _url.length() > 1)
        _url = _url.substr(1, _url.length());
    
    if (_url[0] == '/' && _url.length() == 1)
        _url = "index.html";

    if (stat(_url.c_str(), &s) == 0) //Get file attributes for FILE and put them in 's'.
    {
        if (s.st_mode & S_IFDIR) {
            std::cerr << "it's a directory\n";
            return readDirectoryAutoindex();
        }
    
        else if (s.st_mode & S_IFREG) {
            std::cerr << "it's a file\n";
            return readFileContent();
        }
    
        else
            std::cerr << "something else\n";
    }
    
    else
            std::cerr << "\t|favicon|error: else of if(stat(_url.c_str(),&s) == 0 )\n";
    
    return "";
}

/* ===================================== CHUNKS MANAGEMENT ===============================================================

Data is sent in a series of chunks. The Content-Length header is omitted in this case and at the beginning of each chunk you
need to add the length of the current chunk in hexadecimal format, followed by '\r\n' and then the chunk itself, followed by
another '\r\n'. The terminating chunk is a regular chunk, with the exception that its length is zero. It is followed by the
trailer, which consists of a (possibly empty) sequence of header fields.
https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding#chunked_encoding
https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
https://stackoverflow.com/questions/24625620/how-should-http-server-respond-to-head-request-for-chunked-encoding
*/

std::string httpRequest::read_data( long socket, int size ) {
 
    char buffer[size];
    std::string data = "";

    if ( recv( socket, buffer, size, 0 ) > 0 ) {
            data.append( buffer );
    }

    return data;
}

std::string httpRequest::read_line( long socket, bool incl_endl = true ) {
 
    int n;
    std::string line;
    char c = '\0';
    
    while ( ( n = recv( socket, &c, 1, 0 ) ) > 0 ) {
 
        if ( c == '\r' ) {
 
            if ( incl_endl ) {
                line += c;
            }
 
            // peak for \n
            n = recv( socket, &c, 1, MSG_PEEK );
 
            if ( ( n > 0 ) && ( c == '\n' ) ) {
 
                n = recv( socket, &c, 1, 0 );
 
                if ( incl_endl ) {
                    line += c;
                }
 
                break; // end of line
            }
            
        }
 
        line += c;
    }
    return line;
}

std::vector<std::string> httpRequest::chunksSplit( const std::string &str, const std::string &delimiter ) {
 
    std::vector<std::string>    result;
    size_t                      current;
    size_t                      next = -1;
 
    while ( next != std::string::npos ) {

        current = next + 1;
        next = str.find_first_of( delimiter, current );
        result.push_back( str.substr( current, ( next - current ) ) );

    };
 
    return result;
}

unsigned int httpRequest::hextodec( const std::string &hex ) {
 
    unsigned int dec;
    std::stringstream ss;

    ss << std::hex << hex;
    ss >> dec;

    return dec;
}

unsigned int httpRequest::read_chunk_size( long socket ) {
 
        std::string              line;
        std::vector<std::string> chunk_header;
 
        line = read_line( socket );
        chunk_header = chunksSplit( line, ";" );

        return hextodec( chunk_header.at( 0 ) ); 
}

std::string httpRequest::decodeChunks( long socket ) {
 
        unsigned int chunk_size = 0;
        unsigned int offset = 0;
        std::string chunked_data = "";
 
        while ( ( chunk_size = read_chunk_size( socket ) ) > 0 ) {
 
                offset = chunked_data.size();
 
                // read chunk-data
                chunked_data.append( read_data( socket, chunk_size ) );
 
                // sanity check
                if ( ( chunked_data.size() - offset ) != chunk_size ) {
                        // something went wrong
                        break;
                }
 
                // extra \r\n
                read_data( socket, 2 );
 
        }
 
        // read until the end of chunked data
        while ( read_line( socket, true ).size() > 2 ) ;

        return chunked_data;
}

/* ======================================== URL MANAGEMENT =============================================== */

int     httpRequest::isValid(ConfigFile & cf) {

    _ConfigFile = &cf;

    try {   

        if (checkFirstLine() == -1) {   // _statusCode changed inside functione
         
            std::cout << "ERROR: Invalid request" << std::endl;
         
            _url = cf.getErrorPage(_host, "400");
        }

        if (cf.isMethodAllowed(_host, _url, _method) == false) {
        
            std::cout << rouge << "ERROR: Method is not allowed for this server" << defi << std::endl;
            _statusCode = 405; // "Method Not Allowed"
            _url = cf.getErrorPage(_host, "400");
        }

        // Check if there is a cgi, and if yes, if the config file is set up for it
        size_t ext = _url.find_last_of(".");
        
        if (ext == std::string::npos)
            return 1;
        
        _extension = _url.substr(ext + 1, _url.size());

        if (_extension == "py" || _extension == "pl" || _extension == "php") {
            
            std::vector<std::string> cgi = cf.getValue(_host, "", "cgi");
            std::vector<std::string>::iterator itbeg = cgi.begin();
            
            size_t  pos;
            for (; itbeg != cgi.end(); itbeg++) {
                if (itbeg->find(_extension) != std::string::npos) {
                    pos = itbeg->find_first_of(":");
                    _execArgExtension = itbeg->substr(pos + 1, itbeg->size());
                }
            }
            isCgi = true;
            if (_execArgExtension == "") {
                _statusCode = 400; // bad request
                _url = cf.getErrorPage(_host, "404");
            }
        }
    }
    catch (ConfigFile::ServerNotFoundException &e) {
        
        std::cout << rouge << e.what() << defi << std::endl;
        _statusCode = 400; // bad request
        _url = cf.getErrorPage(_host, "404");
    }
    catch (ConfigFile::ValueNotFoundException &e) {
       
        std::cout << rouge << e.what() << ". Check the URI of your request." << defi << std::endl;
        _statusCode = 400; // bad request
        _url = cf.getErrorPage(_host, "404");           
        isCgi = false;                                    
    }
    return 1;
}


void    httpRequest::handleURL(ConfigFile & cf) {   // find url-corresponding route

    try {

        if (_url.find("error") == std::string::npos || _url.find("errorPages") != std::string::npos) {
            _url = cf.checkRedirection(&_statusCode, _host, _url); // check if there is a redirection
            _url = cf.findPath(_host, _url); // find the path to the right file inside the server

            if (_url.find("http") == std::string::npos) {   // == no redirection to ex: https://42.fr
                std::string path = getenv("PWD");
                struct stat         s;

                if (stat((path + "/" + _url).c_str(), &s) == 0) //Get file attributes for FILE and put them in 's'.
                {
                    if (s.st_mode & S_IFDIR) {
                        // std::cerr << "~it's a directory\n";
                    }
                    else if (s.st_mode & S_IFREG) {
                        // std::cerr << "~it's a file\n";
                    }
                    else {
                        _statusCode = 404;
                        _url = cf.getErrorPage(_host, "404");
                    }
                }
                else {
                    _statusCode = 404;
                    _url = cf.getErrorPage(_host, "404");
                }
            }
        }
    }
    catch (ConfigFile::ServerNotFoundException &e) {
        
        std::cout << rouge << e.what() << defi << std::endl;
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        
        std::cout << rouge << e.what() << defi << std::endl;
    }
}

/* ======================================= SETTERS - GETTERS ===========================================================*/

 void       httpRequest::setUrl(std::string url) {
   
    _url = url;
 }

std::string httpRequest::getUrl() {
   
    return _url;
}

std::string httpRequest::getVersion() {
    return _version;
}

void    httpRequest::setHost(std::string buffer) {
    // get the Host of the request
    size_t start = buffer.find_first_of(":") + 2;
    
    if ((start != std::string::npos)) {
        
        std::string tmp = buffer.substr(start, buffer.length());
        
        _host = buffer.substr(start, tmp.find('\n'));
    }
}

std::string    httpRequest::getHost() {
    
    return _host;
}


std::string httpRequest::getBody() {
    
    return _body;
}

std::string httpRequest::getExtension() {
    
    return _extension;
}

std::string httpRequest::getExecArg() {
    
    return _execArgExtension;
}

void    httpRequest::setMethod(std::string method) {
    
    _method = method;
}

std::string httpRequest::getMethod() {
    
    return _method;
}

void    httpRequest::setStatusCode(int StatusCode) {
  
   _statusCode = StatusCode;
}

int     httpRequest::getStatusCode() {
  
    return _statusCode;
}

std::string *httpRequest::getHeaderValue(std::string const &key) {
	
    for (std::vector<std::pair<std::string, std::string> >::iterator
			 it = _header.begin();
		 it != _header.end(); ++it)
	{
		if (it->first == key)
			return (&it->second);
	}

	return (NULL);
}

void        httpRequest::setHeaderValue(std::string key, std::string value) {
    
    _header.push_back(std::make_pair(key, value));
}

std::vector<std::pair<std::string, std::string > >  httpRequest::getHeader() {
    return _header;
}

ConfigFile* httpRequest::getConfigFile() {
    return _ConfigFile;
}
