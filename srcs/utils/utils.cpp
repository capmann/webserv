#include <map>
#include <string>

std::string     StatusCodeInit(int code) {
    std::map<int, std::string>  StatusCodes;
    
    StatusCodes[100] = "Continue";
    StatusCodes[101] = "Switching Protocols";
    StatusCodes[200] = "OK";
    StatusCodes[201] = "Created";
    StatusCodes[202] = "Accepted";
    StatusCodes[203] = "Non-Authoritative Information";
    StatusCodes[204] = "No Content";
    StatusCodes[205] = "Reset Content";
    StatusCodes[206] = "Partial Content";
    StatusCodes[300] = "Multiple Choices";
    StatusCodes[301] = "Moved Permanently";
    StatusCodes[302] = "Found";
    StatusCodes[303] = "See Other";
    StatusCodes[304] = "Not Modified";
    StatusCodes[305] = "Use Proxy";
    StatusCodes[307] = "Temporary Redirect";
    StatusCodes[400] = "Bad Request";
    StatusCodes[401] = "Unauthorized";
    StatusCodes[402] = "Payment Required";
    StatusCodes[403] = "Forbidden";
    StatusCodes[404] = "Not Found";
    StatusCodes[405] = "Method Not Allowed";
    StatusCodes[406] = "Not Acceptable";
    StatusCodes[407] = "Proxy Authentication Required";
    StatusCodes[408] = "Request Time-out";
    StatusCodes[409] = "Conflict";
    StatusCodes[410] = "Gone";
    StatusCodes[411] = "Length Required";
    StatusCodes[412] = "Precondition Failed";
    StatusCodes[413] = "Request Entity Too Large";
    StatusCodes[414] = "Request-URI Too Large";
    StatusCodes[415] = "Unsupported Media Type";
    StatusCodes[416] = "Requested range not satisfiable";
    StatusCodes[417] = "Expectation Failed";
    StatusCodes[500] = "Internal Server Error";
    StatusCodes[501] = "Not Implemented";
    StatusCodes[502] = "Bad Gateway";
    StatusCodes[503] = "Service Unavailable";
    StatusCodes[504] = "Gateway Time-out";
    StatusCodes[505] = "HTTP Version not supported";
    return StatusCodes[code];
}

std::string     getDate() {
    char    date[33];
	size_t  size;
    time_t  now = time(0);

	tm *gmtm = gmtime(&now);

	size = strftime(date, 32, "%a, %d %b %Y %T GMT", gmtm);
	date[size] = '\0';
	return (std::string(date));
}
