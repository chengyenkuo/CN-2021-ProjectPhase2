#include <map>
#include <string>
#define BUF_SIZE 1024
#define MAX_CLI 30
using namespace std;

class http_request{
    public:
    string method;
    string url;
    string version;
    map<string, string> headers;
    string content;
    void display();
};
void send_to_server(int sockfd, string instr); //
void send_to_server_file(int sockfd, const char* filename); //
string recv_from_server(int sockfd); //
void recv_from_server_file(int sockfd, const char* filename); //
http_request get_http_request(int sockfd); //
void send_http_response(int sockfd, string file); //
void http_redirect(int sockfd, string root);
map<string, string> parse_http_content(string content);