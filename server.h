#include <bits/stdc++.h>
#include <boost/bimap.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

using namespace std;

#define FAILED 0
#define SUCCESSFUL 1
#define BACKLOG 10
#define BUFFER_SIZE 1024

struct Service {
    
}


struct Connection {
    int mode;
    FILE *history, *file, *img;
    string sender, receiver;
};

struct User {
    int fd;
    FILE *fp;
    string name;
    User(string user) : name(user) {}
};
/******************************/
char buffer[BUFFER_SIZE];
string database = "database/";
map<string, unordered_set<string>> client2Friends;
map<int, User> fd2User;
fd_set fds;
boost::bimap<int, Connection> fdConnectionBmp;
boost::bimap<int, String> fdUseBmp;
/******************************/
boost::bimap<int, string> fd_user_bmp;
boost::bimap<int, FILE*> fd_fp_bmp;
unordered_set<int> get_fds;
unordered_set<int> put_fds;

/******************************/
int check (int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return exp;
}
/******************************/
int setupServer (char *port) {
    cout << "setupServer\n";

    int server_fd = check(socket(AF_INET, SOCK_STREAM, 0), "socket failed!");
    client2Friends.clear();
    fd2User.clear();
    FD_ZERO(&fds);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(stoi(string(port)));

    check(bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)), "bind failed!");
    check(listen(server_fd, BACKLOG), "listen failed!");
    return server_fd;
}
/******************************/
int setupClient (int server_fd) {
    cout << "setupClient" << endl;
    return check(accept(server_fd, NULL, NULL), "accept failed!");
}
/******************************/
void addClient (string user) {
    client2Friends.insert(make_pair(user, unordered_set<string>()))
    mkdir((database + user).c_str(), 777);
}
void addUser (int client_fd, string user) {
    fd2User.insert(make_pair(client_fd, User(user)));
    if (client2Friends.count(user) < 1)
        addClient(user);
}
/******************************/
void closeConnection(int client_fd) {
    if (fd2User.at(client_fd).fp != NULL)
        fclose(fd2User.at(client_fd).fp);
    fd2User.erase(client_fd);
    FD_CLR(client_fd, &fds);
    close(client_fd);
}
/******************************/
void list (int client_fd) {
    string str = "";
    for (string s : user2friends.at(fd2User.at(client_fd).name))
        str += (s + "\n");
    
    bzero(buffer, BUFFER_SIZE);
    strcat(buffer, str.c_str());

    if (send(client_fd, buffer, BUFFER_SIZE, 0) < 1)
        closeConnection(client_fd);
}
/******************************/
void add (int client_fd, string name) {
    string user = fd2user.at(client_fd);
    user2friends.at(user).insert(name);
    mkdir((user + "/" + name).c_str(), 777);
}
/******************************/
void remove (int client_fd, string name) {
    string user = fd2user.at(client_fd);
    user2friends.at(user).erase(name);
    rmdir((user + "/" + name).c_str());
}
/******************************/
int checkConnection (int client_fd) {
    cout << "checkConnection\n";
    bzero(&buffer, sizeof(buffer));
    if (recv(client_fd, buffer, sizeof(buffer), MSG_PEEK) < 1)
        return closeConnection(client_fd);
    return SUCCESSFUL;
}
/******************************/
int recvMessage(int client_fd, char *buffer, int length) {
    cout << "recvMessage\n";
    int n;
    if ((n = recv(client_fd, buffer, length, 0)) < 1)
        return closeConnection(client_fd);
    return n;
    //cout << "recvMessage$\n";
}
