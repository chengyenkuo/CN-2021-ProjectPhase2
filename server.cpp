#include "server.h"

void request (int sockfd) {
    /* check connection */
    if (recv(sockfd, buffer, BUFFER_SIZE, MSG_PEEK) < 1) {
        if (recv(sockfd, buffer, BUFFER_SIZE, MSG_PEEK) < 0)
            cout << sockfd << " errno : " << errno << endl;
        return closeSocket(sockfd);
    }

    /* putting */
    if (fd2Service.at(sockfd).bytes != 0)
        return putFile(sockfd);
    
    int n = numOfByte(sockfd);
    if (n < 1)
        return;
        
    /* recv command */
    bezero(buffer, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) < 1)
        return closeSocket(sockfd);
    OK(sockfd);
    
    /* parse command */
    char command[BUFFER_SIZE];
    memcpy(command, buffer, BUFFER_SIZE);
    command = strtok(command, " ");
    switch (command[0]) {
        case 'L' :
            if ((command = strtok(NULL, " ")) != NULL)
                return setUpService(sockfd, string(command));
            else
                return ls(sockfd);
        case 'A' :
            return add(sockfd, string(strtok(NULL, " ")));
        case 'R' :
            return remove(sockfd, string(strtok(NULL, " ")));
        case 'C' :
            return chat(sockfd, string(strtok(NULL, " ")));
        case 'P' :
            return put(sockfd);
        case 'G' :
            return get(sockfd);
    }
}
/*
void response (int sockfd) {
    getFile(sockfd);
}
*/

int main (int argc , char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    mkdir(database.c_str(), 777);

    int server_fd = setUpServer(*(++argv)), max_fd = server_fd;
    FD_SET(server_fd, &fds);
    
    while (true) {
        fd_set read_fds = fds, write_fds = fds;
        check(select((max_fd + 1), &read_fds, &write_fds, NULL, NULL), "select failed");

        for (int fd = 0; fd < (max_fd + 1); fd++) {
            if (FD_ISSET(fd, &write_fds))
                getFile();
            if (FD_ISSET(fd, &read_fds)) {
                if (fd != server_fd)
                    response(fd);
                else {
                    int client_fd = setUpClient(server_fd);
                    max_fd = max(client_fd, max_fd);
                    FD_SET(client_fd, &fds);
                }
            }
        }
    }
    return 0;
}