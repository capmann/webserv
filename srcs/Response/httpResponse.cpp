#include "httpResponse.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream


httpResponse::httpResponse(): execArgv(NULL) {}

httpResponse::~httpResponse() {
    if (execArgv) {
        for(int i = 0;execArgv[i];i++)
            free(execArgv[i]);
        // free(execArgv+2);
        free(execArgv);
    }
}

void    httpResponse::setPageContent(std::string content) {
    pageContent = content;
}

std::string     httpResponse::getPageContent() {
    return pageContent;
}

void    httpResponse::findContentType(std::string url) {
	_contentType = url.substr(url.rfind(".") + 1, url.size() - url.rfind("."));
    if (request.autoI == true || _contentType == "html")  // autoindex
		_contentType = "text/html";
	else if (_contentType == "png")
		_contentType = "image/png";
	else if (_contentType == "css")
		_contentType = "text/css";
	else if (_contentType == "jpeg" || _contentType == "jpg")
		_contentType = "image/jpeg";
	else if (_contentType == "js")
		_contentType = "text/javascript";
	else if (_contentType == "bmp")
		_contentType = "image/bmp";
	else
		_contentType = "text/plain";
}

            /* handle methods */
void    httpResponse::GETMethod() {
    setPageContent(request.readContent());
    findContentType(request.getUrl());
}

//https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST
void    httpResponse::POSTMethod(ConfigFile * cf) {
    try
    {
        std::string max_size = cf->getValue(request.getHost(), "", "client_max_body_size")[0];
        if (max_size.size() < (*request.getHeaderValue("Content-Length:")).size() && !request.getHeaderValue("Content-Disposition:")) {
            request.setStatusCode(413);
            request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "400"));
        }
        else if (max_size.size() == (*request.getHeaderValue("Content-Length:")).size() && !request.getHeaderValue("Content-Disposition:"))
        {
            for (int i = 0; i < (int)max_size.size(); i++)
            {
                if (max_size.at(i) > (*request.getHeaderValue("Content-Length:")).at(i))
                    break ;
                else if (max_size.at(i) < (*request.getHeaderValue("Content-Length:")).at(i))
                {
                    request.setStatusCode(413);
                    request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "400"));
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        // std::cout << "no max_size given - all ok\n";
    }
    
    // check if upload:
    if (request.getStatusCode() != 413 && request.getHeaderValue("Content-Disposition:"))
        POSTUploads(cf);
    setPageContent(request.readContent());
    findContentType(request.getUrl());
}

void    httpResponse::POSTUploads(ConfigFile * cf) {
    std::string path = getenv("PWD");

    try {
        std::vector<std::string> upload = cf->getValue(request.getHost(), "", "upload_path");
        path += "/srcs/Server/www" + upload[0];
    }
    catch (ConfigFile::ValueNotFoundException &e) {
        path += "/srcs/Server/www/uploads";
    }
            
    std::string fileContent = request.getBody();
    size_t      pos;

    POSTcleanUpUploadFile(&fileContent);
    if ((pos = request.getHeaderValue("Content-Disposition:")->find("filename=")) != std::string::npos) {
        _filename = request.getHeaderValue("Content-Disposition:")->substr(pos + 10, request.getHeaderValue("Content-Disposition:")->size());
    } else
        _filename = "newfile";
    if (_filename[_filename.size() - 1] == '"')
        _filename = _filename.substr(0, _filename.size() - 1);      // delete last "
            
    // create a file with raw data from body
    std::string pathwithnewfile(path + "/" +  _filename);
    std::ofstream newFile(pathwithnewfile.c_str());
    if(newFile)
        newFile.write(fileContent.c_str(), fileContent.size());
}

// delete first + last line from body ------:
void    httpResponse::POSTcleanUpUploadFile(std::string *fileContent) {
        size_t      pos;

        if ((pos = fileContent->find_first_of("WebKitFormBoundary")) != std::string::npos && pos < 100) { // pos < 100 to be sure it is the header --- and not the one at the end
            pos = fileContent->find_first_of("\n");
            *fileContent = fileContent->substr(pos + 1, fileContent->size());
        }
        if ((pos = fileContent->find_first_of("Content-Disposition")) != std::string::npos) {
            pos = fileContent->find_first_of("\n");
            *fileContent = fileContent->substr(pos + 1, fileContent->size());
        }
        if ((pos = fileContent->find_first_of("Content-Type")) != std::string::npos) {
            pos = fileContent->find_first_of("\n");
            *fileContent = fileContent->substr(pos + 1, fileContent->size());
        }
        if ((pos = fileContent->find_last_of("------WebKitFormBoundary")) != std::string::npos) {
            *fileContent = fileContent->substr(0, pos);
            *fileContent = fileContent->substr(0, fileContent->find_last_of("\n"));
            *fileContent = fileContent->substr(0, fileContent->find_last_of("\n"));
        }
}

/* The DELETE method deletes the specified resource. */
void    httpResponse::DELETEMethod() {

    struct stat s;
	if (stat(request.getUrl().c_str(), &s) == 0)    // check if file:
	{
		if (s.st_mode & S_IFREG && request.getUrl().find("error") == std::string::npos) {   // file
            if (remove(request.getUrl().c_str()) != 0)
                request.setStatusCode(403);
        }
		else if (s.st_mode & S_IFDIR)    // directory
            request.setStatusCode(404);
		else
            request.setStatusCode(404);
	}
	else
            request.setStatusCode(403);
    
    if (request.getStatusCode() < 400)  // create response or error handling if remove didn't work
        request.setUrl("/srcs/Server/www/DeletOK.html");
    else
        request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "404"));
    setPageContent(request.readContent());
    findContentType(request.getUrl());
}

void        httpResponse::methodHandler(ConfigFile * cf, std::string method) {
    if (request.getStatusCode() < 400 && checkCgi() == 1)
        handleCgi();
    else if (method[0] == 'G')
        GETMethod();
    else if (method[0] == 'P')
        POSTMethod(cf);
    else if (method[0] == 'D')
        DELETEMethod();
    else {
        request.setStatusCode(400);
        request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "400"));
    }
}


    /* CGI */
int     httpResponse::checkCgi() {
    struct stat sb;

    if (request.isCgi == true && request.getMethod() != "DELETE") {
        if (stat(request.getUrl().c_str(), &sb) != 0) {  // path exists
            // std::cout << "path does not exist in checkCgi\n";
            request.setStatusCode(400);
            request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "400"));
            return 0;
        }
        execArgv = (char **)malloc(sizeof(char *) * 3);
        *(execArgv + 2) = NULL;
        *(execArgv + 0) = (char *)strdup(request.getExecArg().c_str());
        return 1;
    }
    return 0;
}

void    httpResponse::handleCgi() {
    executeCgi();
    handleCgiFile();
}

/* unistd.h
    Standard file descriptors.
#define	STDIN_FILENO	0	Standard input.
#define	STDOUT_FILENO	1	Standard output.
#define	STDERR_FILENO	2	Standard error output.
*/
// execve 3 arguments: the path to the program, a pointer to a null-terminated array of argument strings, and a pointer to a null-terminated array of environment variable strings
// execve arg[0] = cgi-bin binary arg[1] = cgi-bin script executable arg[2] = NULL
int httpResponse::executeCgi() {

    pid_t   pid;
    int     status;
    int     newFdOut;
    
    if (request.getUrl()[0] == '/')
        *(execArgv + 1) = (char *)strdup(request.getUrl().substr(1, request.getUrl().size()).c_str());
    else
        *(execArgv + 1) = (char *)strdup(request.getUrl().substr(0, request.getUrl().size()).c_str());

    pid = fork();
    if (!pid) {
        createEnvVar();
        close(STDIN_FILENO);        // close stdin -> new stdin == response from execve
        newFdOut = open("./cgiFile", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// == new stdout
        if (newFdOut == -1)
            exit(1);
        dup2(newFdOut, STDOUT_FILENO);  // change stdin/stdout with dup2
        if ((execve(execArgv[0], execArgv, _envVar)) < 0) {
            std::cout << "execve failed.\n";
            if (execArgv) {
                for(int i = 0;execArgv[i];i++)
                    free(execArgv[i]);
                free(execArgv);
            }
            exit(1);
        }
    }
    else if (pid < 0) {
        request.setStatusCode(500);
        request.setUrl(request.getConfigFile()->getErrorPage(request.getHost(), "400"));
        return -1;
    }
    waitpid(pid, &status, 0);
    return 1;
}

// read 'tmp file' + set Page content
void    httpResponse::handleCgiFile() {
    std::fstream    file;
    std::string     line;
    std::stringstream   sStream;
    bool            failed = false;

    file.open("cgiFile");
    if (!file)
        std::cerr << "open cgiFile error.\n";
    else {
        if (getline(file, line)) {
            if (line.find("execve failed.") != std::string::npos) {
                file.close();
                setPageContent("execve failed. Please check your cgi parameters in the config-file.");
                request.setStatusCode(400);
                failed = true;
            }
            if (line.find("Content-type") && failed == false) { // check if Content-type in first line for header
                size_t points = line.find_first_of(":");
                std::string key = line.substr(0, points);   // without \n
                std::string value = line.substr(points + 1, line.length());
                if (key != "" && value != "")
                    request.setHeaderValue(key, value);
            }
        }
        while (failed == false && getline(file, line)) // skip first line if content-type
            sStream << line + "\n";
    }
    file.close();
    if (failed == false)
        setPageContent(sStream.str());
}
// https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script
// https://darrencgi.tripod.com/env_var.html
void    httpResponse::createEnvVar() {
    std::map<std::string, std::string>  _env;
  
    if (request.getHeaderValue("content-length"))
        _env["CONTENT_LENGTH"] = request.getHeaderValue("content-length")->c_str();
    else
        _env["CONTENT_LENGTH"] = "0";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["PATH_INFO"] = request.getUrl().c_str();
    _env["REDIRECT_STATUS"] = request.getStatusCode();
    _env["REQUEST_METHOD"] = request.getMethod().c_str();
    _env["QUERY_STRING"] = request.getBody().c_str();   // ex.: "first=Anna&last=REISS"
    _env["SCRIPT_NAME"] = "srcs/Server/www/cgi-bin/";
    _env["SERVER_NAME"] = "0";
    _env["SERVER_PORT"] = request.getHost().c_str();
    _env["SERVER_PROTOCOL"] = request.getVersion().c_str();
    _env["SERVER_SOFTWARE"] = "xy/1.0";

	_envVar = new char*[_env.size() + 1];
    int	i = 0;
	for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); it++) {
		std::string	element = it->first + "=" + it->second;
		_envVar[i] = new char[element.size() + 1];
		_envVar[i] = strcpy(_envVar[i], (const char*)element.c_str());
		i++;
	}
	_envVar[i] = NULL;
}

    /* GETTER SETTER */

void   httpResponse::setContentType(std::string type) {
    _contentType = type;
}

std::string httpResponse::getContentType() {
    return _contentType;
}
