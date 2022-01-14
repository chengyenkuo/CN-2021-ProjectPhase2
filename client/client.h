#include <bits/stdc++.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/stat.h>

#define FAILED 0
#define SUCCESSFUL 1
#define BUFFER_SIZE 4096

using namespace std;
/******************************/
const char *dir = "client/";
char request[BUFFER_SIZE], response[BUFFER_SIZE];
/******************************/
void check (int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}
void setupConnection (int sockfd, char *address) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(strtok(address, ":"));
    addr.sin_port = htons(stoi(string(strtok(NULL, ":"))));

    check(connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)),
            "connect error!\n");
}