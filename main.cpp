#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include <stdlib.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <signal.h>
#include <thread>
#include <vector>

void AssertF(bool cmp, char const* err_type, char const* err_msg) {

    if(!cmp) {
        printf("%d\n", errno);
        fprintf(stderr, "[%sErr] %s\n", err_type, err_msg);
        fflush(stderr);
        exit(-1);
    }

}

// void AssertLtZ(int val, char const* err_msg) {

//     AssertF(val < 0, "LessThanZero", err_msg);

// }

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

namespace HTTP {

    enum class RequestMethod {

        GET,
        POST,
        HEAD,
        
    };

    #define GET_S "GET"
    #define POST_S "POST"
    #define HEAD_S "HEAD"

}

HTTP::RequestMethod GetMethod(std::string const& msg) {

    if(msg.find(GET_S) != std::string::npos) {

        return HTTP::RequestMethod::GET;

    } else if(msg.find(POST_S) != std::string::npos) {

        return HTTP::RequestMethod::POST;

    } else if(msg.find(HEAD_S) != std::string::npos) {

        return HTTP::RequestMethod::HEAD;

    } else  {

        AssertF(false, "HTTP", "Unable to find Http method");

    }

    exit(-1);

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

void HandleConnection(int sock_fd) {

    char buffer[1024] = {0};
    AssertNotEq(read(sock_fd, buffer, 1024), (ssize_t)-1, "Failed to read from socket");
    // printf("%s\n", buffer);

    std::vector<std::string> strings;

    Split(&strings, buffer, " ");

    HTTP::RequestMethod method = GetMethod(AssertNotNull(strings[0].c_str(), "Split failed"));
    std::string uri = AssertNotNull(strings[1].c_str(), "Split Failed");
    


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

        std::thread thread(HandleConnection, new_socket);

        threads.push_back(std::move(thread));

    }

    return 0;

}

