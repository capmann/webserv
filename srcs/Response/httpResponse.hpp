#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP
# include "../Request/httpRequest.hpp"
# include <iostream>
# include <sstream>
# include <map>
# include <vector>

class   httpResponse
{
    private:
        std::string pageContent;
        std::string _contentType;

        std::string _filename;
		char        **_envVar;      // env var for cgi execve

    
    public:
        char        **execArgv;     // argument for execve(execArgv[0], execArgv, environ)  tmp public
        httpRequest request;
        httpResponse();
        ~httpResponse();
        void        setPageContent(std::string content);
        std::string getPageContent();
        void        findContentType(std::string url);


        /* handle methods */
        void        GETMethod();
        void        POSTMethod(ConfigFile * cf);
        void        DELETEMethod();
        void        methodHandler(ConfigFile * cf, std::string method);
        void        POSTcleanUpUploadFile(std::string *fileContent);
        void        POSTUploads(ConfigFile * cf);

        /* handle CGI */
        int         checkCgi();
        void        handleCgi();
        int         executeCgi();
        void        handleCgiFile();
        void        createEnvVar();

        void        setContentType(std::string type);
        std::string getContentType();

};

template <typename T>
std::string to_string(T value)
{
    //create an output string stream
    std::ostringstream os ;

    //throw the value into the string stream
    os << value ;

    //convert the string stream into a string and return
    return os.str() ;
}

#endif