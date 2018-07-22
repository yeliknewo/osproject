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

int main(int argc, char** argv, char** environment) {

    pid_t pid;
    int port;
    char content[128];
    char my_home[128];

    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(server_addr);

    char file_request[256];
    int one=1;

    std::vector<std::thread> threads;

    GetMyHomeDir(my_home, environment);


    port = atoi(AssertNotNull(argv[1], "Forgot cmdline args"));
    strcpy(content, AssertNotNull(argv[2], "Forgot cmdline args"));

    int sock_fd = AssertNotEq(socket(AF_INET, SOCK_STREAM, 0), -1, "Socket Failed");

    AssertEq(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one)), 0, "SetSockOpt failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    AssertEq(bind(sock_fd, (struct sockaddr*)&server_addr, (socklen_t)addr_len), 0, "Failed to Bind");

    AssertEq(listen(sock_fd, 0), 0, "Failed to Listen");

    while(true) {

        int new_socket = AssertNotEq(accept(sock_fd, (struct sockaddr*)&server_addr, (socklen_t*)&addr_len), -1, "Failed to Accept");

        std::thread thread(HandleConnection, new_socket)

        threads.push_back(thread);

    }



    return 0;

}

