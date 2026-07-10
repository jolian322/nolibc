#include "../../libs/jlibc.h"
int main()
{
    int socfket = _socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    char sendbuf[] = "hello udp";
    _connect(socfket, &addr, sizeof(addr)); 
    _write(socfket, sendbuf, _strlen(sendbuf));
    memset(&sendbuf, 0, sizeof(sendbuf)); 
    _recvfrom(socfket, sendbuf, sizeof(sendbuf), 0, NULL, NULL);
    _write_str(1, "received: ");
    _write(1, sendbuf, _strlen(sendbuf));
    _write_char(1, '\n');
    return 0;
}