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
#include <algorithm>
#include <filesystem>
#include "http_parser.h"
#define BUFF_SIZE 1024
using namespace std;

void http_request::display(){
    cout << "========START OF REQUEST========\n";
    cout << "METHOD: " << method <<"\nACTION: " << url << "\nVERSION: " << version << "\nHEADERS: \n";
    for (std::map<string,string>::iterator it = headers.begin(); it != headers.end(); it++){
        cout << it->first << " => " << it->second << endl;
    }
    cout << "CONTENT:\n" << content << endl;
    cout << "========END OF REQUEST========\n";
}
http_request get_http_request(int sockfd){
    http_request req;
    char buffer[BUFF_SIZE];
    string line = "";
    int flag = 0;
    bool first_line = true;
    while(1){
        int n = read(sockfd, buffer, 1);
        if(n <= 0)
            break;
        if(buffer[0] == '\n'){
            line = "";
        }
        else if(buffer[0] != '\r'){
            line += buffer[0];
        }
        else if(buffer[0] == '\r'){
            if(line == ""){
                // got the last \r -> finished
                // read \n
                int n = read(sockfd, buffer, 1);
                break;
            }
            if(first_line){
                stringstream i_ss(line);
                i_ss >> req.method >> req.url >> req.version;
                first_line = false;
            }
            else{
                string key, value;
                int cut = line.find(':');
                key = line.substr(0, cut);
                value = line.substr(cut + 2);
                req.headers.insert(pair<string, string>(key, value));
            }
        }
    }
    if(req.headers.count("Content-Length") != 0){
        int length = stoi(req.headers["Content-Length"]);
        //cout << length << endl;
        string content = "";
        while((int)content.length() < length){
            bzero(buffer, sizeof(buffer));
            int n = read(sockfd, buffer, BUFF_SIZE);
            //buffer[n] = '\0';
            //printf("%s\n", buffer);
            content += string(buffer);
        }
        req.content = content;
    }
    else{
        req.content = "";
    }
    return req;
}

void send_http_response(int sockfd, string filename){
    FILE *fp;
    fp = fopen(filename.c_str(), "rb");
    // check file's size
    struct stat buf;
    stat(filename.c_str(), &buf);
    int size = (int)buf.st_size;
    string response = "HTTP/1.1 200 OK\r\nContent-Length: " + to_string(size) + "\r\n\r\n";
    int bytes_sent = 0;
    // send http response
    while(bytes_sent < response.length()){
        int n = write(sockfd, response.substr(bytes_sent).c_str(), response.length() - bytes_sent);
        bytes_sent += n;
    }
    // send corresponding file
    char buffer[BUFF_SIZE];
    bytes_sent = 0;
    int bytes_read;
    while(bytes_sent < size){
        if((bytes_read = fread(buffer, 1, BUFF_SIZE, fp)) <= 0){
            break;
        }
        size_t cur_bytes_sent = 0;
        while(cur_bytes_sent < bytes_read){
            int n = write(sockfd, buffer + cur_bytes_sent, bytes_read - cur_bytes_sent);
            if(n == -1){
                fclose(fp);
                return;
            }
            cur_bytes_sent += n;
        }
        bytes_sent += cur_bytes_sent;
        bzero(buffer, sizeof(buffer));
    }
    fclose(fp);
    return;
}
void http_redirect(int sockfd, string root){
    string http_str = "HTTP/1.1 303 See Other\r\nLocation: " + root + "\r\n\r\n";
    int cnt = 0;
    while(cnt < http_str.length()){
        int n = write(sockfd, http_str.substr(cnt).c_str(), http_str.length() - cnt);
        cnt += n;
    }
}

void send_to_server(int sockfd, string instr){
    write(sockfd, instr.c_str(), BUFF_SIZE);
    return;
}
string recv_from_server(int sockfd){
    int size;
    read(sockfd, &size, sizeof(size));
    cout << "size is : " << size << endl;
    int cnt = 0;
    char buffer[BUFF_SIZE];
    string result = "";
    while(cnt < size){
        bzero(buffer, sizeof(buffer));
        int n = read(sockfd, buffer, size - cnt);
        cnt += n;
        result += string(buffer);
    }
    cout << result << endl;
    return result;
}
void send_to_server_file(int sockfd, const char* filename){
    FILE *fp = fopen(filename, "rb");
    int size;
    struct stat st;
    stat(filename, &st);
    size = (int)st.st_size;
    write(sockfd, &size, sizeof(size));
    cout << "Finish writing bytenum " << size << " to server~" << endl;
    int sent_cnt = 0;
    int cur_bytes_read;
    char buffer[BUFF_SIZE];
    while(sent_cnt < size){
        cout << "In while loop" << endl;
        if((cur_bytes_read = fread(buffer, 1, sizeof(buffer), fp)) <= 0){
            break;
        }
        size_t cur_bytes_sent = 0;
        while(cur_bytes_sent < cur_bytes_read){
            int n = write(sockfd, buffer + cur_bytes_sent, cur_bytes_read - cur_bytes_sent);
            if(n == -1){
                fclose(fp);
                return;
            }
            cur_bytes_sent += n;
        }
        sent_cnt += cur_bytes_sent;
        bzero(buffer, sizeof(buffer));
    }
    fclose(fp);
}
void recv_from_server_file(int sockfd, const char* filename){
    FILE *fp = fopen(filename, "wb");
    int size;
    read(sockfd, &size, sizeof(size));
    cout << "Recv size : " << size << endl;
    int cnt = 0;
    char buffer[BUFF_SIZE];
    while(cnt < size){
        int n = read(sockfd, buffer, min(sizeof(buffer), (long unsigned int)(size - cnt)));
        cnt += n;
        cout << "This round recv " << n << " byte" << endl;
        fwrite(buffer, 1, n, fp);
        bzero(buffer, sizeof(buffer));
    }
    fclose(fp);
}
map<string, string> parse_http_content(string content){
    map<string, string> ret;
    int begin = 0, end = 0;
    while(end <= content.length()){
        if(end == content.length() || content[end] == '&'){
            string entry = content.substr(begin, end - begin);
            int eq = entry.find("=");
            //cout << entry.substr(0, eq) << " " << entry.substr(eq+1) << endl;
            ret.insert(make_pair(entry.substr(0, eq), entry.substr(eq + 1)));
            begin = end + 1;
            //cout << ret.count("username") << endl;
        }
        end += 1;
    }
    return ret;
}
/*<html><body><h2>The file has arrived</h2>
<img src="../image/a.jpeg"></body></html>*/
void make_image_html(string path){
    string ori_template = "<!DOCTYPE html><html><body><h2>The Image has arrived</h2><img src=";
    ori_template += "\"" + path + "\"" +  "alt=\" The Sent Image \"/>";
    ori_template += "</body></html>";
    ofstream tmp_fstream;
    //cout << ori_template << endl;
    tmp_fstream.open("./basic/display_image.html");
    tmp_fstream << ori_template;
    tmp_fstream.close();
    return;
}
/*<html><body><h2>The file has arrived</h2>
<a download href="../image/a.txt">Click to download</a>
</body></html>*/
void make_file_html(string path){
    string ori_template = "<!DOCTYPE html><html><body><h2>The File has arrived</h2><a download href=";
    ori_template += "\"" + path + "\">Click to download";
    ori_template += "</a></body></html>";
    ofstream tmp_fstream;
    //cout << ori_template << endl;
    tmp_fstream.open("./basic/download_file.html");
    tmp_fstream << ori_template;
    tmp_fstream.close();
    return;
}