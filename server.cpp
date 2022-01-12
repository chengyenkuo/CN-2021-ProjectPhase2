#include "server.h"

void parse (int client_fd, bool read) {
    /*  check connection !  */
    if (recv(client_fd, buffer, BUFFER_SIZE, MSG_PEEK) < 1) {
        if (recv(client_fd, buffer, BUFFER_SIZE, MSG_PEEK) < 0)
            cout << "errno : " << errno << endl;
        endService(client_fd);
        return;
    }
    
    /* recv data */
    bzero(buffer, sizeof(buffer));
    if (recv(client_fd, buffer, BUFFER_SIZE, 0) < 1)
        return endService(client_fd);
    cout << "recv : " << buffer << endl;

    /* check command : LOGIN、LIST、ADD、REMOVE、CHAT、LOGIN、IMG、OTHERS */
    if (!strncmp(buffer, "LOGIN ", strlen("LOGIN ")))
        setUpService(client_fd, string(buffer + strlen("LOGIN ")));
    else if (!strncmp(buffer, "LIST ", strlen("LIST ")))
        ls(client_fd);
    else if (!strncmp(buffer, "ADD ", strlen("ADD ")))
        add(client_fd, string(buffer + strlen("ADD ")));
    else if (!strncmp(buffer, "REMOVE ", strlen("REMOVE ")))
        remove(client_fd, string(buffer + strlen("REMOVE ")));
    else if (!strncmp(buffer, "CHAT ", strlen("CHAT ")))
        chat(client_fd, string(buffer + strlen("CHAT ")));
    else if (!strncmp(buffer, "GET ", strlen("GET ")))
        get(client_fd, string(buffer + strlen("GET ")));
    else if (!strncmp(buffer, "PUT ", strlen("PUT ")))
        put(client_fd, string(buffer + strlen("PUT ")));
    else {
        if (read) {

        }
        else {

        }
    }
}

int main (int argc , char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    mkdir(database.c_str(), 777);

    int server_fd = setUpServer(*(++argv)), max_fd = server_fd;
    FD_SET(server_fd, &fds);
    
    while (true) {
        fd_set read_fds = fds, write_fds = fds;
        check(select((max_fd + 1), &read_fds, &write_fds, NULL, NULL), "select failed");

        for (int fd = 0; fd < (max_fd + 1); fd++) {
            if (FD_ISSET(fd, &write_fds)) {
                ;
            }
            if (FD_ISSET(fd, &read_fds)) {
                if (fd != server_fd)
                    parse(fd);
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