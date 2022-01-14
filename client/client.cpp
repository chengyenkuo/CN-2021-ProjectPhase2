#include "client.h"

void OK (int sockfd) {
    int n;
    recv(sockfd, &n, sizeof(int), 0);
    bzero(response, BUFFER_SIZE);
    recv(sockfd, response, n, 0);
    cout << response << endl;
}

int main(int argc , char *argv[]) {
    int sockfd;

    check((sockfd = socket(AF_INET , SOCK_STREAM , 0)), "socket error!\n");
    setupConnection(sockfd, *(++argv));
    
    bzero(&request, BUFFER_SIZE);
    while (fgets(request, BUFFER_SIZE, stdin) != NULL) {
        /* send request */
        request[strlen(request)-1] = '\0';
        send(sockfd, request, BUFFER_SIZE, 0);

        int size;
        if (request[0] == 'L' && request[1] == 'I') {
            recv(sockfd, &size, 4, 0);
            
            if (size) {
                recv(sockfd, response, size, 0);
                cout << response << endl;
            }
            else
                cout << "superedge\n";
        }
        else if (request[0] == 'C') {
            bzero(&request, BUFFER_SIZE);
            fgets(request, BUFFER_SIZE, stdin);
            size = strlen(request) - 1;
            request[size] = '\0';

            send(sockfd, &size, 4, 0);
            send(sockfd, request, size, 0);
            OK(sockfd);
        }
        else if (request[0] == 'P') {
            char *file = strtok(request, " ");
            for (int i = 0; i < 3; i++)
                file = strtok(NULL, " ");
            
            FILE *fp = fopen(file, "r");
            fseek(fp, 0, SEEK_END);
            int size = (int) ftell(fp);
            cout << size << endl;
            send(sockfd, &size, sizeof(int), 0);
            
            fseek(fp, 0, SEEK_SET);
            while ((size = fread(request, sizeof(char), BUFFER_SIZE, fp)) > 0) {
                cout << "send\n";
                send(sockfd, request, size, 0);
            }
            OK(sockfd);
        }
        else
            OK(sockfd);
        bzero(&request, BUFFER_SIZE);
    }
    close(sockfd);
    return 0;
}