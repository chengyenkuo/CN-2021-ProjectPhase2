#include <boost/bimap.hpp>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

using namespace std;

#define BACKLOG 10
#define BUFFER_SIZE 4096
/******************************/
struct Service {
    FILE *fp;
    long bytes, offset;
    string sender, recver, path;
    Service (string user) : fp(NULL), bytes(0), offset(0), sender(user), recver("") {}
};
/******************************/
map<string, unordered_set<string>> user2Friends;
boost::bimap<int, string> fdUserBmp;
map<int, Service> fd2Service;
string database = "database";
char buffer[BUFFER_SIZE];
fd_set fds;
/******************************/
void OK (int sockfd);
int check (int exp, const char *msg);
int numOfByte (int sockfd);
int numOfFile (string path);
int setUpServer (char *port);
int setUpClient (int sockfd);
void setUpService(int sockfd, string user);
void setUpUser (string user);
void endService(int sockfd);
void closeSocket (int sockfd);
void ls (int sockfd);
void chat (int sockfd, string recver);
void put (int sockfd, string recver, string type);
void putFile (int sockfd);
void get (int sockfd, string recver, string type, string file);
void getFile (int sockfd);
/******************************/

void add (int sockfd, string userB);
void remove (int sockfd, string userB);

/******************************/