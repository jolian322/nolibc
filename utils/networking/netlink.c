#include "../../libs/jlibc.h"

void print_ip(unsigned char *ip) {
    _write_int(1, ip[0]); _write_char(1, '.');
    _write_int(1, ip[1]); _write_char(1, '.');
    _write_int(1, ip[2]); _write_char(1, '.');
    _write_int(1, ip[3]);
}

int main() {
    int sockfd = _socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) return 1;

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = _getpid();
    _bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    struct {
        struct nlmsghdr hdr;
        struct rtmsg    rt;
    } req;

    memset(&req, 0, sizeof(req));
    req.hdr.nlmsg_len   = NLMSG_LENGTH(sizeof(req.rt));
    req.hdr.nlmsg_type  = RTM_GETROUTE;
    req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.hdr.nlmsg_seq   = 1;
    req.hdr.nlmsg_pid   = _getpid();
    req.rt.rtm_family   = AF_INET;

    _sendto(sockfd, &req, req.hdr.nlmsg_len, 0, NULL, 0);

    char buffer[8192];
    int flags = 0;
    int route_num = 0;

    while (1) {
        ssize_t n = _recvfrom(sockfd, buffer, sizeof(buffer), flags, NULL, NULL);
        if (n < 0) break;
        flags = MSG_DONTWAIT;

        struct nlmsghdr *resp = (struct nlmsghdr *)buffer;
        ssize_t remaining = n;

        while (remaining > 0) {
            if (resp->nlmsg_type == NLMSG_DONE) goto done;
            if (resp->nlmsg_type == NLMSG_ERROR) goto done;
            if (resp->nlmsg_type != RTM_NEWROUTE) goto next;

            struct rtmsg *rt = (struct rtmsg *)(resp + 1);

            // skip non main table routes
            if (rt->rtm_table != RT_TABLE_MAIN) goto next;

            route_num++;
            _write_str(1, "\nroute ");
            _write_int(1, route_num);
            _write_str(1, ":\n");

            // print destination prefix length
            _write_str(1, "  dst_len: /");
            _write_int(1, rt->rtm_dst_len);
            _write_char(1, '\n');

            // parse route attributes
            struct rtattr *attr = (struct rtattr *)(rt + 1);
            int attr_len = resp->nlmsg_len
                           - sizeof(struct nlmsghdr)
                           - sizeof(struct rtmsg);

            while (attr_len > 0 && attr->rta_len >= sizeof(*attr)) {
                void *data = (char *)attr + sizeof(*attr);

                switch (attr->rta_type) {
                    case RTA_DST:
                        _write_str(1, "  dst: ");
                        print_ip(data);
                        _write_char(1, '\n');
                        break;
                    case RTA_GATEWAY:
                        _write_str(1, "  gateway: ");
                        print_ip(data);
                        _write_char(1, '\n');
                        break;
                    case RTA_PREFSRC:
                        _write_str(1, "  prefsrc: ");
                        print_ip(data);
                        _write_char(1, '\n');
                        break;
                    case RTA_OIF:
                        _write_str(1, "  oif index: ");
                        _write_int(1, *(int *)data);
                        _write_char(1, '\n');
                        break;
                    case RTA_PRIORITY:
                        _write_str(1, "  metric: ");
                        _write_int(1, *(int *)data);
                        _write_char(1, '\n');
                        break;
                }

                int len = attr->rta_len;
                // align to 4 bytes
                len = (len + 3) & ~3;
                attr_len -= len;
                attr = (struct rtattr *)((char *)attr + len);
            }

next:
            remaining -= resp->nlmsg_len;
            resp = (struct nlmsghdr *)((char *)resp + resp->nlmsg_len);
        }
    }
done:
    _close(sockfd);
    return 0;
}