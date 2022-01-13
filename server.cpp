#include "serverfunc.h"

void getRequest (int sockfd) {
    /* check connection */
    if (recv(sockfd, buffer, BUFFER_SIZE, MSG_PEEK) < 1) {
        if (recv(sockfd, buffer, BUFFER_SIZE, MSG_PEEK) < 0)
            cout << sockfd << " errno : " << errno << endl;
        return closeSocket(sockfd);
    }

    /* putting */
    if (fd2Service.at(sockfd).bytes != 0)
        return putFile(sockfd);
        
    /* recv command */
    bzero(buffer, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) < 1)
        return closeSocket(sockfd);
    
    /* parse command */
    char *cmd = strtok(buffer, " "), *recver, *type;
    switch (cmd[0]) {
        case 'L' :
            if ((cmd = strtok(NULL, " ")) != NULL)
                return setUpService(sockfd, string(cmd));
            else
                return ls(sockfd);
        case 'A' :
            return add(sockfd, string(strtok(NULL, " ")));
        case 'R' :
            return remove(sockfd, string(strtok(NULL, " ")));
        case 'C' :
            return chat(sockfd, string(strtok(NULL, " ")));
        case 'P' :
            recver = strtok(NULL, " ");
            return put(sockfd, string(recver), string(strtok(NULL, " ")));
        case 'G' :
            return get(sockfd, string(strtok(NULL, " ")),
                string(strtok(NULL, " ")), string(strtok(NULL, " ")));
    }
}

int main (int argc , char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    mkdir(database.c_str(), 0777);

    int server_fd = setUpServer(*(++argv)), max_fd = server_fd;
    FD_SET(server_fd, &readFdSet);
    
    while (true) {
        fd_set read_fds = readFdSet, write_fds = writeFdSet;
        check(select((max_fd + 1), &read_fds, &write_fds, NULL, NULL),
            "select failed");

        for (int fd = 0; fd < (max_fd + 1); fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                cout << "read fd : " << fd << endl;
                if (fd != server_fd)
                    getRequest(fd);
                else {
                    int client_fd = setUpClient(server_fd);
                    max_fd = max(client_fd, max_fd);
                    FD_SET(client_fd, &readFdSet);
                }
                cout << "*****************************\n";
            }
            if (FD_ISSET(fd, &write_fds)) {
                cout << "write fd : " << fd << endl;
                getFile(fd);
                cout << "*****************************\n";
            }
        }
    }
    return 0;
}
