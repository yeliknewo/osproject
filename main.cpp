#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <fstream>
#include <iterator>
#include <thread>
#include <vector>
#include <stdio.h>
#include <stdlib.h>


void AssertF(bool cmp, char const* err_type, char const* err_msg) {

    if(!cmp) {
        printf("%d\n", errno);
        fprintf(stderr, "[%sErr] %s\n", err_type, err_msg);
        fflush(stderr);
        exit(-1);
    }

}

template <typename T>
T AssertNotNull(T obj, char const* err_msg) {
    AssertF(obj != NULL, "AssertNotNull", err_msg);
    return obj;
}

template <typename T>
T AssertNotEq(T obj, T other, char const* err_msg) {

    AssertF(obj != other, "AssertNotEq", err_msg);
    return obj;

}

template <typename T>
T AssertEq(T obj, T other, char const* err_msg) {

    AssertF(obj == other, "AssertEq", err_msg);
    return obj;

}

void GetMyHomeDir(char* my_home, char** environment) {
    int i = 0;
    while(environment[i] != NULL) {
        if(!strncmp(environment[i], "HOME=", 5)) {
            strcpy(my_home, &environment[i][5]);
            return;
        }
        i++;
    }
    AssertF(false, "Fatal", "Home is not in environment variables");
}

bool CanFind(std::string const& msg, std::string const& query) {

    return msg.find(query) != std::string::npos;

}

namespace HTTP {

    namespace RequestMethods {

        enum class E {

            GET,
            POST,
            HEAD,
            
        };

        namespace S {

            static const std::string& GET = "GET";
            static const std::string& POST = "POST";    
            static const std::string& HEAD = "HEAD";

        }

        E Parse(std::string const& msg) {

            if(CanFind(msg, S::GET)) {

                return E::GET;

            } else if(CanFind(msg, S::POST)) {

                return E::POST;

            } else if(CanFind(msg, S::HEAD)) {

                return E::HEAD;

            } else  {

                AssertF(false, "HTTP", "Unable to find Http method");

            }

            exit(-1);

        }

    }

    namespace Versions {

        enum class E {

            ONE_ZERO,
            ONE_ONE,

        };

        namespace S {

            static const std::string& ONE_ZERO = "HTTP/1.0";
            static const std::string& ONE_ONE = "HTTP/1.1";

        }

        E Parse(std::string const& msg) {

            if(CanFind(msg, S::ONE_ZERO)) {

                return E::ONE_ZERO;

            } else if(CanFind(msg, S::ONE_ONE)) {

                return E::ONE_ONE;

            } else {

                AssertF(false, "HTTP", "Unable to find Http Version");

            }

            exit(-1);

        }

        std::string GetString(E version) {

            switch(version) {

                case E::ONE_ONE:
                    return S::ONE_ONE;
                case E::ONE_ZERO:
                    return S::ONE_ZERO;
                default:
                    AssertF(false, "HTTP", "Unable to match Http Version");
                    exit(-1);
            }

        }


    }

    namespace ResponseCodes {

        enum class E {

            OK,
            NOT_FOUND,

        };

        namespace S {

            namespace Codes {

                // 100

                // 200
                static const std::string& OK = "200";

                // 300

                // 400
                static const std::string& NOT_FOUND = "404";

                // 500

            }

            namespace Human {

                // 200
                static const std::string& OK = "OK";

                // 404
                static const std::string& NOT_FOUND = "Not Found";

            }

        }

        std::string GetCodeString(E response_code) {

            switch(response_code) {

                case E::NOT_FOUND:
                    return S::Codes::NOT_FOUND;

                case E::OK:
                    return S::Codes::OK;

                default:
                    AssertF(false, "HTTP", "Unable to match Response Code");
                    exit(-1);

            }

        }

        std::string GetHumanString(E response_code) {

            switch(response_code) {

                case E::NOT_FOUND:
                    return S::Human::NOT_FOUND;

                case E::OK:
                    return S::Human::OK;

                default:
                    AssertF(false, "HTTP", "Unable to match Human Response Code");
                    exit(-1);

            }

        }

    }

}

void Split(std::vector<std::string>* out, std::string const& in, std::string const& delim) {

    std::string s = in;

    size_t start = 0;
    size_t end = s.find(delim);

    while(end != std::string::npos) {

        (*out).push_back(s.substr(start, end-start));

        start = end +delim.length();
        end = s.find(delim, start);

    }

    (*out).push_back(s.substr(start, end));

}
// See references
inline bool fileExists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

void HandleGet(int sock_fd, HTTP::Versions::E http_version, std::string uri, std::string const& content){
    // Get file content points to.
    printf("Requestiong folder: %s\n", content.c_str());
    printf("URI is: %s\n", uri.c_str());
    std::string fileToServe;
    fileToServe.append(content.c_str());
    fileToServe.append(uri.c_str());
    printf("File to serve is: %d\n", fileToServe.c_str());
    HTTP::ResponseCodes::E response_code = HTTP::ResponseCodes::E::OK;
    if(!fileExists(fileToServe)){
        fileToServe = "files/not_found.html";
        response_code = HTTP::ResponseCodes::E::NOT_FOUND;
    } 
    std::ifstream ifs(fileToServe);
    std::ifstream file(fileToServe);
    std::string fileContent((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

    std::ifstream in(fileToServe, std::ifstream::ate | std::ifstream::binary);
    int fileLength = in.tellg();
    printf("File length is: %d\n", fileLength);
    
    std::string response = 
    HTTP::Versions::GetString(http_version) + " " + 
    HTTP::ResponseCodes::GetCodeString(response_code) + " " + 
    HTTP::ResponseCodes::GetHumanString(response_code) + "\r\n" + 
    "Host: "+"localhost\r\n"+
    "Content-Length: "+std::to_string(fileLength)+
    "\r\n"+"Content-Type: "+
    "text/html\r\n\r\n"+fileContent + "\r\n";
    //Create pointer
    const char * responsePointer = response.c_str();

    // This is creating headers
    printf("The response is:\n%s\n", response.c_str());
    send(sock_fd, responsePointer, strlen(responsePointer), 0);
}

void SendInitialResponse(int sock_fd, HTTP::Versions::E http_version, std::string uri, std::string const& content, HTTP::RequestMethods::E method) {
    // How to compare?
    if(method == HTTP::RequestMethods::E::GET){
        HandleGet(sock_fd, http_version, uri, content);
    } else{
        //Handle put here
    }
    

}


void HandleConnection(int sock_fd, std::string const& my_home, std::string const& content) {

    char buffer[1024] = {0};
    AssertNotEq(read(sock_fd, buffer, 1024), (ssize_t)-1, "Failed to read from socket");

    std::vector<std::string> strings;

    Split(&strings, buffer, " ");

    HTTP::RequestMethods::E method = HTTP::RequestMethods::Parse(AssertNotNull(strings[0].c_str(), "Split failed"));
    std::string uri = AssertNotNull(strings[1].c_str(), "Split Failed");
    HTTP::Versions::E http_version = HTTP::Versions::Parse(AssertNotNull(strings[2].c_str(), "Split failed"));

    

    SendInitialResponse(sock_fd, http_version, uri, content, method);
    //SendBody(sock_fd, uri, content);

}





int main(int argc, char** argv, char** environment) {

    int port;
    char content[128];
    char my_home[128];

    struct sockaddr_in address;
    int addr_len = sizeof(address);

    int one=1;

    std::vector<std::thread> threads;

    GetMyHomeDir(my_home, environment);

    port = atoi(AssertNotNull(argv[1], "Forgot cmdline args"));
    strcpy(content, AssertNotNull(argv[2], "Forgot cmdline args"));

    int sock_fd = AssertNotEq(socket(AF_INET, SOCK_STREAM, 0), -1, "Socket Failed");

    AssertEq(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one)), 0, "SetSockOpt failed");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    AssertEq(bind(sock_fd, (struct sockaddr*)&address, (socklen_t)addr_len), 0, "Failed to Bind");
    AssertEq(listen(sock_fd, 0), 0, "Failed to Listen");

    while(true) {

        for(std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); ++it) {

            if((*it).joinable()) {

                (*it).join();

            }

        }

        int new_socket = AssertNotEq(accept(sock_fd, (struct sockaddr*)&address, (socklen_t*)&addr_len), -1, "Failed to Accept");

        std::thread thread(HandleConnection, new_socket, my_home, content);

        threads.push_back(std::move(thread));

    }

    return 0;

}

