#include "server.h"
void OK (int sockfd) {
    if ((send(sockfd, "OK", 2, 0) < 1)
        closeSocket(sockfd);
}
int check (int exp, const char *msg) {
    if (exp > 0)
        return exp;
    perror(msg);
    exit(EXIT_FAILURE);
}
int numOfByte (int sockfd) {
    int num = 0;
    if (recv(sockfd, &(num), sizeof(int), 0) < 1)
        closeSocket(sockfd);
    return num;
}
int numOfFile (string path) {
    int num = 0;
    DIR *dp = opendir(path.c_str());
    while (readdir(dp) != NULL)
        num++;
    closedir(dp);
    return (num - 2);
}
int setUpServer (char *port) {
    int serverFd = check(socket(AF_INET, SOCK_STREAM, 0), "socket failed!");
    user2Friends.clear();
    fd2Service.clear();
    fdUserBmp.clear();
    FD_ZERO(&fds);

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
    int fd = check(accept(sockfd, NULL, NULL), "accept failed!");
    fd2Service.insert(make_pair(fd, Service("")));
    return fd;
}
void setUpService(int sockfd, string user) {
    endService(sockfd);
    fd2Service.insert(make_pair(sockfd, Service(user)));
    if (user2Friends.count(user) < 1)
        setUpUser(user);
    OK(sockfd);
}
void setUpUser (string user) {
    user2Friends.insert(make_pair(user, unordered_set<string>()));
    mkdir((database + "/" + user).c_str(), 777);
}
void endService(int sockfd) {
    if (fd2Service.at(sockfd).fp != NULL)
        fclose(fd2Service.at(sockfd).fp);
    fdUserBmp.left.erase(sockfd);
    fd2Service.erase(sockfd);
}
void closeSocket (int sockfd) {
    FD_CLR(sockfd, &fds);
    endService(sockfd);
    close(sockfd);
}
void ls (int sockfd) {
    string sender = fd2Service.at(sockfd).sender, str = 0;
    for (string s : user2Friends.at(sender))
        str += (s + "\n");
    
    int size = str.length();
    if ((send(sockfd, &size, sizeof(int), 0) < 1) || 
        (send(sockfd, str.c_str(), size, 0) < 1))
        closeSocket(sockfd);
}
void chat (int sockfd, string recver) {
    fd2Service.at(sockfd).path = database + "/" +
        fd2Service.at(sockfd).sender + "/" + recver + "/chat";
    fd2Service.at(sockfd).bytes = numOfByte(sockfd);
    //OK(sockfd);
}
void put (int sockfd, string recver, string type) {
    fd2Service.at(sockfd).path = database + "/" + fd2Service.at(sockfd).sender +
        "/" + recver + "/" + type;
    int num = numOfFile(fd2Service.at(sockfd).path);
    fd2Service.at(sockfd).path += "/[" + type + "_" + to_string(num) + "]";
    fd2Service.at(sockfd).bytes = numOfByte(sockfd);
    //OK(sockfd);
}
void putFile (int sockfd) {
    int n = min(fd2Service.at(sockfd).bytes, BUFFER_SIZE);

    bzero(buffer, BUFFER_SIZE);
    if ((n = recv(sockfd, buffer, n, 0)) < 1)
        return closeSocket(sockfd);
    fd2Service.at(sockfd).bytes -= n;
    OK(sockfd);
    
    FILE *fp = fopen(fd2Service.at(sockfd).path);
    fwrite(buffer, sizeof(char), n, fp);
    fclose(fp);
}
void get (int sockfd, string recver, string type, string file) {
    fd2Service.at(sockfd).path = database + "/" +
        fd2Service.at(fd).client + "/" + recver + "/" + type + "/" + file;
    
    FILE *fp = fopen(path.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    bzero(buffer, BUFFER_SIZE);
    lltoa(size, buffer);
    if (send(sockfd, buffer, BUFFER_SIZE, 0) < 1)
        closeSocket(sockfd);
    fd2Service.at(sockfd).offset = 0;
}
void getFile (int sockfd) {
    FILE *fp = fopen(fd2Service.at(sockfd).path);
    fseek(fp, fd2Service.at(sockfd).offset, SEEK_SET);

    bezero(buffer, BUFFER_SIZE);
    int n = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
    fclose(fp);

    if (send(sockfd, buffer, n, 0) < 1)
        return closeSocket(sockfd);
    fd2Service.at(sockfd).offset += n;
}
/******************************/
void add (int sockfd, string userB) {
    string userA = fd2Service.at(sockfd).sender;

    if (user2Friends.at(userA).count(userB))
        return;
    user2Friends.at(userA).insert(userB);
    user2Friends.at(userB).insert(userA);

    string shared = database + "/shared/" + 
                    ((userA < userB) ? (userA + "&" + userB) : (userB + "&" + userA));
    mkdir(shared.c_str(), 777);
    symlink(shared.c_str(), (database + "/" + userA + "/" + userB).c_str());
    symlink(shared.c_str(), (database + "/" + userB + "/" + userA).c_str());
    OK(sockfd);
}
void remove (int sockfd, string userB) {
    string userA = fd2Service.at(sockfd).sender;

    if (!user2Friends.at(userA).count(userB))
        return;
    user2Friends.at(userA).erase(userB);
    user2Friends.at(userB).erase(userA);

    unlink((database + "/" + userA + "/" + userB).c_str());
    unlink((database + "/" + userB + "/" + userA).c_str());
    string shared = database + "/shared/" + 
                    ((userA < userB) ? (userA + "&" + userB) : (userB + "&" + userA));
    rmdir(shared.c_str());
    OK(sockfd);
}