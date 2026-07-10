// hadnling signals
#include "../libs/jlibc.h"

struct sigaction mysig = {0};
sigset_t sigset = 0;
int flags = SA_RESTART | SA_RESTORER;
void handler(int num)
{
    _write_str(1, "Received signal: ");
    _write_int(1, num);
    _write_char(1, '\n');
    _write_str(1, "Exiting from signal handler...\n");
    _exit(0);
}

int main(int argc, char **argv, char **envp)
{
}

// mysig.sa_handler = handler;
// mysig.sa_mask = sigset;
// mysig.sa_flags = flags;
// mysig.sa_restorer = _rt_sigreturn;
// _rt_sigaction(SIGALRM, &mysig, NULL, sizeof(sigset_t));

// _alarm(5);
// _write_str(1, "waiting 5s ..");
// while (1)
// {

// }

// mysig.sa_handler = handler;
// mysig.sa_mask = sigset;
// mysig.sa_flags = flags;
// mysig.sa_restorer = _rt_sigreturn;
// _rt_sigaction(SIGINT, &mysig, NULL, sizeof(sigset_t));
// _write_str(1, "Press Ctrl+C to trigger the signal handler...\n");
// while (1) {
//     // Infinite loop to keep the program running and able to receive signals
// }