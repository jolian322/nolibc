// process management
#include "../libs/jlibc.h"

pid_t _jps(char *path, char **argv)
{
    pid_t ret = _fork();

    if (ret < 0)
    {
        _write_str(2, "Fork error\n");
        return ret;
    }

    if (ret == 0)
    {
        char *envp[] = {NULL};
        _write_str(1, "Executing: ");
        _write_str(1, path);
        _write_char(1, '\n');
        int b = _execve(path, argv, envp);

        _write_str(2, "Error: execve failed\n");
        _write_str(2, "Path: ");
        _write_str(2, path);
        _write_str(2, "\n");
        _write_str(2, "Errno: ");
        _write_int(2, b);
        _write_char(2, '\n');

        _exit(b);
    }

    return ret;
}

pid_t _jps_parse(int argc, char **argv)
{
    if (argc < 2)
    {
        _write_str(2, "Usage: pm <program> [args...]\n");
        return -1;
    }

    return _jps(argv[1], argv + 1);
}

int _jhp_b(pid_t pid)
{
    // handle process with pid *BLOCKING*
    struct rusage usage;
    int status;
    pid_t ret = _wait4(pid, &status, 0, &usage);

    if (ret == -1)
    {
        _write_str(2, "Error: failed to wait for process\n");
        return -1;
    }

    // Print resource usage
    _write_str(1, "User CPU: ");
    _write_int(1, usage.ru_utime.tv_sec);
    _write_str(1, "s\n");

    if (WIFEXITED(status))
    {
        _write_str(1, "Process ");
        _write_int(1, pid);
        _write_str(1, " exited with status: ");
        _write_int(1, WEXITSTATUS(status));
        _write_char(1, '\n');
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        _write_str(1, "Process ");
        _write_int(1, pid);
        _write_str(1, " killed by signal: ");
        _write_int(1, WTERMSIG(status));

        if (WCOREDUMP(status))
        {
            _write_str(1, " (core dumped)");
        }
        _write_char(1, '\n');
        return 128 + WTERMSIG(status);
    }
    else if (WIFSTOPPED(status))
    {
        _write_str(1, "Process ");
        _write_int(1, pid);
        _write_str(1, " stopped by signal: ");
        _write_int(1, WSTOPSIG(status));
        _write_char(1, '\n');
        return 128 + WSTOPSIG(status);
    }
    else if (WIFCONTINUED(status))
    {
        _write_str(1, "Process ");
        _write_int(1, pid);
        _write_str(1, " continued\n");
        return 0;
    }
    else
    {
        _write_str(2, "Error: unknown process status\n");
        return -1;
    }
}
void _psLike()
{
    int status;
    _write_str(1, "PID: ");
    _write_int(1, _getpid());
    _write_str(1, "\nPPID: ");
    _write_int(1, _getppid());
    _write_char(1, '\n');
    pid_t pid = _fork();
    if (pid == 0)
    {
        _write_str(1, "Child PID: ");
        _write_int(1, _getpid());
        _write_char(1, '\n');
        _exit(0);
    }
    pid_t ret = _wait4(pid, &status, 0, NULL); // blocks
}
int main(int argc, char **argv, char **envp)
{
    _psLike();
}

// pid_t pid = _jps_parse(argc, argv);

// if (pid > 0) {
//     return _jhp_b(pid);

// } else if (pid < -1 || pid == 0) {
//     _write_str(2, "Error: failed to spawn\n");
//     return 1;
// }

// return 1;