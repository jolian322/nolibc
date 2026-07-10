#include "../../libs/jlibc.h"
struct sigaction mysig = {0};
sigset_t sigset = 0;
int flags = SA_RESTART | SA_RESTORER;
void handler(int num)
{
    while(_wait4(-1, NULL, WNOHANG, NULL) > 0); // reap any zombie child processes
}
int main(int argc, char *argv[]) {
    mysig.sa_handler = handler;
    mysig.sa_flags = flags;
    _rt_sigaction(SIGINT, &mysig, NULL, sizeof(sigset_t));
    int socket = _socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    char sendbuf[] = "hello";
    int connectres = _connect(socket, &addr, sizeof(addr));
    if (connectres == 0) {
        // send the message
        size_t len = 0;
        while (sendbuf[len] != '\0') len++;
        _write(socket, sendbuf, len);

        // receive echo back
        char buf[128];
        ssize_t n = _recvfrom(socket, buf, sizeof(buf), 0, NULL, NULL);
        _write_str(1, "received: ");
        _write(1, buf, n);
        _write_char(1, '\n');
}
}