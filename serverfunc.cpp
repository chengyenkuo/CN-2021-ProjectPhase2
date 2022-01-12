#include "server.h"

int setUpServer (char *port) {
    int server_fd = check(socket(AF_INET, SOCK_STREAM, 0), "socket failed!");
    user2Friends.clear();
    fd2Service.clear();
    fdUserBmp.clear();
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
int setUpClient (int server_fd) {
    return check(accept(server_fd, NULL, NULL), "accept failed!");
}
void setUpService(int client_fd, string user) {
    if (fd2Service.count(client_fd) > 0)
        endService(client_fd);
    fd2Service.insert(make_pair(client_fd, Service(user)));
    if (user2Friends.count(user) < 1)
        setUpUser(user);
    else {
        /* chat */
    }
}
void setUpUser (string user) {
    user2Friends.insert(make_pair(user, unordered_set<string>()));
    mkdir((database + user).c_str(), 777);
}
/******************************/
void endService(int client_fd) {
    if (fd2Service.at(client_fd).chat != NULL)
        fclose(fd2Service.at(client_fd).chat);
    if (fd2Service.at(client_fd).file != NULL)
        fclose(fd2Service.at(client_fd).file);
    if (fd2Service.at(client_fd).img != NULL)
        fclose(fd2Service.at(client_fd).img);
    fdUserBmp.left.erase(client_fd);
    fd2Service.erase(client_fd);
}
void endConnection (int client_fd) {
    endService(client_fd);
    FD_CLR(client_fd, &fds);
    close(client_fd);
}
/******************************/
void ls (int client_fd) {
    string str = "";
    for (string s : user2Friends.at(fd2Service.at(client_fd).sender))
        str += (s + "\n");
    
    bzero(buffer, BUFFER_SIZE);
    strcat(buffer, str.c_str());

    if (send(client_fd, buffer, BUFFER_SIZE, 0) < 1)
        endService(client_fd);
}
/******************************/
void add (int client_fd, string name) {
    string user = fdUserBmp.left.at(client_fd);
    user2Friends.at(user).insert(name);
    mkdir((database + user + "/" + name).c_str(), 777);
    mkdir((database + name + "/" + user).c_str(), 777);
}
void remove (int client_fd, string name) {
    string user = fdUserBmp.left.at(client_fd);
    user2Friends.at(user).insert(name);
    rmdir((database + user + "/" + name).c_str());
    rmdir((database + name + "/" + user).c_str());
}
/******************************/
void chat (int client_fd, string name) {
    
}

void file (int client_fd, string name) {

}

void img (int client_fd, string name) {

}

int numOfFile (string path) {
    int cnt = 0;
    DIR *dp = opendir(path.c_str());

    while (readdir(dp))
        cnt++;
    return cnt;
}

void get (int client_fd, string sender, string recver, string type, string file) {
    string path = database + "/" + sender + "/" + recver + "/" + type + "/" + file;
    fd2Service.at(client_fd).fp = fopen(path.c_str(), "r");
}

void put (int client_fd, string sender, string recver, string type) {
    string path = database + "/" + sender + "/" + recver + "/" + type + "/";
    path += "[" + type + "]_" + string(numOfFile(path));
    fd2Service.at(client_fd).fp = fopen(path.c_str(), "a");
}

void sendFile (int client_fd) {
    FILE *fp = fd2Service.at(client_fd).fp;
    bzero(buffer, BUFFER_SIZE);

    if (fread(buffer, sizeof(char), BUFFER_SIZE, 0) > 0)
        if (send(client_fd, buffer, BUFFER_SIZE, 0) < 1)
            endConnection(client_fd);
    else {
        fd2Service.at(client_fd).fp = NULL;
        fclose(fp);
    }
}

void recvFile (int client_fd) {
    FILE *fp = fd2Service.at(client_fd).fp;
    bzero(buffer, BUFFER_SIZE);

    int n;
    if ((n = recv(client_fd, buffer, BUFFER_SIZE, 0)) < 1) {
        fclose(fp);
        endConnection(client_fd);
    }
    else {
        fwrite(buffer, sizeof(char), n, fp);
    }
}