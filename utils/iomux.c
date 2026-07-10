#include "../libs/jlibc.h"

int main()
{
    int listenfd = _socket(AF_INET, SOCK_STREAM, 0);

    // // SO_REUSEADDR
    // int opt = 1;
    // _setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8081);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (_bind(listenfd, &addr, sizeof(addr)) < 0)
        return -1;
    if (_listen(listenfd, 1024) < 0)
        return -1;

    // track active fds by index
    int active[__FD_SETSIZE];
    memset(active, 0, sizeof(active));
    active[listenfd] = 1;
    int maxfd = listenfd;

    fd_set readset;

    while (1)
    {
        // rebuild set every iteration
        FD_ZERO(&readset);
        for (int i = 0; i <= maxfd; i++)
        {
            if (active[i])
                FD_SET(i, &readset);
        }

        _select(maxfd + 1, &readset, NULL, NULL, NULL);

        // check all active fds
        for (int i = 0; i <= maxfd; i++)
        {
            if (!active[i] || !FD_ISSET(i, &readset))
                continue;

            if (i == listenfd)
            {
                // new connection
                struct sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int conn = _accept4(listenfd, &client_addr, &client_addrlen, 0);
                if (conn < 0)
                    continue;

                active[conn] = 1;
                if (conn > maxfd)
                    maxfd = conn;

                _write_str(1, "new client: fd ");
                _write_int(1, conn);
                _write_char(1, '\n');
            }
            else
            {
                // existing client has data
                char buf[256];
                ssize_t n = _read(i, buf, sizeof(buf));
                if (n <= 0)
                {
                    // disconnected
                    _write_str(1, "client disconnected: fd ");
                    _write_int(1, i);
                    _write_char(1, '\n');
                    _close(i);
                    active[i] = 0;
                }
                else
                {
                    for (int j = 0; j <= maxfd; j++)
                    {
                        if (!active[j] || j == listenfd || j == i)
                            continue;
                        else{
                            _write(j, buf,n);
                        }
                    }
                }
            }
        }
    }

    _close(listenfd);
    return 0;
}