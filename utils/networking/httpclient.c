#include "../../libs/jlibc.h"


int main(){
    // http request header
    char* const request = "GET / HTTP/1.1\r\nAccept-Language: en-US,en;q=0.5\r\n\r\n";
    int sockfd = _socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = htonl(0x8efb1064);
    int connectstatus =_connect(sockfd, &addr, sizeof(addr));
    if (connectstatus < 0) {
        _write_str(1, "Failed to connect to server\n");
        _write_int(1, connectstatus);
        _write_char(1, '\n');
        return connectstatus;
    }

    _write_str(sockfd, request);
    char buffer[124];
    _recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    _write_str(1, "Request sent successfully\n");
    _shutdown(sockfd, SHUT_RDWR);
    _close(sockfd);
    _write_str(1, "Socket closed\n");
    _write_str(1, buffer); 
}