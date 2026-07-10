#include "../../libs/jlibc.h"

int main()
{
    char buf[256];
    int socfket = _socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    _bind(socfket, &addr, sizeof(addr)); 
    int bufread= _recvfrom(socfket, buf, sizeof(buf), 0, &client_addr, &client_addrlen);
    _sendto(socfket, buf, bufread, 0, &client_addr, client_addrlen);
    _write_str(1,buf);
    _write_char(1, '\n'); 
    return 0;
}