#include <boost/bimap.hpp>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

using namespace std;

#define BACKLOG 10
#define BUFFER_SIZE 1024

/******************************/
struct Service {
    FILE *fp;
    int bytes, offset;
    string sender, recver, path;
    Service (string user) : fp(NULL), bytes(0), offset(0), 
        sender(user), recver(""), path("") {}
};
/******************************/
map<string, unordered_set<string>> user2Friends;
boost::bimap<int, string> fdUserBmp;
map<int, Service> fd2Service;
string database = "database";
fd_set readFdSet, writeFdSet;
char buffer[BUFFER_SIZE];
/******************************/
int check (int exp, const char *msg);
int numOfByte (int sockfd);
int numOfFile (string path);
int setUpServer (char *port);
int setUpClient (int sockfd);
void setUpService(int sockfd, string user);
void setUpUser (string user);
void endService(int sockfd);
void closeSocket (int sockfd);
void OK (int sockfd);
void ls (int sockfd);
void add (int sockfd, string userB);
void remove (int sockfd, string userB);
void chat (int sockfd, string recver);
void put (int sockfd, string recver, string type);
void putFile (int sockfd);
void get (int sockfd, string recver, string type, string file);
void getFile (int sockfd);
string sharedPath (string userA, string userB);
/******************************/
void appendChat(FILE *fp, string sender, string recver, string type, string content) {
    fprintf(fp, "{\"Sender\":\"%s\",\"Recver\":\"%s\",\"Type\":\"%s\",\"Content\":\"%s\"}]}",
            sender.c_str(), recver.c_str(), type.c_str(), content.c_str());
}
