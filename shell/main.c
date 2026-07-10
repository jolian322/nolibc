#include "../libs/jlibc.h"

// jobs
#define MAX_JOBS 1024
typedef enum
{
    EMPTY,
    RUNNING,
    STOPPED
} job_state;
typedef struct job_t
{
    pid_t pid;
    int id; // place in the list
    job_state state;
    // TODO: add the command string
} job_t;
job_t jobs[MAX_JOBS];

void add_job(pid_t pid, job_state state)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].state == EMPTY)
        {
            jobs[i].pid = pid;
            jobs[i].id = i + 1;
            jobs[i].state = state;
            // TODOD add the command string
            return;
        }
    }
}
void remove_job(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].pid == pid)
            jobs[i] = (job_t){0};
}
job_t *get_job(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}
void mark_job_stopped(pid_t pid)
{
    job_t *j = get_job(pid);
    if (j)
        j->state = STOPPED;
}
// interpert
typedef enum
{
    CMD_ADD,     // +
    CMD_UNKNOWN, // UNKNOWN
    CMD_QUIT,    // quit
    CMD_FG,      // fg
    CMD_BG,      // bg
    CMD_JOBS,    // jobs
    CMD_PROGRAM  // a program name in the PATH to execute
} cmd_type;

typedef struct token_t
{
    const char *str;
    size_t len;
    cmd_type type; // enum of command types, e.g. CMD_ADD, CMD_QUIT, etc.
} token_t;

typedef struct command_t
{
    token_t cmd; // the command token
    char *args;  // array of argument strings (separated by space terminators) end with null terminator
} command_t;

struct token_t builtinTokens[] = {
    {.str = "+", .len = 1, .type = CMD_ADD},
    {.str = "quit", .len = 4, .type = CMD_QUIT},
    {.str = "exit", .len = 4, .type = CMD_QUIT}, // treat "exit" as an alias for "quit"
    {.str = "fg", .len = 2, .type = CMD_FG},
    {.str = "bg", .len = 2, .type = CMD_BG},
    {.str = "jobs", .len = 4, .type = CMD_JOBS}};
char *get_path_env(char **envp)
{
    char *path = _getENV("PATH", envp);
    if (path == NULL)
    {
        path = "?";
    }
    return path;
}
void setsignal(int sig, void *hndl);

void bg(const char *cmd_args, char **envp)
{
    // fork and put in background !
    // Extract program name from cmd_args
    char program_name[BUFFER_SIZE_SMALL];
    size_t program_len = _charsTillSep(cmd_args, ' ');
    if (program_len >= BUFFER_SIZE_SMALL)
        program_len = BUFFER_SIZE_SMALL - 1;
    for (size_t i = 0; i < program_len; i++)
        program_name[i] = cmd_args[i];
    program_name[program_len] = '\0';

    pid_t pid = _fork();
    if (pid == 0)
    {
        // Child process: execute program in background
        _setpgid(0, 0); // separate process group
        setsignal(SIGINT, SIG_DFL);
        setsignal(SIGTSTP, SIG_DFL);
        setsignal(SIGCHLD, SIG_DFL);
        setsignal(SIGTTIN, SIG_DFL);
        setsignal(SIGTTOU, SIG_DFL);

        // Redirect background I/O away from the controlling terminal so the shell prompt
        // does not get interleaved with command output and the shell appears to exit.
        int devnull = _open("/dev/null", O_RDWR, 0);
        if (devnull >= 0)
        {
            _dup2(devnull, 0);
            _dup2(devnull, 1);
            _dup2(devnull, 2);
            if (devnull > 2)
                _close(devnull);
        }

        char *path_env = get_path_env(envp);
        char *path = path_env;
        while (*path != '\0')
        {
            size_t path_len = _charsTillSep(path, ':');
            char full_path[BUFFER_SIZE_OPTIMAL];
            size_t fp = 0;
            for (size_t i = 0; i < path_len && fp + 1 < BUFFER_SIZE_OPTIMAL; i++)
                full_path[fp++] = path[i];
            if (fp + 1 < BUFFER_SIZE_OPTIMAL)
                full_path[fp++] = '/';
            for (size_t i = 0; i < program_len && fp + 1 < BUFFER_SIZE_OPTIMAL; i++)
                full_path[fp++] = program_name[i];
            full_path[fp] = '\0';

            // Parse arguments and build argv array
            char *argv_exec[256]; // max 256 arguments
            int argc_exec = 0;
            argv_exec[argc_exec++] = program_name;

            // Find and parse remaining arguments
            char *arg_ptr = (char *)cmd_args;
            // Skip past the program name first
            while (*arg_ptr != ' ' && *arg_ptr != '\0' && *arg_ptr != '\n')
                arg_ptr++;

            while (*arg_ptr != '\0' && *arg_ptr != '\n' && argc_exec < 255)
            {
                // Skip whitespace
                while (*arg_ptr == ' ')
                    arg_ptr++;

                // Check if we've reached end of input
                if (*arg_ptr == '\0' || *arg_ptr == '\n')
                    break;

                // Found start of an argument
                argv_exec[argc_exec++] = arg_ptr;

                // Find end of argument (next space, newline, or end of string)
                while (*arg_ptr != ' ' && *arg_ptr != '\0' && *arg_ptr != '\n')
                    arg_ptr++;

                if (*arg_ptr == '\n')
                {
                    *arg_ptr = '\0';
                    break;
                }

                // Replace space with null terminator to separate arguments
                if (*arg_ptr == ' ')
                {
                    *arg_ptr = '\0';
                    arg_ptr++;
                }
            }
            argv_exec[argc_exec] = NULL; // Null-terminate the argv array

            _execve(full_path, argv_exec, envp);

            if (path[path_len] == ':')
                path += path_len + 1;
            else
                break;
        }
        _exit(-1);
    }
    else if (pid > 0)
    {
        // Parent process: add job to background
        add_job(pid, RUNNING);
        _write_str(1, "[Background] pid=");
        _write_int(1, pid);
        _write_char(1, '\n');
    }
}

token_t construct_token(const char *str, size_t len)
{
    token_t token;
    token.str = str;
    token.len = len;
    token.type = CMD_PROGRAM;
    // search in builtinTokens for a match
    for (size_t i = 0; i < sizeof(builtinTokens) / sizeof(builtinTokens[0]); ++i)
    {
        if (len == builtinTokens[i].len && _strncmp(str, builtinTokens[i].str, len) == 0)
        {
            token.type = builtinTokens[i].type;
            break;
        }
    }
    return token;
}
static inline void _sigemptyset(sigset_t *set)
{
    // zero out all 128 bytes
    unsigned long *p = (unsigned long *)set;
    for (size_t i = 0; i < sizeof(sigset_t) / sizeof(unsigned long); i++)
        p[i] = 0;
}

static inline void _sigaddset(sigset_t *set, int sig)
{
    // signals are 1-indexed, bit position is (sig - 1)
    int idx = (sig - 1) / (sizeof(unsigned long) * 8);
    int bit = (sig - 1) % (sizeof(unsigned long) * 8);
    ((unsigned long *)set)[idx] |= (1UL << bit);
}
void setsignal(int sig, void *hndl)
{
    struct sigaction sa = {0};
    sa.sa_handler = hndl;
    sa.sa_flags = SA_RESTORER | SA_RESTART;
    sa.sa_restorer = _rt_sigreturn;
    _rt_sigaction(sig, &sa, NULL, sizeof(sigset_t));
}
void eval(const command_t command, char outbuffer[BUFFER_SIZE_OPTIMAL])
{
    switch (command.cmd.type)
    {
    case CMD_ADD:
    {
        char *ptr = command.args;
        long sum = 0;
        while (*ptr != '\0' && *ptr != '\n')
        {
            while (*ptr == ' ')
            {
                ptr++;
            }
            if (*ptr == '\0' || *ptr == '\n')
            {
                break;
            }

            size_t arg_len = _charsTillSep(ptr, ' ');
            if (arg_len == 0)
            {
                break;
            }

            long arg_sum = 0;
            int valid = 1;
            for (size_t i = 0; i < arg_len; i++)
            {
                char c = ptr[i];
                if (c >= '0' && c <= '9')
                {
                    arg_sum = arg_sum * 10 + (c - '0');
                }
                else
                {
                    valid = 0;
                    break;
                }
            }
            if (valid)
            {
                sum += arg_sum;
            }
            ptr += arg_len;
            if (*ptr == ' ')
            {
                ptr++;
            }
        }

        int idx = 0;
        if (sum == 0)
        {
            outbuffer[idx++] = '0';
        }
        else
        {
            if (sum < 0)
            {
                outbuffer[idx++] = '-';
                unsigned int unum = -(unsigned int)sum;
                int start = idx;
                do
                {
                    outbuffer[idx++] = '0' + (unum % 10);
                    unum /= 10;
                } while (unum > 0);
                for (int j = start, k = idx - 1; j < k; j++, k--)
                {
                    char temp = outbuffer[j];
                    outbuffer[j] = outbuffer[k];
                    outbuffer[k] = temp;
                }
            }
            else
            {
                unsigned int unum = (unsigned int)sum;
                int start = idx;
                do
                {
                    outbuffer[idx++] = '0' + (unum % 10);
                    unum /= 10;
                } while (unum > 0);
                for (int j = start, k = idx - 1; j < k; j++, k--)
                {
                    char temp = outbuffer[j];
                    outbuffer[j] = outbuffer[k];
                    outbuffer[k] = temp;
                }
            }
        }
        outbuffer[idx] = '\0';
        break;
    }
    default:
        outbuffer[0] = '\0';
        return;
    }
}
int path_env_len(char *env_path)
{
    int len = 0;
    for (size_t i = 0; i < _strlen(env_path); i++)
    {
        if (env_path[i] == ':')
        {
            len++;
        }
    }
    return len + 1;
}

char *get_cwd(char **envp)
{
    char *cwd = _getENV("PWD", envp);
    if (cwd == NULL)
    {
        cwd = "?";
    }
    return cwd;
}
char *get_name(char **envp)
{
    char *name = _getENV("LOGNAME", envp);
    if (name == NULL)
    {
        name = "?";
    }
    return name;
}
char *get_host(char **envp)
{
    char *name = _getENV("NAME", envp);
    if (name == NULL)
    {
        name = "?";
    }
    return name;
}
// input buffer for the shell
char buff[BUFFER_SIZE_OPTIMAL];
char input_buf[BUFFER_SIZE_OPTIMAL];
size_t input_len = 0;
size_t input_pos = 0;
// output buffer for the shell
char outbuff[BUFFER_SIZE_OPTIMAL];
// states of the main routine:
enum
{
    PROMPT,
    WAIT,
    READ,
    EVAL
} state = PROMPT;
// current command token
token_t current_token;
void hndlchild(int num)
{
    (void)num;
    // should block signals here!
    int status;
    pid_t pid;
    // handle childern
    while ((pid = _wait4(-1, &status, WNOHANG | WUNTRACED, NULL)) > 0)
    {
        if (WIFSTOPPED(status))
            mark_job_stopped(pid); // keep in table, state = STOPPED
        if (WIFEXITED(status))
            remove_job(pid); // clean it up
        if (WIFSIGNALED(status))
            remove_job(pid); // killed, clean up
    }
}

int main(int argc, char *argv[], char *envp[])
{
    // take control of terminal and set
    pid_t shell_pid = _getpid();
    _setpgid(0, 0);
    _ioctl(0, TIOCSPGRP, (unsigned long)shell_pid);
    setsignal(SIGINT, SIG_IGN);
    setsignal(SIGTSTP, SIG_IGN);
    setsignal(SIGTTIN, SIG_IGN);
    setsignal(SIGTTOU, SIG_IGN);
    setsignal(SIGHUP, SIG_IGN);
    setsignal(SIGCHLD, hndlchild);

    while (1)
    {
        switch (state)
        {
        case PROMPT:
            // display a prompt and read a command line
            _write_str(1, get_name(envp));
            _write_str(1, "@");
            _write_str(1, get_host(envp));
            _write_str(1, ":");
            _write_str(1, get_cwd(envp));
            _write_str(1, "($?)> "); // TODO ($?)
            /** FALLTHROUGH**/
        case WAIT:
            // wait for a user command to be entered and parse the command line into a command and arguments
            if (input_pos >= input_len)
            {
                ssize_t n;
                do
                {
                    n = _read(0, input_buf, BUFFER_SIZE_OPTIMAL - 1);
                } while (n < 0);
                if (n == 0)
                {
                    _exit(0);
                }
                input_len = (size_t)n;
                input_pos = 0;
            }

            // copy the next command from the input buffer into buff
            size_t line_end = input_pos;
            while (line_end < input_len && input_buf[line_end] != '\n')
            {
                line_end++;
            }

            size_t line_len = line_end - input_pos;
            if (line_len >= BUFFER_SIZE_OPTIMAL)
                line_len = BUFFER_SIZE_OPTIMAL - 1;
            for (size_t i = 0; i < line_len; i++)
                buff[i] = input_buf[input_pos + i];
            buff[line_len] = '\0';

            if (line_end < input_len && input_buf[line_end] == '\n')
            {
                input_pos = line_end + 1;
            }
            else
            {
                input_pos = input_len;
            }
            /** FALLTHROUGH**/
        case READ:
            // check token of command and set token variable
            // first check the word size to avoid unnecessary string comparisons
            size_t cmd_len = 0;
            while (buff[cmd_len] != ' ' && buff[cmd_len] != '\n' && buff[cmd_len] != '\0')
            {
                cmd_len++;
            }
            current_token = construct_token(buff, cmd_len); // construct token for command
            /** FALLTHROUGH **/
        case EVAL:
            /* code */
            // just echo the command for now
            switch (current_token.type)
            {
            case CMD_QUIT: // better to be in its own state along with eval and read but for simplicity just handle it here
                _write_str(1, "Quitting...\n");
                _exit(0);
            case CMD_BG:
            {
                // Skip the "bg" command and whitespace
                char *bg_args = buff + cmd_len;
                while (*bg_args == ' ')
                    bg_args++;

                // Check if program name is provided
                if (*bg_args == '\0' || *bg_args == '\n')
                {
                    _write_str(1, "bg: missing program name\n");
                }
                else
                {
                    bg(bg_args, envp);
                }
                state = PROMPT;
                break;
            }
            case CMD_JOBS:
                for (int i = 0; i < MAX_JOBS; i++)
                {
                    if (jobs[i].state != EMPTY)
                    {
                        _write_str(1, "job:");
                        _write_int(1, jobs[i].pid);
                        _write_str(1, " , state:");
                        _write_int(1, jobs[i].state);
                        _write_char(1, '\n');
                    }
                }
                state = PROMPT;
                break;
            case CMD_FG:
            {
                char *fg_args = buff + cmd_len;
                while (*fg_args == ' ')
                    fg_args++;

                // parse job id (%1) or pid (1234)
                job_t *job = NULL;
                if (*fg_args == '%')
                {
                    // job ID lookup
                    int jid = 0;
                    fg_args++; // skip '%'
                    while (*fg_args >= '0' && *fg_args <= '9')
                        jid = jid * 10 + (*fg_args++ - '0');
                    for (int i = 0; i < MAX_JOBS; i++)
                    {
                        if (jobs[i].state != EMPTY && jobs[i].id == jid)
                        {
                            job = &jobs[i];
                            break;
                        }
                    }
                }
                else
                {
                    // raw PID lookup
                    pid_t target = 0;
                    while (*fg_args >= '0' && *fg_args <= '9')
                        target = target * 10 + (*fg_args++ - '0');
                    job = get_job(target);
                }

                if (job == NULL)
                {
                    _write_str(1, "fg: no such job\n");
                    state = PROMPT;
                    break;
                }

                pid_t pid = job->pid;
                job->state = RUNNING;

                // hand terminal
                _ioctl(0, TIOCSPGRP, (unsigned long)&pid);

                // wake it up if it was stopped
                _kill(-pid, SIGCONT);

                // block until it stops or exits
                int status;
                _wait4(pid, &status, WUNTRACED, NULL);
                if (WIFSTOPPED(status))
                    mark_job_stopped(pid);
                else
                    remove_job(pid);

                // reclaim terminal
                pid_t shell_pgid = _getpid();
                _ioctl(0, TIOCSPGRP, (unsigned long)&shell_pgid);

                state = PROMPT;
                break;
            }
            case CMD_ADD:
                command_t cmd_add;
                cmd_add.cmd = current_token;
                cmd_add.args = buff + cmd_len;
                while (*cmd_add.args == ' ')
                {
                    cmd_add.args++;
                }
                // return the sum of the arguments as integers
                eval(cmd_add, outbuff);
                _write_str(1, outbuff);
                _write_char(1, '\n');
                break;
            case CMD_PROGRAM:
                // fork and try to execute the program in the PATH with the arguments
                {
                    sigset_t mask, prev;
                    int status;
                    _sigemptyset(&mask);
                    _sigaddset(&mask, SIGCHLD);
                    _rt_sigprocmask(SIG_BLOCK, &mask, &prev, sizeof(sigset_t));

                    pid_t pid = _fork();
                    if (pid == 0)
                    {
                        _setpgid(0, 0);
                        _rt_sigprocmask(SIG_SETMASK, &prev, NULL, sizeof(sigset_t));
                        setsignal(SIGINT, SIG_DFL);
                        setsignal(SIGTSTP, SIG_DFL);
                        setsignal(SIGCHLD, SIG_DFL);
                        setsignal(SIGTTIN, SIG_DFL);
                        setsignal(SIGTTOU, SIG_DFL);

                        char *path_env = get_path_env(envp);
                        char program_name[BUFFER_SIZE_SMALL];
                        size_t program_len = _charsTillSep(buff, ' ');
                        if (program_len >= BUFFER_SIZE_SMALL)
                            program_len = BUFFER_SIZE_SMALL - 1;
                        for (size_t i = 0; i < program_len; i++)
                            program_name[i] = buff[i];
                        program_name[program_len] = '\0';

                        char args_copy[BUFFER_SIZE_OPTIMAL];
                        size_t args_copy_len = _strlen(buff);
                        if (args_copy_len >= BUFFER_SIZE_OPTIMAL)
                            args_copy_len = BUFFER_SIZE_OPTIMAL - 1;
                        for (size_t i = 0; i <= args_copy_len; i++)
                            args_copy[i] = buff[i];

                        char *argv_exec[256];
                        int argc_exec = 0;
                        argv_exec[argc_exec++] = program_name;

                        char *arg_ptr = args_copy + program_len;
                        while (*arg_ptr != '\0' && *arg_ptr != '\n' && argc_exec < 255)
                        {
                            while (*arg_ptr == ' ')
                                arg_ptr++;
                            if (*arg_ptr == '\0' || *arg_ptr == '\n')
                                break;
                            argv_exec[argc_exec++] = arg_ptr;
                            while (*arg_ptr != ' ' && *arg_ptr != '\0' && *arg_ptr != '\n')
                                arg_ptr++;
                            if (*arg_ptr == ' ')
                            {
                                *arg_ptr = '\0';
                                arg_ptr++;
                            }
                            else if (*arg_ptr == '\n')
                            {
                                *arg_ptr = '\0';
                                break;
                            }
                        }
                        argv_exec[argc_exec] = NULL;

                        char *path = path_env;
                        while (*path != '\0')
                        {
                            size_t path_len = _charsTillSep(path, ':');
                            char full_path[BUFFER_SIZE_OPTIMAL];
                            size_t fp = 0;
                            for (size_t i = 0; i < path_len && fp + 1 < BUFFER_SIZE_OPTIMAL; i++)
                                full_path[fp++] = path[i];
                            if (fp + 1 < BUFFER_SIZE_OPTIMAL)
                                full_path[fp++] = '/';
                            for (size_t i = 0; i < program_len && fp + 1 < BUFFER_SIZE_OPTIMAL; i++)
                                full_path[fp++] = program_name[i];
                            full_path[fp] = '\0';

                            _execve(full_path, argv_exec, envp); // reuse same argv every iteration

                            if (path[path_len] == ':')
                                path += path_len + 1;
                            else
                                break;
                        }
                        _exit(-1);
                    }
                    else if (pid > 0)
                    {

                        _setpgid(pid, pid);                        // prevent race condition
                        _ioctl(0, TIOCSPGRP, (unsigned long)&pid); // give control to the job
                        add_job(pid, RUNNING);
                        //_sigprocmask(SIG_SETMASK, &prev, NULL, sizeof(sigset_t));
                        // blocking for now
                        _wait4(pid, &status, WUNTRACED, NULL);
                        if (WIFSTOPPED(status))
                            mark_job_stopped(pid);
                        else
                            remove_job(pid);
                        pid_t shell_pgid = _getpid();
                        _ioctl(0, TIOCSPGRP, (unsigned long)&shell_pgid); // get control of the terminal
                        _rt_sigprocmask(SIG_SETMASK, &prev, NULL, sizeof(sigset_t));
                    }
                    else if (pid < 0)
                    {
                        _write_str(1, "error!");
                    }
                    _rt_sigprocmask(SIG_SETMASK, &prev, NULL, sizeof(sigset_t));
                }
                break;
            default: // CMD_UNKNOWN
                _write_str(1, buff);
                _write_char(1, '\n');
                break;
            }
            state = PROMPT; // go back to prompt
            break;
        }
    }
    return 0;
}