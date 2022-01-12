#include <boost/bimap.hpp>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>

using namespace std;

#define BACKLOG 10
#define BUFFER_SIZE 1024

/******************************/
struct Service {
    int mode;   // CONTROL、CHAT、 FILE、IMG
    string sender, receiver;
    FILE *chatFp, *fileFp, *imgFp;
    FILE *fp;

    Service (string name) : sender(name) {}
};
/******************************/
char buffer[BUFFER_SIZE];
string database = "database";
map<string, unordered_set<string>> user2Friends;
boost::bimap<int, string> fdUserBmp;
map<int, Service> fd2Service;
fd_set fds;
/******************************/
int check (int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return exp;
}
int setUpServer (char *port);
int setUpClient (int server_fd);
void setUpUser (string user);
void setUpService(int client_fd, string user);
void endService(int client_fd);
void endConnection (int client_fd);
void ls (int client_fd);
void add (int client_fd, string name);
void remove (int client_fd, string name);
void chat (int client_fd, string name);
void file (int client_fd, string name);
void img (int client_fd, string name);