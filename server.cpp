#include "server.h"

void parse (int client_fd) {
    /*  check connection !  */
    if (recv(client_fd, buffer, BUFFER_SIZE, MSG_PEEK) < 1) {
        closeConnection();
        if (recv(client_fd, buffer, BUFFER_SIZE, MSG_PEEK) < 0)
            cout << "errno : " << errno << endl;
        return;
    }
    
    /* recv data */
    bzero(buffer, sizeof(buffer));
    if (recv(client_fd, buffer, BUFFER_SIZE, 0) < 1)
        return closeConnection(client_fd);
    cout << "recv : " << buffer << endl;

    /* new user */
    if (fd2User.count(client_fd) < 1) {
        return addUser(client_fd, string(buffer));
    
    /* check command : LIST、ADD、REMOVE、CHAT、LOGIN、IMG、*/
    if (!strncmp(buffer, "list", strlen("list")))
        list(client_fd);
    else if (!strncmp(buffer, "add", strlen("add"))) {
        
    }
    else if (!strncmp(buffer, "delete", strlen("delete"))) {

    }
    else if (!strncmp(buffer, "chat", strlen("chat"))) {

    }
    else {
        cout << "transforming data";
    }
}

int main (int argc , char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    mkdir("server_dir/", 777);

    int server_fd = setupServer(*(++argv)), max_fd = server_fd;

    FD_SET(server_fd, &fds);
    while (true) {
        fd_set read_fds = fds, write_fds = fds;
        check(select((max_fd + 1), &read_fds, &write_fds, NULL, NULL), "select failed");

        for (int fd = 0; fd < (max_fd + 1); fd++) {
            if (FD_ISSET(fd, &write_fds));
            if (FD_ISSET(fd, &read_fds)) {
                if (fd != server_fd)
                    parse(fd);
                else {
                    int client_fd = setupClient(server_fd);
                    max_fd = (client_fd > max_fd) ? client_fd : max_fd;
                    FD_SET(client_fd, &fds);
                }
            }
        }
    }
    return 0;
}