#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include "http_parser.h"
#include <algorithm>
#include <filesystem>
#include <signal.h>
#define BUFF_SIZE 1024
#define ERR_EXIT(a){ perror(a); exit(1); }
using namespace std;
int init_client(char* ip_port, struct sockaddr_in addr){
    int sockfd;
    char ip[32] = {'\0'};
    char port[10] = {'\0'};
    int flag = 0;
    int t = 0;
    for(int i = 0; i < strlen(ip_port); i++){
        if(ip_port[i] == ':')
            flag = 1;
        else if(flag == 0)
            ip[i] = ip_port[i];
        else{
            port[t] = ip_port[i];
            t++;
        }
    }
    // Get socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        ERR_EXIT("socket failed\n");
    }
    // Set server address
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));
    //printf("%s %s\n", ip, port);
    // Connect to the server
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        ERR_EXIT("connect failed\n");
    }
    return sockfd;
}
int init_server(int port, struct sockaddr_in &server_addr){
    int server_sockfd;
    if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        ERR_EXIT("socket failed\n")
    }
    int opt = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        ERR_EXIT("setsockopt failed");
    }
    // Set server address information
    bzero(&server_addr, sizeof(server_addr)); // clear the data
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    // Bind the server file descriptor to the server address
    if(::bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        ERR_EXIT("bind failed\n");
    }
    // Listen on the server file descriptor
    if(listen(server_sockfd , 10) < 0){
        ERR_EXIT("listen failed\n");
    }
    return server_sockfd;
}
int main(int argc, char* argv[]){
    signal(SIGPIPE, SIG_IGN);
    struct sockaddr_in cli_addr, serv_addr;
    char buffer[BUFF_SIZE] = {};
    // one for communicating with server(as client), one for recieving browser's request(as server)
    int toserver_sockfd = init_client(argv[1], cli_addr);
    int tobrowser_sockfd = init_server(atoi(argv[2]), serv_addr);
    int browser_sockfd;
    bool browser_connected = 0;
    cout << "\033[1;35mLet's get started...\033[1;35m" << endl;
    // start handling browser request...
    //fd_set readfds;
    bool log_in = false;
    string user = "";
    while(1){
        /*
        FD_ZERO(&readfds);
        FD_SET(toserver_sockfd, &readfds);
        int max_sd;
        if(browser_connected){
            FD_SET(browser_sockfd, &readfds);
            max_sd = max(toserver_sockfd, browser_sockfd);
        }else{
            FD_SET(tobrowser_sockfd, &readfds);
            max_sd = max(toserver_sockfd, tobrowser_sockfd);
        }
        int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
        if ((activity < 0) && (errno!=EINTR))printf("select error"); 
        */
        // accept browser connection
        if(!browser_connected){
            int addrlen = sizeof(serv_addr);
            if((browser_sockfd = accept(tobrowser_sockfd, (struct sockaddr *) &serv_addr, (socklen_t*) &addrlen)) < 0){
                ERR_EXIT("accept failed\n");
            }
            browser_connected = true;
            cout << "\033[1;32mBrowser Connect Success\033[1;32m" << endl;
        }
        if(browser_connected){
            int n;
            char buffer[BUFF_SIZE];
            // check if browser disconnects
            if((n = recv(browser_sockfd, buffer, 1, MSG_PEEK | MSG_DONTWAIT)) == 0){
                close(browser_sockfd);
                browser_connected = 0;
                cout << "\033[1;31mBrowser Disconnect\033[0m" << endl;
            }
            // check the http request
            else{
                http_request req = get_http_request(browser_sockfd);
                cout << "\033[1;31mGet Request\033[0m" << endl;
                //req.display();
                if(req.url.substr(0, 7) == "/image/"){
                    send_http_response(browser_sockfd, "." + req.url);
                }
                else if(req.url == "/favicon.ico"){
                    send_http_response(browser_sockfd, "./image/ig_favicon.ico");
                }
                else if(req.url.substr(0, 11) == "/client_dir"){
                    cout << "sending image file to browser" << endl;
                    send_http_response(browser_sockfd, "." + req.url);
                    cout << "finish sending file to browser" << endl;
                }
                else{
                    if(!log_in){
                        if(req.method == "POST"){
                            // implemnt login (message)
                            if(req.url == "/login"){
                                //cout << req.content << endl;
                                map<string, string> tmp = parse_http_content(req.content);
                                //send instr + username
                                cout << "\033[1;32mReceive Username: \033[0m" << tmp["username"] << endl;
                                user = tmp["username"];
                                send_to_server(toserver_sockfd, "LOGIN " + tmp["username"]);
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mLogin Success\033[0m" << endl;
                                    log_in = true;    
                                }
                                else{
                                    cout << "\033[1;32mLogin Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/");
                            }
                            else{
                                cout << "\033[1;32mUndefined Operation\033[0m" << endl;
                            }
                        }
                        else if(req.method == "GET"){
                            // implement login (interface -> image)
                            if(req.url.substr(0,7) == "/basic/"){
                                send_http_response(browser_sockfd, "." + req.url);
                            }
                            else{
                                //send_http_response(browser_sockfd, "./basic/display.html");
                                send_http_response(browser_sockfd, "./basic/login.html");
                            }
                        }
                    }
                    else{
                        if(req.method == "POST"){
                            // implement add friend, del friend, send message to friend
                            if(req.url == "/add"){
                                map<string, string> tmp = parse_http_content(req.content);
				                cout << tmp["username"] << endl;
                                send_to_server(toserver_sockfd, "ADD " + tmp["username"]);
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mAdd Friend Success\033[0m" << endl;
                                }
                                else{
                                    cout << "\033[1;32mAdd Friend Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/");
                            }
                            else if(req.url == "/remove"){
                                map<string, string> tmp = parse_http_content(req.content);
                                send_to_server(toserver_sockfd, "REMOVE " + tmp["username"]);
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mRemove Friend Success\033[0m" << endl;
                                }
                                else{
                                    cout << "\033[1;32mRemove Friend Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/");
                            }
                            else if(req.url == "/send_mess"){
                                // sending message to others
                                map<string, string> tmp = parse_http_content(req.content);
                                send_to_server(toserver_sockfd, "CHAT " + tmp["recver"]);
                                send_to_server(toserver_sockfd, tmp["text"]);
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mSending(Mess) Success\033[0m" << endl;
                                }
                                else{
                                    cout << "\033[1;32mSending(Mess) Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/chat/" + tmp["recver"]);
                            }
                            else if(req.url == "/send_image"){
                                // sending image
                                map<string, string> tmp = parse_http_content(req.content);
                                send_to_server(toserver_sockfd, "PUT " + tmp["recver"] + " IMAGE");
                                send_to_server_file(toserver_sockfd, ("./image/" + tmp["image"]).c_str());
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mSending(Image) Success\033[0m" << endl;
                                }
                                else{
                                    cout << "\033[1;32mSending(Image) Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/chat/" + tmp["recver"]);
                            }
                            else if(req.url == "/send_file"){
                                // sending file
                                map<string, string> tmp = parse_http_content(req.content);
                                send_to_server(toserver_sockfd, "PUT " + tmp["recver"] + " FILE");
                                send_to_server_file(toserver_sockfd, ("./file/" + tmp["file"]).c_str());
                                cout << "End of transmitting file" << endl;
                                string ce = recv_from_server(toserver_sockfd);
                                if(ce == "OK"){
                                    cout << "\033[1;32mSending(File) Success\033[0m" << endl;
                                }
                                else{
                                    cout << "\033[1;32mSending(File) Fail\033[0m" << endl;
                                }
                                http_redirect(browser_sockfd, "/chat/" + tmp["recver"]);
                            }
                        }
                        else if(req.method == "GET"){
                            // implement open chatroom, list friend, click file
                            if(req.url == "/friends"){
                                send_to_server(toserver_sockfd, "LIST");
                                string tmp = recv_from_server(toserver_sockfd);
                                ofstream tmp_fstream;
                                string path = "./data/friends.json";
                                tmp_fstream.open(path);
                                tmp_fstream << tmp;
                                tmp_fstream.close();
                                cout << "send friend list to browser" << endl;
                                send_http_response(browser_sockfd, path);
                            }
                            else if(req.url.substr(0,6) == "/chat/"){
                                send_http_response(browser_sockfd, "./basic/chatroom.html");
                            }
                            else if(req.url.substr(0,6) == "/view/"){
                                string target = req.url.substr(6);
                                send_to_server(toserver_sockfd, "GET " + target + " FILE CHAT");
                                string path = "./data/chat_record.json";
                                //get the record from server
                                recv_from_server_file(toserver_sockfd, path.c_str());
                                cout << "record received" << endl;
                                send_http_response(browser_sockfd, path);
                            }
                            else if(req.url.substr(0, 13) == "/dload_image/"){
                                //must parse url to sender / recver / filename
                                //b08902015&b08902069_[IMAGE_0]
                                req.url = req.url.substr(13);
                                int mark1 = req.url.find("&");
                                string sender = req.url.substr(0, mark1);
                                int mark2 = req.url.find("_");
                                string recver = req.url.substr(mark1 + 1, mark2 - mark1 - 1);
                                string filename = req.url.substr(mark2 + 1);
                                if(user != sender)
                                    recver = sender;
                                cout << recver << " " << filename << endl;
                                send_to_server(toserver_sockfd, "GET " + recver + " IMAGE " + filename);
                                recv_from_server_file(toserver_sockfd, ("./client_dir_image/" + filename).c_str());
                                cout << "Recv file " + filename + " from server" << endl;
                                make_image_html("../client_dir_image/" + filename);
                                send_http_response(browser_sockfd, "./basic/display_image.html");
                            }
                            else if(req.url.substr(0, 12) == "/dload_file/"){
                                //must parse url to sender / recver / filename
                                //b08902015&b08902069_[IMAGE_0]
                                req.url = req.url.substr(12);
                                int mark1 = req.url.find("&");
                                string sender = req.url.substr(0, mark1);
                                int mark2 = req.url.find("_");
                                string recver = req.url.substr(mark1 + 1, mark2 - mark1 - 1);
                                string filename = req.url.substr(mark2 + 1);
                                if(user != sender)
                                    recver = sender;
                                cout << recver << " " << filename << endl;
                                send_to_server(toserver_sockfd, "GET " + recver + " FILE " + filename);
                                recv_from_server_file(toserver_sockfd, ("./client_dir_file/" + filename).c_str());
                                cout << "Recv file " + filename + " from server" << endl;
                                make_file_html("../client_dir_file/" + filename);
                                send_http_response(browser_sockfd, "./basic/download_file.html");
                            }
                            else{
                                // redirect
                                send_http_response(browser_sockfd, "./basic/main.html");
                                // <script> -> /friends -> append chatroom
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
