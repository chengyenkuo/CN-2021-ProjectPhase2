#include "server.h"

int check (int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return exp;
}
int numOfByte (int sockfd) {
    cout << "numOfByte\n";
    int size;
    if (recv(sockfd, &size, sizeof(int), 0) < 1)
        closeSocket(sockfd);
    return size;
}
int numOfFile (string path) {
    cout << "numOfFile\n";
    int num = 0;
    DIR *dp = opendir(path.c_str());
    while (readdir(dp) != NULL)
        num++;
    closedir(dp);
    return (num - 2);
}
int setUpServer (char *port) {
    cout << "setUpServer\n";
    user2Friends.clear();
    fd2Service.clear();
    fdUserBmp.clear();
    FD_ZERO(&readFdSet);

    int serverFd = check(socket(AF_INET, SOCK_STREAM, 0), "socket failed!");
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(stoi(string(port)));

    check(bind(serverFd, (struct sockaddr*) &addr, sizeof(addr)), "bind failed!");
    check(listen(serverFd, BACKLOG), "listen failed!");
    return serverFd;
}
int setUpClient (int sockfd) {
    cout << "setUpClient\n";
    int fd = check(accept(sockfd, NULL, NULL), "accept failed!");
    fd2Service.insert(make_pair(fd, Service(string(""))));
    return fd;
}
void setUpService(int sockfd, string user) {
    cout << "setUpService\n";
    endService(sockfd);
    fd2Service.insert(make_pair(sockfd, Service(user)));
    if (user2Friends.count(user) < 1)
        setUpUser(user);
    OK(sockfd);
}
void setUpUser (string user) {
    cout << "setUpUser\n";
    user2Friends.insert(make_pair(user, unordered_set<string>()));
}
void endService(int sockfd) {
    cout << "endService\n";
    if (fd2Service.at(sockfd).fp != NULL)
        fclose(fd2Service.at(sockfd).fp);
    fdUserBmp.left.erase(sockfd);
    fd2Service.erase(sockfd);
}
void closeSocket (int sockfd) {
    cout << "closeSocket\n";
    FD_CLR(sockfd, &readFdSet);
    if (FD_ISSET(sockfd, &writeFdSet))
        FD_CLR(sockfd, &writeFdSet);
    endService(sockfd);
    close(sockfd);
}
void OK (int sockfd) {
    cout << "OK\n";
    int size = 2;
    if ((send(sockfd, &size, 4, 0) < 1) || (send(sockfd, "OK", size, 0) < 1))
        closeSocket(sockfd);
}
void add (int sockfd, string userB) {
    cout << "add\n";
    string userA = fd2Service.at(sockfd).sender;

    if (user2Friends.at(userA).count(userB))
        return;

    user2Friends.at(userA).insert(userB);
    user2Friends.at(userB).insert(userA);

    string path = sharedPath(userA, userB);
    mkdir(path.c_str(), 0777);
    mkdir((path + "/FILE").c_str(), 0777);
    mkdir((path + "/IMAGE").c_str(), 0777);
    FILE *fp = fopen((path + "/FILE/CHAT").c_str(), "w");
    fclose(fp);
    OK(sockfd);
}
void remove (int sockfd, string userB) {
    string userA = fd2Service.at(sockfd).sender;

    if (!user2Friends.at(userA).count(userB))
        return;
    user2Friends.at(userA).erase(userB);
    user2Friends.at(userB).erase(userA);
    OK(sockfd);
}
void ls (int sockfd) {
    cout << "ls\n";
    string sender = fd2Service.at(sockfd).sender, str = "{\"friends\" : [ ";
    for (string s : user2Friends.at(sender))
        str += ("\"" + s + "\",");
    str.pop_back();
    str += " ]}";
    
    int size = str.length();
    if ((send(sockfd, &size, sizeof(int), 0) < 1) || 
        (send(sockfd, str.c_str(), size, 0) < 1))
        closeSocket(sockfd);
}
void chat (int sockfd, string recver) {
    cout << "chat\n";
    string sender = fd2Service.at(sockfd).sender,
        path = sharedPath(sender, recver) + "/FILE/CHAT";

    int size;
    if (recv(sockfd, &size, sizeof(int), 0) < 1)
        closeSocket(sockfd);
    bzero(buffer, BUFFER_SIZE);
    if (recv(sockfd, buffer, size, 0) < 1)
        closeSocket(sockfd);
    
    sender += ":\n";
    FILE *fp = fopen(path.c_str(), "a");
    fwrite(sender.c_str(), 1, sender.length(), fp);
    fwrite(buffer, sizeof(char), size, fp);
    fwrite("\n", sizeof(char), 1, fp);
    fclose(fp);
    OK(sockfd);
}
void put (int sockfd, string recver, string type) {
    cout << "put\n"
    string sender = fd2Service.at(sockfd).sender;
    fd2Service.at(sockfd).path = sharedPath(sender, recver) + "/" + type;
    
    int num = numOfFile(fd2Service.at(sockfd).path);
    fd2Service.at(sockfd).path += "/[" + type + "_" + to_string(num) + "]";
    fd2Service.at(sockfd).bytes = numOfByte(sockfd);
}
void putFile (int sockfd) {
    cout << "putFile\n";
    int size, bytes = fd2Service.at(sockfd).bytes;
    
    bzero(buffer, BUFFER_SIZE);
    if ((size = recv(sockfd, buffer, min(bytes, BUFFER_SIZE), 0)) < 1)
        return closeSocket(sockfd);
    fd2Service.at(sockfd).bytes -= size;
    
    FILE *fp = fopen(fd2Service.at(sockfd).path.c_str(), "a");
    fwrite(buffer, sizeof(char), size, fp);
    fclose(fp);

    if (!fd2Service.at(sockfd).bytes)
        OK(sockfd);
}
string sharedPath (string userA, string userB) {
    return database + "/" + ((userA < userB) ?
        (userA + "&" + userB) : (userB + "&" + userA));
}
/******************************/
void get (int sockfd, string recver, string type, string file) {
    cout << "get\n";
    fd2Service.at(sockfd).path = database + "/" +
        fd2Service.at(sockfd).sender + "/" + recver + "/" + type + "/" + file;
    
    FILE *fp = fopen(fd2Service.at(sockfd).path.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    int size = (int) ftell(fp);
    fclose(fp);

    if (send(sockfd, &size, sizeof(int), 0) < 1)
        closeSocket(sockfd);
    FD_SET(sockfd, &writeFdSet);
    fd2Service.at(sockfd).offset = 0;
}
void getFile (int sockfd) {
    cout << "getFile\n";
    FILE *fp = fopen(fd2Service.at(sockfd).path.c_str(), "r");
    cout << fd2Service.at(sockfd).path << endl;
    fseek(fp, fd2Service.at(sockfd).offset, SEEK_SET);
    cout << "getFile\n";
    bzero(buffer, BUFFER_SIZE);
    int n = fread(buffer, sizeof(char), BUFFER_SIZE, fp);

    if (send(sockfd, buffer, n, 0) < 1)
        return closeSocket(sockfd);
    fd2Service.at(sockfd).offset += n;
    
    if (feof(fp)) {
        FD_CLR(sockfd, &writeFdSet);
        OK(sockfd);
    }
    fclose(fp);
}

