#include "../../libs/jlibc.h"
int main(int argc, char *argv[])
{
    int socket = _socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (_bind(socket, &addr, sizeof(addr)) < 0)
        return -1;
    if (_listen(socket, 1024) < 0)
        return 4;

    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int running = 1;
    while (running)
    {

        int conn = _accept4(socket, &client_addr, &client_addrlen, 0);
        if (conn >= 0)
        {
            int pid = _fork();
            if (pid == 0)
            {
                _close(socket);
                char buf[256];
                ssize_t n = _recvfrom(conn, buf, sizeof(buf), 0, NULL, NULL);
                _write(conn, buf, n);
                _shutdown(conn, SHUT_RDWR);
                _close(conn);
                _exit(0);
            }
            _close(conn);
        }
    }
    _close(socket);
}
