#ifndef JLIBC_H
#define JLIBC_H

/* Types */
typedef unsigned long size_t;   // sizes
typedef long ssize_t;           // sizes or error if < 0
typedef unsigned short umode_t; // mode for file creation
typedef int pid_t;              // process ID type
typedef long time_t;            // time in seconds since the epoch
typedef long suseconds_t;       // microseconds (millionths of a second)
typedef unsigned long sigset_t; // typically 8 bytes, 64 bits
typedef unsigned int uid_t;     // user ID type
typedef struct _IO_FILE FILE;   // file stream type
typedef long long off_t;        // file offset type
// integer types used by networking
typedef unsigned short sa_family_t; // address family type
typedef unsigned int socklen_t;     // length of socket-related data structures
typedef unsigned int in_addr_t;     // IPv4 address (network byte order)
typedef unsigned short in_port_t;   // port number  (network byte order)
typedef struct sockaddr
{
    // generic socket address structure used in various socket-related syscalls (e.g., bind, connect, accept)
    unsigned short sa_family; // address family (e.g., AF_INET for IPv4)
    char sa_data[14];         // protocol-specific address data
} sockaddr;
struct iovec
{
    // structure used for scatter/gather I/O operations (e.g., readv/writev)
    void *iov_base;
    size_t iov_len;
};

typedef struct msghdr
{
    // structure used for sendmsg and recvmsg syscalls to specify message headers, including multiple buffers (iovec), destination address, and ancillary data
    void *msg_name;        // optional address (e.g., for sendto/recvfrom)
    socklen_t msg_namelen; // size of msg_name
    struct iovec *msg_iov; // scatter/gather array of buffers
    size_t msg_iovlen;     // number of elements in msg_iov
    void *msg_control;     // ancillary data buffer
    size_t msg_controllen; // size of ancillary data buffer
    int msg_flags;         // flags on received message (for recvmsg)
};
typedef struct _IO_FILE
{
    // internal buffer for file I/O
    int fd;

    unsigned char *buf;
    unsigned char *rpos, *rend;
    unsigned char *wpos, *wend;
    unsigned char *wbase;

    size_t buf_size;
    unsigned flags;
    size_t (*write)(FILE *, const unsigned char *, size_t);
    size_t (*read)(FILE *, unsigned char *, size_t);
    off_t (*seek)(FILE *, off_t, int);
    int (*close)(FILE *);

    int (*flush)(FILE *);
    int lock;
} FILE;
extern int strcmp(const char *a, const char *b); // compare two strings
typedef struct siginfo_t
{
    // information about a signal, passed to signal handlers when SA_SIGINFO flag is used
    int si_signo;  // Signal number
    int si_errno;  // Errno (unused)
    int si_code;   // Signal code
    pid_t si_pid;  // Sender's PID (for certain signals)
    uid_t si_uid;  // Sender's UID
    void *si_addr; // Fault address (for SIGSEGV, etc.)
    int si_status; // Exit value or signal (for SIGCHLD)
    long si_band;  // Band event (for SIGPOLL)
} siginfo_t;

struct sigaction
{
    // structure used to define signal handlers
    union
    {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, struct siginfo_t *, void *);
    };
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    sigset_t sa_mask;
};

struct timeval
{
    // structure used to represent time values with second and microsecond precision
    time_t tv_sec;       // seconds (whole seconds)
    suseconds_t tv_usec; // microseconds (millionths of a second)
};
struct rusage
{
    //  structure used to report resource usage statistics for a process (used with wait4)
    struct timeval ru_utime; // User CPU time used
    struct timeval ru_stime; // System CPU time used
    long ru_maxrss;          // Maximum resident set size (in kilobytes)
    long ru_ixrss;           // Integral shared memory size (kilobytes * ticks of execution)
    long ru_idrss;           // Integral unshared data size (kilobytes * ticks of execution)
    long ru_isrss;           // Integral unshared stack size (kilobytes * ticks of execution)
    long ru_minflt;          // Page reclaims (soft page faults)
    long ru_majflt;          // Page faults (hard page faults)
    long ru_nswap;           // Swaps
    long ru_inblock;         // Block input operations
    long ru_oublock;         // Block output operations
    long ru_msgsnd;          // IPC messages sent
    long ru_msgrcv;          // IPC messages received
    long ru_nsignals;        // Signals received
    long ru_nvcsw;           // Voluntary context switches
    long ru_nivcsw;          // Involuntary context switches
};
#define __LITTLE_ENDIAN 1234
#define __BYTE_ORDER __LITTLE_ENDIAN
// fdset
/* Number of descriptors that can fit in an `fd_set'.  */
#define __FD_SETSIZE 1024
/* The fd_set member is required to be an array of longs.  */
typedef long int __fd_mask;
/* Some versions of <linux/posix_types.h> define this macros.  */
#undef __NFDBITS

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS (8 * (int)sizeof(__fd_mask))
#define __FD_ELT(d) ((d) / __NFDBITS)
#define __FD_MASK(d) ((__fd_mask)(1UL << ((d) % __NFDBITS)))
/* fd_set for select and pselect.  */
typedef struct
{
    /* XPG4.2 requires this member name.  Otherwise avoid the name
       from the global namespace.  */
#ifdef __USE_XOPEN
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
#define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
#define __FDS_BITS(set) ((set)->__fds_bits)
#endif
} fd_set;
#define __STD_TYPE typedef
#define __SLONGWORD_TYPE long int
#define __SYSCALL_SLONG_TYPE __SLONGWORD_TYPE
#define __TIME_T_TYPE __SYSCALL_SLONG_TYPE
__STD_TYPE __TIME_T_TYPE __time_t; /* Seconds since the Epoch.  */
/* Maximum number of file descriptors in `fd_set'.  */
#define FD_SETSIZE __FD_SETSIZE

/* We don't use `memset' because this would require a prototype and
   the array isn't too big.  */
#define __FD_ZERO(s)                                                   \
    do                                                                 \
    {                                                                  \
        unsigned int __i;                                              \
        fd_set *__arr = (s);                                           \
        for (__i = 0; __i < sizeof(fd_set) / sizeof(__fd_mask); ++__i) \
            __FDS_BITS(__arr)                                          \
        [__i] = 0;                                                     \
    } while (0)
#define __FD_SET(d, s) \
    ((void)(__FDS_BITS(s)[__FD_ELT(d)] |= __FD_MASK(d)))
#define __FD_CLR(d, s) \
    ((void)(__FDS_BITS(s)[__FD_ELT(d)] &= ~__FD_MASK(d)))
#define __FD_ISSET(d, s) \
    ((__FDS_BITS(s)[__FD_ELT(d)] & __FD_MASK(d)) != 0)
struct timespec
{
#ifdef __USE_TIME_BITS64
    __time64_t tv_sec; /* Seconds.  */
#else
    __time_t tv_sec; /* Seconds.  */
#endif
#if __WORDSIZE == 64 || (defined __SYSCALL_WORDSIZE && __SYSCALL_WORDSIZE == 64) || (__TIMESIZE == 32 && !defined __USE_TIME_BITS64)
    __syscall_slong_t tv_nsec; /* Nanoseconds.  */
#else
#if __BYTE_ORDER == __BIG_ENDIAN
    int : 32;         /* Padding.  */
    long int tv_nsec; /* Nanoseconds.  */
#else
    long int tv_nsec; /* Nanoseconds.  */
    int : 32;         /* Padding.  */
#endif
#endif
};

/* Access macros for `fd_set'.  */
#define FD_SET(fd, fdsetp) __FD_SET(fd, fdsetp)
#define FD_CLR(fd, fdsetp) __FD_CLR(fd, fdsetp)
#define FD_ISSET(fd, fdsetp) __FD_ISSET(fd, fdsetp)
#define FD_ZERO(fdsetp) __FD_ZERO(fdsetp)
#define uint32_t unsigned int
#define uint16_t unsigned short
#define F_ERR (1 << 0)                     // error occurred
#define F_EOF (1 << 1)                     // end of file reached
#define F_LBF (1 << 2)                     // line buffered
#define F_NBF (1 << 3)                     // no buffering (unbuffered)
extern size_t _strlen(const char *string); // returns size of a string
/* Syscalls */
// read syscall
extern ssize_t _read(unsigned int fd, void *buff, size_t count);                                                                 // read from a file descriptor
extern ssize_t _write(unsigned int fd, void *buff, size_t count);                                                                // write to a file descriptor
extern int _open(const char *path, int flags, umode_t mode);                                                                     // open a file with specified flags and mode
extern int _close(int fd);                                                                                                       // close a file descriptor
extern pid_t _getpid();                                                                                                          // get the process ID
extern pid_t _getppid();                                                                                                         // get the parent process ID
extern void _exit(int error_code);                                                                                               // exit with status code
extern pid_t _fork();                                                                                                            // creates a child process
extern pid_t _wait4(pid_t pid, int *stat_addr, int options, struct rusage *ru);                                                  // wait for process to change state (exit or stop) and returns its pid
extern int _rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, size_t sigsetsize);                  // examine and change a signal action
extern void _rt_sigreturn();                                                                                                     // return from signal handler
extern int _rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset, size_t sigsetsize);                                   // examine and change blocked signals
extern int _rt_sigsuspend(const sigset_t *mask, size_t sigsetsize);                                                              // temporarily replace the signal mask and suspend execution until a signal is received
extern int _rt_sigpending(sigset_t *set, size_t sigsetsize);                                                                     // examine pending signals
extern int _ioctl(int fd, unsigned int cmd, unsigned long arg);                                                                  // control device
extern int _kill(pid_t pid, int sig);                                                                                            // send a signal to a process
extern unsigned int _alarm(unsigned int seconds);                                                                                // set an alarm clock for delivery of a signal
extern long _execve(const char *filename, char *const argv[], char *const envp[]);                                               // execute a program
extern pid_t _dup(pid_t oldpid);                                                                                                 // duplicate a file descriptor
extern int _dup2(int oldfd, int newfd);                                                                                          // replace a file descriptor
extern void _write_str(int fd, const char *str);                                                                                 // write a null-terminated string to file descriptor
extern void _write_int(int fd, int num);                                                                                         // write an integer to file descriptor
extern void _write_char(int fd, char c);                                                                                         // write a single character to file descriptor
extern void _write_byte_hex(int fd, char num);                                                                                   // write a byte as two hex digits to file descriptor
extern void _write_ptr(int fd, void *ptr);                                                                                       // write a pointer value in hexadecimal format to file descriptor
extern long _countSeperators(const char *str, char sep);                                                                         // count the number of occurrences of a separator character in a string
extern long _charsTillSep(const char *str, char sep);                                                                            // count the number of characters until the first occurrence of a separator character in a string (or end of string if separator is not found)
extern void _replaceChars(const char *str, char oldChar, char newChar);                                                          // replace all occurrences of oldChar with newChar in the string
extern void *_brk(void *addr);                                                                                                   // set new program break
extern void *_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);                                        // map file or new segment
extern int _munmap(unsigned long addr, size_t len);                                                                              // mem unmap
extern int _setpgid(pid_t pid, pid_t pgid);                                                                                      // set group id
extern int _strncmp(const char *s1, const char *s2, size_t n);                                                                   // compare two strings up to n characters
extern char *_getENV(const char *name, char **envp);                                                                             // get the value of an environment variable from envp
extern ssize_t _writev(unsigned long fd, const struct iovec *vec, unsigned long vlen);                                           // writes iovcnt buffers of data described by iov to the file associated with the file descriptor fd
extern int _socket(int domain, int type, int protocol);                                                                          // create an endpoint for communication and return a file descriptor
extern int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);                                                    // bind a name to a socket
extern int _listen(int sockfd, int backlog);                                                                                     // listen for connections on a socket
extern int _accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);                                           // accept a connection on a socket flag 0 for normal accept
extern int _connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);                                                 // initiate a connection on a socket
extern ssize_t _sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen); // send a message on a socket
extern ssize_t _recvfrom(int sockfd, void *buf, size_t size, int flags, struct sockaddr *src_addr, socklen_t *addrlen);          // receive a message from a socket
extern ssize_t _sendmsg(int sockfd, const struct msghdr *msg, int flags);                                                        // send a message on a socket
extern ssize_t _recvmsg(int sockfd, struct msghdr *msg, int flags);                                                              // receive a message from a socket
extern int _shutdown(int sockfd, int how);                                                                                       // shut down part or all of a socket
extern int _select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);                     // monitor multiple file descriptors for readiness to perform I/O
extern pid_t _clone(
    unsigned long flags,
    unsigned long stack,
    int *parent_tid,
    int *child_tid,
    unsigned long tls); // create a new process or thread with specified flags, stack, and optional TID pointers
extern long _futex(
    uint32_t *uaddr,
    int op,
    uint32_t val,
    const struct timespec *utime,
    uint32_t *uaddr2,
    int val3);                                          // perform a futex operation on a futex located at uaddr
extern int _tgkill(pid_t tgid, pid_t tid, int sig);     // send a signal to a specific thread in a specific process
extern pid_t _set_tid_address(int *tidptr);             // set the address of the TID for the calling thread
extern int _mprotect(void *addr, size_t len, int prot); // change protection of memory region
/*
 * cloning flags:
 */

/*
 * Are there any waiters for this robust futex:
 */
#define FUTEX_WAITERS 0x80000000

/*
 * The kernel signals via this bit that a thread holding a futex
 * has exited without unlocking the futex. The kernel also does
 * a FUTEX_WAKE on such futexes, after setting the bit, to wake
 * up any possible waiters:
 */
#define FUTEX_OWNER_DIED 0x40000000

/*
 * The rest of the robust-futex field is for the TID:
 */
#define FUTEX_TID_MASK 0x3fffffff

/*
 * This limit protects against a deliberately circular list.
 * (Not worth introducing an rlimit for it)
 */
#define ROBUST_LIST_LIMIT 2048

/*
 * bitset with all bits set for the FUTEX_xxx_BITSET OPs to request a
 * match of any bit.
 */
#define FUTEX_BITSET_MATCH_ANY 0xffffffff

#define FUTEX_OP_SET 0  /* *(int *)UADDR2 = OPARG; */
#define FUTEX_OP_ADD 1  /* *(int *)UADDR2 += OPARG; */
#define FUTEX_OP_OR 2   /* *(int *)UADDR2 |= OPARG; */
#define FUTEX_OP_ANDN 3 /* *(int *)UADDR2 &= ~OPARG; */
#define FUTEX_OP_XOR 4  /* *(int *)UADDR2 ^= OPARG; */

#define FUTEX_OP_OPARG_SHIFT 8 /* Use (1 << OPARG) instead of OPARG.  */

#define FUTEX_OP_CMP_EQ 0 /* if (oldval == CMPARG) wake */
#define FUTEX_OP_CMP_NE 1 /* if (oldval != CMPARG) wake */
#define FUTEX_OP_CMP_LT 2 /* if (oldval < CMPARG) wake */
#define FUTEX_OP_CMP_LE 3 /* if (oldval <= CMPARG) wake */
#define FUTEX_OP_CMP_GT 4 /* if (oldval > CMPARG) wake */
#define FUTEX_OP_CMP_GE 5 /* if (oldval >= CMPARG) wake */

/* FUTEX_WAKE_OP will perform atomically
   int oldval = *(int *)UADDR2;
   *(int *)UADDR2 = oldval OP OPARG;
   if (oldval CMP CMPARG)
     wake UADDR2;  */

#define FUTEX_OP(op, oparg, cmp, cmparg) \
    (((op & 0xf) << 28) | ((cmp & 0xf) << 24) | ((oparg & 0xfff) << 12) | (cmparg & 0xfff))

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_FD 2
#define FUTEX_REQUEUE 3
#define FUTEX_CMP_REQUEUE 4
#define FUTEX_WAKE_OP 5
#define FUTEX_LOCK_PI 6
#define FUTEX_UNLOCK_PI 7
#define FUTEX_TRYLOCK_PI 8
#define FUTEX_WAIT_BITSET 9
#define FUTEX_WAKE_BITSET 10
#define FUTEX_WAIT_REQUEUE_PI 11
#define FUTEX_CMP_REQUEUE_PI 12
#define FUTEX_LOCK_PI2 13

#define FUTEX_PRIVATE_FLAG 128
#define FUTEX_CLOCK_REALTIME 256
#define FUTEX_CMD_MASK ~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)

#define FUTEX_WAIT_PRIVATE (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#define FUTEX_REQUEUE_PRIVATE (FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PRIVATE (FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_OP_PRIVATE (FUTEX_WAKE_OP | FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI_PRIVATE (FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI2_PRIVATE (FUTEX_LOCK_PI2 | FUTEX_PRIVATE_FLAG)
#define FUTEX_UNLOCK_PI_PRIVATE (FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_TRYLOCK_PI_PRIVATE (FUTEX_TRYLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_BITSET_PRIVATE (FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE (FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_REQUEUE_PI_PRIVATE (FUTEX_WAIT_REQUEUE_PI | \
                                       FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PI_PRIVATE (FUTEX_CMP_REQUEUE_PI | \
                                      FUTEX_PRIVATE_FLAG)

/*
 * Flags for futex2 syscalls.
 *
 * NOTE: these are not pure flags, they can also be seen as:
 *
 *   union {
 *     u32  flags;
 *     struct {
 *       u32 size    : 2,
 *           numa    : 1,
 *                   : 4,
 *           private : 1;
 *     };
 *   };
 */
#define FUTEX2_SIZE_U8 0x00
#define FUTEX2_SIZE_U16 0x01
#define FUTEX2_SIZE_U32 0x02
#define FUTEX2_SIZE_U64 0x03
#define FUTEX2_NUMA 0x04
/*	0x08 */
/*	0x10 */
/*	0x20 */
/*	0x40 */
#define FUTEX2_PRIVATE FUTEX_PRIVATE_FLAG

#define FUTEX2_SIZE_MASK 0x03

/* do not use */
#define FUTEX_32 FUTEX2_SIZE_U32 /* historical accident :-( */

/*
 * Max numbers of elements in a futex_waitv array
 */
#define FUTEX_WAITV_MAX 128
#define CSIGNAL 0x000000ff              /* signal mask to be sent at exit */
#define CLONE_VM 0x00000100             /* set if VM shared between processes */
#define CLONE_FS 0x00000200             /* set if fs info shared between processes */
#define CLONE_FILES 0x00000400          /* set if open files shared between processes */
#define CLONE_SIGHAND 0x00000800        /* set if signal handlers and blocked signals shared */
#define CLONE_PIDFD 0x00001000          /* set if a pidfd should be placed in parent */
#define CLONE_PTRACE 0x00002000         /* set if we want to let tracing continue on the child too */
#define CLONE_VFORK 0x00004000          /* set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT 0x00008000         /* set if we want to have the same parent as the cloner */
#define CLONE_THREAD 0x00010000         /* Same thread group? */
#define CLONE_NEWNS 0x00020000          /* New mount namespace group */
#define CLONE_SYSVSEM 0x00040000        /* share system V SEM_UNDO semantics */
#define CLONE_SETTLS 0x00080000         /* create a new TLS for the child */
#define CLONE_PARENT_SETTID 0x00100000  /* set the TID in the parent */
#define CLONE_CHILD_CLEARTID 0x00200000 /* clear the TID in the child */
#define CLONE_DETACHED 0x00400000       /* Unused, ignored */
#define CLONE_UNTRACED 0x00800000       /* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID 0x01000000   /* set the TID in the child */
#define CLONE_NEWCGROUP 0x02000000      /* New cgroup namespace */
#define CLONE_NEWUTS 0x04000000         /* New utsname namespace */
#define CLONE_NEWIPC 0x08000000         /* New ipc namespace */
#define CLONE_NEWUSER 0x10000000        /* New user namespace */
#define CLONE_NEWPID 0x20000000         /* New pid namespace */
#define CLONE_NEWNET 0x40000000         /* New network namespace */
#define CLONE_IO 0x80000000             /* Clone io context */
/* Flags for the clone3() syscall. */
#define CLONE_CLEAR_SIGHAND 0x100000000ULL /* Clear any signal handler and reset to SIG_DFL. */
#define CLONE_INTO_CGROUP 0x200000000ULL   /* Clone into a specific cgroup given the right permissions. */

/*
 * cloning flags intersect with CSIGNAL so can be used with unshare and clone3
 * syscalls only:
 */
#define CLONE_NEWTIME 0x00000080 /* New time namespace */
#define NULL ((void *)0)         // null pointer constant
#define EOF (-1)                 // end of file constant for functions like getchar() that return int
#define BUFFER_SIZE_SMALL 256    // small buffer size for temporary use (e.g., reading small files, formatting output)
#define BUFFER_SIZE_OPTIMAL 1024 //   optimal buffer size for general use (e.g., reading typical files, formatting output)
#define BUFFER_SIZE_MEDIUM 4096  // medium buffer size for larger data (e.g., reading large files, formatting large output)
#define PAGE_SIZE 4096           // typical page size for memory mapping and buffer alignment
#define BUFFER_SIZE_LARGE 8192   // large buffer size for very large data (e.g., reading very large files, formatting very large output)
// syscalls numbers
#define SYS_read 0              // read from a file descriptor
#define SYS_write 1             // write to a file descriptor
#define SYS_open 2              // open and possibly create a file
#define SYS_close 3             // close a file descriptor
#define SYS_getpid 39           // get the process ID
#define SYS_getppid 110         // get the parent process ID
#define SYS_exit 60             // terminate the calling process
#define SYS_fork 57             // create a child process
#define SYS_wait4 61            //  wait for process to change state (exit or stop) and returns its pid
#define SYS_rt_sigaction 13     // examine and change a signal action
#define SYS_rt_sigreturn 15     // return from signal handler
#define SYS_rt_sigprocmask 14   // examine and change blocked signals
#define SYS_rt_sigsuspend 130   // temporarily replace the signal mask and suspend execution until a signal is received
#define SYS_rt_sigpending 127   // examine pending signals
#define SYS_kill 62             // send a signal to a process
#define SYS_alarm 37            // set an alarm clock for delivery of a signal
#define SYS_execve 59           // execute a program
#define SYS_ioctl 16            // control device
#define SYS_dup 32              // duplicate a file descriptor
#define SYS_dup2 33             // replace a file descriptor
#define SYS_setpgid 109         // set process group ID
#define SYS_mmap 9              // map file or a segment dynamic
#define SYS_brk 12              // set program break
#define SYS_munmap 11           // unmap
#define SYS_writev 20           // write multiple buffers
#define SYS_socket 41           // create an endpoint for communication and return a file descriptor
#define SYS_bind 49             // bind a name to a socket
#define SYS_listen 50           // listen for connections on a socket
#define SYS_accept4 288         // accept a connection on a socket flag 0 for normal accept
#define SYS_connect 42          // initiate a connection on a socket
#define SYS_sendto 44           // send a message on a socket
#define SYS_recvfrom 45         // receive a message from a socket
#define SYS_sendmsg 46          // send a message on a socket
#define SYS_recvmsg 47          // receive a message from a socket
#define SYS_shutdown 48         // shut down part or all of a socket
#define SYS_select 23           // monitor multiple file descriptors for readiness to perform I/O
#define SYS_clone 56            // create a new process or thread with specified flags, stack, and optional TID pointers
#define SYS_futex 202           // perform a futex operation on a futex located at uaddr
#define SYS_tgkill 234          // send a signal to a specific thread in a specific process
#define SYS_set_tid_address 256 // set the address of the TID for the calling thread
#define SYS_mprotect 10         // change protection of memory region
//// access modes
#define O_RDONLY 00      // reading only
#define O_WRONLY 01      // writing only
#define O_RDWR 02        // reading and writing
#define O_PATH 010000000 // path based operations only
//// creation flags
#define O_CREAT 0100        // create if file doesn't exist (requires file mode to rdx)
#define O_EXCL 0200         // create file if doesn't exist (used with O_CREATE) if file exist the syscall will fail
#define O_TRUNC 01000       // truncate file to zero length if it exists
#define O_DIRECTORY 0200000 // fail if the path is not a directory
#define O_NOFOLLOW 0400000  // do not follow symbolic links
#define O_CLOEXEC 02000000  // close file descriptor on exec()
#define O_TMPFILE 020200000 // 	create an unnamed temporary file (requires mode arg)
//// status flags
#define O_APPEND 02000      // always write at the end of the file
#define O_NONBLOCK 04000    // open in non-blocking mode // doesn't wait
#define O_ASYNC 020000      // generates a signal (SIGIO) when input/output is possible.
#define O_DIRECT 040000     // minimize caching effects; I/O is done directly to/from user-space buffers.
#define O_LARGEFILE 0100000 // allow opening files whose size cannot be represented in an off_t
#define O_NOATIME 01000000  // do not update the file last access time (st_atime) when the file is read.
#define O_DSYNC 010000      // write operations wait for data to be physically stored on hardware.
#define O_SYNC 04010000     // write operations wait for both data and metadata (like timestamps) to be stored.
#define O_NOCTTY 0400       // do not assign controlling terminal
//// umode_t permission
////// user (owner)
#define S_IRUSR 00400 // read
#define S_IWUSR 00200 // write
#define S_IXUSR 00100 // execute
#define S_IRWXU 00700 // read, write, and execute
////// group
#define S_IRGRP 00040 // read
#define S_IWGRP 00020 // write
#define S_IXGRP 00010 // execute
#define S_IRWXG 00070 // read, write, and execute
////// others (world)
#define S_IROTH 00004 // read
#define S_IWOTH 00002 // write
#define S_IXOTH 00001 // execute
#define S_IRWXO 00007 // read, write, and execute
/////// special Bits
#define S_ISUID 04000 // set-user-ID bit
#define S_ISGID 02000 // set-group-ID bit
#define S_ISVTX 01000 // sticky bit (restricted deletion)
// options for wait4()
#define WNOHANG 01     // return immediately if no child has changed state // non-blocking wait
#define WUNTRACED 02   // also report stopped children (due to SIGTTIN, SIGTTOU, etc.)
#define WCONTINUED 010 // also report continued children (SIGCONT)
#define WSTOPPED 02    // synonym for WUNTRACED
//// linux‑specific extensions
#define WEXITED 020      // wait for processes that have terminated (for waitid)
#define WNOWAIT 0400     // keep child in waitable state (later wait can retrieve again)
#define __WALL 04000000  // wait for any child (clone or not)
#define __WCLONE 0400000 // wait only for clone children without signal
// True / false tests for options
#define WIFEXITED(status) (((status) & 0x7f) == 0)                            // normal exit?
#define WIFSIGNALED(status) (((signed char)(((status) & 0x7f) + 1) >> 1) > 0) // killed by signal?
#define WIFSTOPPED(status) (((status) & 0xff) == 0x7f)                        // stopped?
#define WIFCONTINUED(status) ((status) == 0xffff)                             // continued?
// Extract values for options
#define WEXITSTATUS(status) (((status) & 0xff00) >> 8) // exit code (if WIFEXITED true)
#define WTERMSIG(status) ((status) & 0x7f)             // signal number (if WIFSIGNALED true)
#define WSTOPSIG(status) (((status) & 0xff00) >> 8)    // stop signal (if WIFSTOPPED true)
#define WCOREDUMP(status) ((status) & 0x80)            // core dumped? (if WIFSIGNALED true)
/* signals */
#define SIGHUP 1      // hangup detected on controlling terminal or death of controlling process
#define SIGINT 2      // interrupt from keyboard
#define SIGQUIT 3     // quit from keyboard
#define SIGILL 4      // illegal instruction
#define SIGTRAP 5     // illegal instruction
#define SIGABRT 6     // abort signal from abort(3)
#define SIGIOT 6      // synonym for SIGABRT
#define SIGBUS 7      // bus error (bad memory access)
#define SIGFPE 8      // floating point exception
#define SIGKILL 9     // cannot be caught or ignored, and the signal handler is not executed. This signal is used to cause immediate program termination.
#define SIGUSR1 10    // user defined signal 1
#define SIGSEGV 11    // invalid memory reference
#define SIGUSR2 12    // user defined signal 2
#define SIGPIPE 13    // write on a pipe with no reader
#define SIGALRM 14    // alarm clock
#define SIGTERM 15    // termination signal
#define SIGSTKFLT 16  // stack fault on coprocessor (unused)
#define SIGCHLD 17    // child stopped or terminated
#define SIGCONT 18    // continue if stopped
#define SIGSTOP 19    // stop process (cannot be caught or ignored)
#define SIGTSTP 20    // background stop
#define SIGTTIN 21    // background read from tty
#define SIGTTOU 22    // background read/write to tty
#define SIGURG 23     // urgent I/O condition
#define SIGXCPU 24    // CPU time limit exceeded
#define SIGXFSZ 25    // CPU time limit exceeded, file size limit exceeded
#define SIGVTALRM 26  // virtual timer expired
#define SIGPROF 27    // profiling timer expired
#define SIGWINCH 28   // window size change
#define SIGIO 29      // I/O possible (pollable event occurred)
#define SIGPOLL SIGIO // pollable event occurred
#define SIGPWR 30     // power failure
#define SIGSYS 31     // bad system call
#define SIGUNUSED 31  // unused signal
/* Signal set operations */
#define SIG_BLOCK 0   // block signals
#define SIG_UNBLOCK 1 // unblock signals
#define SIG_SETMASK 2 // set the set of blocked signals
/* Signal action flags */
#define SA_NOCLDSTOP 1          // do not generate SIGCHLD when children stop
#define SA_NOCLDWAIT 2          // do not transform children into zombies when they terminate
#define SA_SIGINFO 4            // signal handler takes 3 arguments instead of 1 sa_sigaction instead of sa_handler
#define SA_ONSTACK 0x08000000   // use an alternate signal stack
#define SA_RESTART 0x10000000   // restart system calls if possible
#define SA_NODEFER 0x40000000   // do not block the signal from within its own handler
#define SA_RESETHAND 0x80000000 // reset to default handler when signal is delivered
#define SA_RESTORER 0x04000000  // not supported in glibc; used by kernel to indicate presence of sa_restorer field
// Signal set operations (macros or static inline)
#define sigemptyset(set) (*(set) = 0)                             // initialize signal set to empty
#define sigfillset(set) (*(set) = ~0UL)                           // initialize signal set to full (all signals)
#define sigaddset(set, sig) (*(set) |= 1UL << ((sig) - 1))        // add signal to set
#define sigdelset(set, sig) (*(set) &= ~(1UL << ((sig) - 1)))     // remove signal from set
#define sigismember(set, sig) (!!(*(set) & (1UL << ((sig) - 1)))) // check if signal is in set
#define SIG_DFL ((void *)0)                                       // defualt handler for signal
#define SIG_IGN ((void *)1)                                       // ignore signal
#define SIG_ERR ((void *)-1)                                      //
/*
    ioctl is a huge family of device-specific control operations. The specific commands and their arguments depend on the device being controlled. Here are some
    ioctl inputs :
        fd: file descriptor of the device to control
        cmd: request code that specifies the control operation to perform
        arg: argument for the control operation, its meaning depends on the cmd value and the device
*/
/*
    relavent:
        struct termios : terminal I/O interface // used for controlling terminal behavior (input/output processing, line discipline, etc.)
        c_lflag : local modes in struct termios, used to control terminal behavior such as canonical mode, echoing, signal generation, etc.
            ICANON: enable canonical mode (line buffering and special character processing)
            ECHO: enable echoing of input characters
            ISIG: enable generation of signals for special characters (e.g., SIGINT for Ctrl-C)
            IEXTEN: enable implementation-defined input processing (e.g., Ctrl-V for literal next)
        TCGETS : get the parameters associated with the terminal (struct termios)
        TCSETS : set the parameters associated with the terminal (struct termios)
        TIOCGPGRP : get the foreground process group ID of the terminal // who owns the keyboard and can receive input from it
        TIOCSPGRP : set the foreground process group ID of the terminal // give a process control of the terminal and allow it to receive input from the keyboard
*/
/* defintions for tty control */
// types
typedef unsigned int tcflag_t; // terminal control flags
typedef unsigned int speed_t;  // terminal baud rate
typedef unsigned char cc_t;    // terminal control character type
#define NCCS 32                // number of control characters
// structures
struct termios // terminal I/O interface
{
    tcflag_t c_iflag; // input modes
    tcflag_t c_oflag; // output modes
    tcflag_t c_cflag; // control modes
    tcflag_t c_lflag; // local modes
    cc_t c_line;      // line discipline
    cc_t c_cc[NCCS];  // control characters
    speed_t c_ispeed; // input speed
    speed_t c_ospeed; // output speed
};
struct winsize // window size
{
    unsigned short ws_row;    // rows in characters
    unsigned short ws_col;    // columns in characters
    unsigned short ws_xpixel; // horizontal size in pixels (often 0)
    unsigned short ws_ypixel; // vertical size in pixels (often 0)
};
struct ltchars // local special characters used by line disciplines for control functions
{
    char t_intrc;  // interrupt character
    char t_quitc;  // quit character
    char t_startc; // start output character
    char t_stopc;  // stop output character
    char t_suspc;  // suspend character
    char t_dsuspc; // delayed suspend character
    char t_rprntc; // reprint character
    char t_flushc; // flush output character
    char t_werasc; // word erase character
    char t_lnextc; // literal next character
};
// control character indices  (index into c_cc[])
#define VINTR 0     // interrupt character (default: ^C)
#define VQUIT 1     // quit character (default: ^\)
#define VERASE 2    // erase character (default: DEL or ^H)
#define VKILL 3     // kill character; erases current line (default: ^U)
#define VEOF 4      // end-of-file character (canonical mode, default: ^D)
#define VTIME 5     // timeout in deciseconds for non-canonical read
#define VMIN 6      // minimum number of characters for non-canonical read
#define VSWTC 7     // switch character (not used on Linux)
#define VSTART 8    // start output character (default: ^Q)
#define VSTOP 9     // stop output character (default: ^S)
#define VSUSP 10    // suspend character; sends SIGTSTP (default: ^Z)
#define VEOL 11     // additional end-of-line character (canonical mode)
#define VREPRINT 12 // reprint unread characters (default: ^R)
#define VDISCARD 13 // toggle discarding of output (default: ^O)
#define VWERASE 14  // word erase (default: ^W)
#define VLNEXT 15   // literal next; quotes the next input character (default: ^V)
#define VEOL2 16    // secondary end-of-line character (canonical mode)
// input modes  (c_iflag)
#define IGNBRK 0000001  // ignore break condition on input
#define BRKINT 0000002  // break causes input/output queues to be flushed and SIGINT sent
#define IGNPAR 0000004  // ignore framing and parity errors
#define PARMRK 0000010  // mark parity and framing errors with \377 \0 prefix
#define INPCK 0000020   // enable input parity checking
#define ISTRIP 0000040  // strip the eighth bit from input characters
#define INLCR 0000100   // translate NL to CR on input
#define IGNCR 0000200   // ignore CR on input
#define ICRNL 0000400   // translate CR to NL on input (unless IGNCR is set)
#define IUCLC 0001000   // map uppercase to lowercase on input (non-POSIX)
#define IXON 0002000    // enable XON/XOFF flow control on output
#define IXANY 0004000   // any character restarts stopped output (not just XON)
#define IXOFF 0010000   // enable XON/XOFF flow control on input
#define IMAXBEL 0020000 // ring bell when input queue is full
// output modes  (c_oflag)
#define OPOST 0000001  // enable implementation-defined output processing
#define ONLCR 0000002  // map NL to CR-NL on output
#define OLCUC 0000004  // map lowercase to uppercase on output (non-POSIX)
#define OCRNL 0000010  // map CR to NL on output
#define ONOCR 0000020  // do not output CR at column 0
#define ONLRET 0000040 // do not output CR (NL performs CR function)
#define OFILL 0000100  // use fill characters instead of timing for delays
#define OFDEL 0000200  // fill character is DEL (ASCII 127); if unset, fill is NUL
#define NLDLY 0000400  // newline delay mask
#define CRDLY 0003000  // carriage return delay mask
#define TABDLY 0014000 // horizontal tab delay mask
#define BSDLY 0020000  // backspace delay mask
#define VTDLY 0040000  // vertical tab delay mask
#define FFDLY 0100000  // form feed delay mask
// control modes  (c_cflag)
#define CBAUD 0010017 // baud rate mask
// baud rates
#define B0 0000000   // hang up (drop DTR)
#define B50 0000001  // 50 baud
#define B75 0000002  // 75 baud
#define B110 0000003 // 110 baud
#define B134 0000004
#define B150 0000005
#define B200 0000006
#define B300 0000007
#define B600 0000010
#define B1200 0000011
#define B1800 0000012
#define B2400 0000013
#define B4800 0000014
#define B9600 0000015
#define B19200 0000016
#define B38400 0000017
#define B57600 0010001
#define B115200 0010002
#define B230400 0010003
#define B460800 0010004
#define B500000 0010005
#define B576000 0010006
#define B921600 0010007
#define B1000000 0010010
#define B1152000 0010011
#define B1500000 0010012
#define B2000000 0010013
#define B2500000 0010014
#define B3000000 0010015
#define B3500000 0010016
#define B4000000 0010017
//// character size
#define CSIZE 0000060  // character size mask
#define CS5 0000000    // 5-bit characters
#define CS6 0000020    // 6-bit characters
#define CS7 0000040    // 7-bit characters
#define CS8 0000060    // 8-bit characters
#define CSTOPB 0000100 // send two stop bits (if unset: one stop bit)
#define CREAD 0000200  // enable receiver
#define PARENB 0000400 // enable parity generation and detection
#define PARODD 0001000 // use odd parity (if unset: even parity)
#define HUPCL 0002000  // hang up on last close of terminal
#define CLOCAL 0004000 // ignore modem control lines
// local modes  (c_lflag)
#define ISIG 0000001    // generate signal when INTR, QUIT, SUSP, or DSUSP is received
#define ICANON 0000002  // enable canonical mode (line buffering + special character processing)
#define ECHO 0000010    // echo input characters
#define ECHOE 0000020   // echo erase as BS SP BS sequence
#define ECHOK 0000040   // echo KILL by erasing the current line
#define ECHONL 0000100  // echo NL even if ECHO is not set
#define NOFLSH 0000200  // disable flushing of queues when generating SIGINT, SIGQUIT, SIGSUSP
#define TOSTOP 0000400  // send SIGTTOU to background processes writing to the terminal
#define IEXTEN 0100000  // enable implementation-defined input processing (DISCARD, LNEXT, etc.)
#define ECHOCTL 0200000 // echo control characters as ^X (e.g., ^C for INTR)
#define ECHOPRT 0400000 // visual erase mode for hardcopy terminals
#define ECHOKE 01000000 // visual erase for line kill on hardcopy terminals
#define FLUSHO 02000000 // output is being flushed (state determined by driver)
#define PENDIN 04000000 // retain pending input at next read
#define IUTF8 100000000 // support UTF-8 input (do not strip high bit from input)
// line discipline  (c_line)
#define N_TTY 0           // standard tty line discipline
#define N_SLIP 1          // serial line IP (SLIP)
#define N_MOUSE 2         // mouse protocol (PS/2, etc.)
#define N_PPP 3           // Point-to-Point Protocol (PPP)
#define N_STRIP 4         // strip high bit off characters
#define N_AX25 5          // Amateur Radio AX.25
#define N_X25 6           // ITU X.25 PLP
#define N_6PACK 7         // 6PACK (obsolete)
#define N_MASC 8          // Multi-Access Serial Carrier
#define N_R3964 9         // R3964 (obsolete)
#define N_PROFIBUS_FDL 10 // PROFIBUS Fieldbus Data Link
#define N_IRDA 11         // IrDA
#define N_SMSBLOCK 12     // SMS block protocol
#define N_HDLC 13         // HDLC
#define N_SYNC_PPP 14     // Synchronous PPP
#define N_HCI 15          // Bluetooth HCI UART
#define N_IEEE802_15_4 16 // IEEE 802.15.4
#define N_LAPD 17         // LAPD
#define N_CAN 18          // Controller Area Network
#define N_ISDN 19         // ISDN
#define N_ECONET 20       // Acorn Econet
#define N_HDLC_RAW 21     // raw HDLC
#define N_RAW 22          // raw IP
#define N_TTY_VH 255      // virtual console line discipline
// ioctl commands — terminal attributes
#define TCGETS 0x5401  // get terminal attributes;                      arg = struct termios *
#define TCSETS 0x5402  // set terminal attributes immediately;           arg = const struct termios *
#define TCSETSW 0x5403 // set terminal attributes after output drains;   arg = const struct termios *
#define TCSETSF 0x5404 // set terminal attributes after flushing input;  arg = const struct termios *
// ioctl commands — break
#define TCSBRK 0x5409 // send break signal; arg = int (duration in deciseconds; 0 = 250-500ms)
// ioctl commands — flow control
#define TCXONC 0x540a // flow control; arg = int (one of TCOOFF/TCOON/TCIOFF/TCION)
#define TCOOFF 0      // suspend output
#define TCOON 1       // resume output
#define TCIOFF 2      // send STOP character to suspend input
#define TCION 3       // send START character to resume input
// ioctl commands — queue flushing
#define TCFLSH 0x540b // flush queues; arg = int (one of TCIFLUSH/TCOFLUSH/TCIOFLUSH)
#define TCIFLUSH 0    // flush pending input
#define TCOFLUSH 1    // flush pending output
#define TCIOFLUSH 2   // flush both input and output
// ioctl commands — exclusive use
#define TIOCEXCL 0x540c // set exclusive use of terminal; arg = 0
#define TIOCNXCL 0x540d // clear exclusive use of terminal; arg = 0
// ioctl commands — controlling terminal
#define TIOCSCTTY 0x540e // set controlling terminal for calling process; arg = 0
#define TIOCNOTTY 0x5422 // detach from controlling terminal; arg = 0
#define TIOCGSID 0x5429  // get session ID of terminal; arg = int *
// ioctl commands — process group
#define TIOCGPGRP 0x540f // get foreground process group of terminal; arg = int *
#define TIOCSPGRP 0x5410 // set foreground process group of terminal; arg = const int *
// ioctl commands — output queue
#define TIOCOUTQ 0x5411 // get number of bytes in output queue; arg = int *
// ioctl commands — input queue
#define FIONREAD 0x541b // get number of bytes available to read without blocking; arg = int *
// ioctl commands — inject input
#define TIOCSTI 0x5412 // insert character into input queue as if typed; arg = const char *
// ioctl commands — window size
#define TIOCGWINSZ 0x5413 // get terminal window size; arg = struct winsize *
#define TIOCSWINSZ 0x5414 // set terminal window size; arg = const struct winsize *
// ioctl commands — modem control lines
#define TIOCMGET 0x5415 // get modem bits; arg = int *
#define TIOCMBIS 0x5416 // set modem bits; arg = const int *
#define TIOCMBIC 0x5417 // clear modem bits; arg = const int *
#define TIOCMSET 0x5418 // set modem bits (replace all); arg = const int *
/* modem bit flags (used with TIOCMGET / TIOCMBIS / TIOCMBIC / TIOCMSET) */
#define TIOCM_LE 0x001     // line enable
#define TIOCM_DTR 0x002    // data terminal ready
#define TIOCM_RTS 0x004    // request to send
#define TIOCM_ST 0x008     // secondary transmit
#define TIOCM_SR 0x010     // secondary receive
#define TIOCM_CTS 0x020    // clear to send
#define TIOCM_CAR 0x040    // carrier detect
#define TIOCM_CD TIOCM_CAR // carrier detect
#define TIOCM_RNG 0x080    // ring indicator
#define TIOCM_RI TIOCM_RNG // ring indicator
#define TIOCM_DSR 0x100    // data set ready
// ioctl commands — soft carrier
#define TIOCGSOFTCAR 0x542a // get software carrier detect state; arg = int * (1 = enabled)
#define TIOCSSOFTCAR 0x542b // set software carrier detect state; arg = const int *
// ioctl commands — local modes
#define TIOCLGET 0x5424 // get local mode word; arg = int *
#define TIOCLSET 0x5425 // set local mode word (replace all); arg = const int *
#define TIOCLBIS 0x5426 // set bits in local mode word; arg = const int *
// ioctl commands — local special characters
#define TIOCGLTC 0x5427 // get local special characters; arg = struct ltchars *
#define TIOCSLTC 0x5428 // set local special characters; arg = const struct ltchars *
// ioctl commands — console I/O redirection
#define TIOCCONS 0x541d // redirect kernel console output to this fd; arg = int *
// mmap prot
#define PROT_READ 1  // read
#define PROT_WRITE 2 // write
#define PROT_EXEC 4  // execute
#define PROT_NONE 0  // no access
// mmap flags (sharing / visibility)
#define MAP_SHARED 1          // shared mapping (changes written back)
#define MAP_PRIVATE 2         // private copy-on-write mapping
#define MAP_SHARED_VALIDATE 3 // shared with strict flag validation (Linux)
// mmap flags (behaviour modifiers)
#define MAP_FIXED 16                // force mapping at exact address
#define MAP_ANONYMOUS 32            // no file backing (fd = -1)
#define MAP_ANON 32                 // alias for MAP_ANONYMOUS
#define MAP_GROWSDOWN 256           // stack‑like segment (grow down)
#define MAP_LOCKED 8192             // lock pages in RAM
#define MAP_NORESERVE 16384         // do not reserve swap space
#define MAP_POPULATE 32768          // pre‑fault pages (populate page tables)
#define MAP_NONBLOCK 65536          // only populate page tables, no readahead (with MAP_POPULATE)
#define MAP_STACK 131072            // allocate stack (for threading)
#define MAP_HUGETLB 262144          // use huge pages
#define MAP_SYNC 524288             // synchronous page faults for DAX files (requires MAP_SHARED_VALIDATE)
#define MAP_FIXED_NOREPLACE 1048576 // MAP_FIXED but fail if address already mapped
#define MAP_UNINITIALIZED 67108864  // do not clear anonymous memory
/* networking */
// netlink families
#define NETLINK_ROUTE 0
#define NETLINK_UNUSED 1
#define NETLINK_USERSOCK 2
#define NETLINK_FIREWALL 3
#define NETLINK_SOCK_DIAG 4
#define NETLINK_NFLOG 5
#define NETLINK_XFRM 6
#define NETLINK_SELINUX 7
#define NETLINK_ISCSI 8
#define NETLINK_AUDIT 9
#define NETLINK_FIB_LOOKUP 10
#define NETLINK_CONNECTOR 11
#define NETLINK_NETFILTER 12
#define NETLINK_IP6_FW 13
#define NETLINK_DNRTMSG 14
#define NETLINK_KOBJECT_UEVENT 15
#define NETLINK_GENERIC 16
// netlink struct
struct sockaddr_nl
{
    unsigned short nl_family; // AF_NETLINK
    unsigned short nl_pad;    // zero
    unsigned int nl_pid;      // process ID (or 0 for kernel)
    unsigned int nl_groups;   // multicast groups mask
};

struct nlmsghdr
{
    uint32_t nlmsg_len;   // total message length
    uint16_t nlmsg_type;  // what operation (RTM_GETROUTE etc)
    uint16_t nlmsg_flags; // NLM_F_REQUEST, NLM_F_DUMP etc
    uint32_t nlmsg_seq;   // sequence number
    uint32_t nlmsg_pid;   // sender process ID
};
struct rtmsg
{
    unsigned char rtm_family;   // address family (AF_INET)
    unsigned char rtm_dst_len;  // destination prefix length
    unsigned char rtm_src_len;  // source prefix length
    unsigned char rtm_tos;      // type of service
    unsigned char rtm_table;    // routing table (RT_TABLE_MAIN)
    unsigned char rtm_protocol; // routing protocol (RTPROT_STATIC etc)
    unsigned char rtm_scope;    // route scope (RT_SCOPE_UNIVERSE)
    unsigned char rtm_type;     // route type (RTN_UNICAST etc)
    unsigned int rtm_flags;     // flags
};

/* Routing message attributes */

enum rtattr_type_t
{
    RTA_UNSPEC,
    RTA_DST,
    RTA_SRC,
    RTA_IIF,
    RTA_OIF,
    RTA_GATEWAY,
    RTA_PRIORITY,
    RTA_PREFSRC,
    RTA_METRICS,
    RTA_MULTIPATH,
    RTA_PROTOINFO, /* no longer used */
    RTA_FLOW,
    RTA_CACHEINFO,
    RTA_SESSION, /* no longer used */
    RTA_MP_ALGO, /* no longer used */
    RTA_TABLE,
    RTA_MARK,
    RTA_MFC_STATS,
    RTA_VIA,
    RTA_NEWDST,
    RTA_PREF,
    RTA_ENCAP_TYPE,
    RTA_ENCAP,
    RTA_EXPIRES,
    RTA_PAD,
    RTA_UID,
    RTA_TTL_PROPAGATE,
    RTA_IP_PROTO,
    RTA_SPORT,
    RTA_DPORT,
    RTA_NH_ID,
    __RTA_MAX
};

struct rtattr
{
    unsigned short rta_len;
    unsigned short rta_type;
};
enum rt_class_t
{
    RT_TABLE_UNSPEC = 0,
    /* User defined values */
    RT_TABLE_COMPAT = 252,
    RT_TABLE_DEFAULT = 253,
    RT_TABLE_MAIN = 254,
    RT_TABLE_LOCAL = 255,
    RT_TABLE_MAX = 0xFFFFFFFF
};
// nlmsg_type
#define RTM_NEWLINK 16  // create or update network interface
#define RTM_DELLINK 17  // delete network interface
#define RTM_GETLINK 18  // get network interface information
#define RTM_SETLINK 19  // set network interface information
#define RTM_NEWADDR 20  // create or update IP address
#define RTM_DELADDR 21  // delete IP address
#define RTM_GETADDR 22  // get IP address information
#define RTM_NEWROUTE 24 // create or update route
#define RTM_DELROUTE 25 // delete route
#define RTM_GETROUTE 26 // get route information
#define RTM_NEWNEIGH 28 // create or update neighbor (ARP) entry
#define RTM_DELNEIGH 29 // delete neighbor (ARP) entry
#define RTM_GETNEIGH 30 // get neighbor (ARP) entry information
#define RTM_NEWRULE 32  // create or update traffic control rule
#define RTM_DELRULE 33  // delete traffic control rule
#define RTM_GETRULE 34  // get traffic control rule information
// nlmsg_flags
#define NLM_F_REQUEST 1   // this is a request message
#define NLM_F_MULTI 2     // multipart message, terminated by NLMSG_DONE
#define NLM_F_ACK 4       // reply with ack, with zero or error code
#define NLM_F_ECHO 8      // echo this request in the reply
#define NLM_F_DUMP 0x300  // dump was requested (NLM_F_ROOT | NLM_F_MATCH) // return all entries
#define NLM_F_ROOT 0x100  // specify tree root (dump only)
#define NLM_F_MATCH 0x200 // return all matching (dump only)
// netlink NLMSG_
#define NLMSG_ALIGNTO 4U
#define NLMSG_ALIGN(len) (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN ((int)NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh) ((void *)(((char *)nlh) + NLMSG_HDRLEN))
#define NLMSG_NEXT(nlh, len) ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                              (struct nlmsghdr *)(((char *)(nlh)) +   \
                                                  NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh, len) ((len) >= (int)sizeof(struct nlmsghdr) &&       \
                            (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
                            (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh, len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))
#define NLMSG_NOOP 0x1    /* Nothing.             */
#define NLMSG_ERROR 0x2   /* Error                */
#define NLMSG_DONE 0x3    /* End of a dump        */
#define NLMSG_OVERRUN 0x4 /* Data lost            */
#define NETLINK_ADD_MEMBERSHIP 1
#define NETLINK_DROP_MEMBERSHIP 2
#define NETLINK_PKTINFO 3
#define NETLINK_BROADCAST_ERROR 4
#define NETLINK_NO_ENOBUFS 5
#define NETLINK_RX_RING 6
#define NETLINK_TX_RING 7
#define NETLINK_LISTEN_ALL_NSID 8
#define NETLINK_LIST_MEMBERSHIPS 9
#define NETLINK_CAP_ACK 10
#define NETLINK_EXT_ACK 11
#define NETLINK_GET_STRICT_CHK 12

// address families
#define AF_UNSPEC 0      // unspecified
#define AF_UNIX 1        // local to host (pipes and file-domain)
#define AF_INET 2        // IPv4 Internet protocols
#define AF_AX25 3        // Amateur Radio AX.25
#define AF_IPX 4         // Novell Internet Protocol
#define AF_APPLETALK 5   // AppleTalk
#define AF_NETROM 6      // Amateur Radio NET/ROM
#define AF_BRIDGE 7      // Multiprotocol Label Switching
#define AF_ATMPVC 8      // ATM PVCs
#define AF_X25 9         // ITU X.25 PLP
#define AF_INET6 10      // IPv6 Internet protocols
#define AF_ROSE 11       // Amateur Radio X.25 PLP
#define AF_DECnet 12     // DECnet
#define AF_NETBEUI 13    // NetBIOS
#define AF_SECURITY 14   // Security callback pseudo AF
#define AF_KEY 15        // PF_KEY key management API
#define AF_NETLINK 16    // Kernel user interface device (Netlink)
#define AF_ROUTE 16      // Alias to emulate 4.4BSD
#define AF_LLC 17        // Logical Link Control
#define AF_IB 27         // Native InfiniBand address family
#define AF_MPLS 28       // MPLS
#define AF_CAN 29        // Controller Area Network
#define AF_TIPC 30       // TIPC (Transparent Inter-Process Communication)
#define AF_BLUETOOTH 31  // Bluetooth
#define AF_IUCV 32       // IUCV (Inter-User Communication Vehicle)
#define AF_RXRPC 33      // RxRPC (Remote Execution)
#define AF_ISDN 34       // ISDN
#define AF_PHONET 35     // Phonet (SIMBA)
#define AF_IEEE802154 36 // IEEE 802.15.4
#define AF_CAIF 37       // CAIF (Cellular Abstract Interface)
#define AF_ALG 38        // Algorithm interface
#define AF_NFC 39        // NFC protocol
#define AF_VSOCK 40      // vSockets (VMware)
#define AF_MAX 41        // maximum address family value
// most common _socket types and protocol families
#define IPPROTO_IP 0     // dummy for IP
#define IPPROTO_ICMP 1   // ping
#define IPPROTO_TCP 6    // TCP
#define IPPROTO_UDP 17   // UDP
#define IPPROTO_IPV6 41  // IPv6 header
#define IPPROTO_GRE 47   // tunneling
#define IPPROTO_ESP 50   // IPsec encryption
#define IPPROTO_AH 51    // IPsec authentication
#define IPPROTO_SCTP 132 // stream control (telecoms)
// socket types
#define SOCK_STREAM 1    // stream socket (TCP)
#define SOCK_DGRAM 2     // datagram socket (UDP)
#define SOCK_RAW 3       // raw socket
#define SOCK_RDM 4       // reliably-delivered message
#define SOCK_SEQPACKET 5 // sequenced packet socket
#define SOCK_DCCP 6      // Datagram Congestion Control Protocol
#define SOCK_PACKET 10   // linux specific, for low level access to network devices
// socket flags (can be OR'd into socket type for socket() and accept4())
#define SOCK_NONBLOCK 0x800  // set O_NONBLOCK on the new fd
#define SOCK_CLOEXEC 0x80000 // set O_CLOEXEC  on the new fd

// shutdown how
#define SHUT_RD 0   // shut down reading side of socket
#define SHUT_WR 1   // shut down writing side of socket
#define SHUT_RDWR 2 // shut down both sides of socket

// flags for send() / sendto() / recv() / recvfrom()
#define MSG_OOB 0x1                 // process data out of band
#define MSG_PEEK 0x2                // peek at incoming message
#define MSG_DONTROUTE 0x4           // do not use local routing
#define MSG_CTRUNC 0x8              // control data truncated
#define MSG_PROXY 0x10              // supply or ask second address in sendmsg()/recv
#define MSG_TRUNC 0x20              // message truncated
#define MSG_DONTWAIT 0x40           // non-blocking I/O
#define MSG_EOR 0x80                // end of record
#define MSG_WAITALL 0x100           // wait for full request or error
#define MSG_FIN 0x200               // FIN packet (TCP)
#define MSG_SYN 0x400               // SYN packet (TCP)
#define MSG_CONFIRM 0x800           // confirm path validity (e.g., for mobile)
#define MSG_RST 0x1000              // RST packet (TCP)
#define MSG_ERRQUEUE 0x2000         // fetch message from error queue
#define MSG_NOSIGNAL 0x4000         // do not generate SIGPIPE
#define MSG_MORE 0x8000             // sender will send more
#define MSG_WAITFORONE 0x10000      // wait for at least one packet to return
#define MSG_FASTOPEN 0x20000000     // send data in TCP SYN
#define MSG_CMSG_CLOEXEC 0x40000000 // set close-on-exec flag for file descriptors received via SCM_RIGHTS
#define MSG_ZEROCOPY 0x80000000     // zero-copy receive (if supported by kernel)

// ancillary message types (for sendmsg() and recvmsg())
#define SCM_RIGHTS 0x01      // access rights (e.g., passing file descriptors)
#define SCM_CREDENTIALS 0x02 // process credentials
#define SCM_SECURITY 0x03    // security messages

// socket-level options (for setsockopt() and getsockopt() with SOL_SOCKET)
#define SOL_SOCKET 1    // socket level
#define SO_DEBUG 1      // enable debugging info recording
#define SO_REUSEADDR 2  // allow reuse of local addresses
#define SO_TYPE 3       // get socket type
#define SO_ERROR 4      // get error status and clear
#define SO_DONTROUTE 5  // send without using routing tables
#define SO_BROADCAST 6  // permit sending of broadcast messages
#define SO_SNDBUF 7     // set buffer size for output
#define SO_RCVBUF 8     // set buffer size for input
#define SO_KEEPALIVE 9  // keep connections alive
#define SO_OOBINLINE 10 // leave received OOB data in line
#define SO_LINGER 13    // linger on close (struct linger)
#define SO_REUSEPORT 15 // allow multiple sockets to bind to the same port (since Linux 3.9)
#define SO_PASSCRED 16  // pass credentials
#define SO_PEERCRED 17  // get the credentials of the peer process
#define SO_RCVTIMEO 20  // receive timeout (struct timeval)
#define SO_SNDTIMEO 21  // send timeout    (struct timeval)

// TCP-level options (for setsockopt() and getsockopt() with SOL_TCP)
#define SOL_TCP 6       // TCP socket level (same value as IPPROTO_TCP)
#define TCP_NODELAY 1   // disable Nagle algorithm
#define TCP_MAXSEG 2    // maximum segment size
#define TCP_KEEPIDLE 4  // seconds of idle before sending keepalive probe
#define TCP_KEEPINTVL 5 // interval in seconds between keepalive probes
#define TCP_KEEPCNT 6   // number of keepalive probes before dropping connection
#define TCP_FASTOPEN 23 // enable TCP Fast Open

// IP protocol numbers (for socket() and setsockopt())
#define IPPROTO_IP 0      // dummy for IP
#define IPPROTO_ICMP 1    // ICMP
#define IPPROTO_IGMP 2    // IGMP
#define IPPROTO_IPIP 4    // IPIP tunnels
#define IPPROTO_TCP 6     // TCP
#define IPPROTO_UDP 17    // UDP
#define IPPROTO_IPV6 41   // IPv6 header
#define IPPROTO_GRE 47    // GRE tunnels
#define IPPROTO_ESP 50    // Encap Security Payload
#define IPPROTO_AH 51     // Authentication Header
#define IPPROTO_ICMPV6 58 // ICMPv6
#define IPPROTO_MTP 92    // Multicast Transport Protocol
#define IPPROTO_BEETPH 94 // IP option pseudo header for BEET
#define IPPROTO_ENCAP 98  // Encapsulation Header
#define IPPROTO_RAW 255   // raw IP packets
#define IPPROTO_MAX 256   // maximum valid protocol number

// well-known IPv4 addresses (in network byte order)
#define INADDR_ANY 0x00000000u       // bind to all interfaces
#define INADDR_LOOPBACK 0x7f000001u  // 127.0.0.1
#define INADDR_BROADCAST 0xffffffffu // 255.255.255.255
#define INADDR_NONE 0xffffffffu      // returned on error

// well-known IPv6 addresses (initializer form for struct in6_addr)
#define IN6ADDR_ANY_INIT {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}      // ::
#define IN6ADDR_LOOPBACK_INIT {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}} // ::1

// ioctl commands — network interfaces
#define SIOCGIFNAME 0x8910    // get interface name;              arg = struct ifreq *
#define SIOCGIFCONF 0x8912    // get interface list;              arg = struct ifconf *
#define SIOCGIFFLAGS 0x8913   // get flags;                       arg = struct ifreq *
#define SIOCGIFADDR 0x8915    // get PA address;                  arg = struct ifreq *
#define SIOCGIFDSTADDR 0x8917 // get remote PA address;           arg = struct ifreq *
#define SIOCGIFBRDADDR 0x8919 // get broadcast PA address;        arg = struct ifreq *
#define SIOCGIFNETMASK 0x891b // get network PA mask;             arg = struct ifreq *
#define SIOCGIFMTU 0x8921     // get MTU size;                    arg = struct ifreq *
#define SIOCGIFHWADDR 0x8927  // get hardware address;            arg = struct ifreq *
#define SIOCGIFINDEX 0x8933   // get interface index;             arg = struct ifreq *

// IPv4 address
struct in_addr
{
    in_addr_t s_addr; // address in network byte order
};

// IPv4 socket address
struct sockaddr_in
{
    sa_family_t sin_family;  // AF_INET
    in_port_t sin_port;      // port in network byte order
    struct in_addr sin_addr; // IPv4 address
    char sin_zero[8];        // padding to match size of struct sockaddr
};

// IPv6 address
struct in6_addr
{
    unsigned char s6_addr[16]; // address in network byte order
};

// IPv6 socket address
struct sockaddr_in6
{
    sa_family_t sin6_family;    // AF_INET6
    in_port_t sin6_port;        // port in network byte order
    unsigned int sin6_flowinfo; // IPv6 flow info
    struct in6_addr sin6_addr;  // IPv6 address
    unsigned int sin6_scope_id; // scope id
};

// Unix domain socket address
struct sockaddr_un
{
    sa_family_t sun_family; // AF_UNIX
    char sun_path[108];     // socket file path
};

// linger structure for SO_LINGER socket option
struct linger
{
    int l_onoff;  // 0 = disabled; non-zero = enabled
    int l_linger; // seconds to linger when closing (if l_onoff is non-zero)
};

// ancillary data header (for sendmsg() / recvmsg() control messages)
struct cmsghdr
{
    socklen_t cmsg_len;        // length of cmsghdr plus data in cmsg_data
    int cmsg_level;            // originating protocol (e.g., SOL_SOCKET)
    int cmsg_type;             // protocol-specific type (e.g., SCM_RIGHTS)
    unsigned char cmsg_data[]; // data follows header
};

// poll file descriptor (for poll())
struct pollfd
{
    int fd;        // file descriptor to monitor
    short events;  // events to watch for (bitmask)
    short revents; // events that occurred (set by kernel on return)
};

// epoll event (for epoll_ctl() and epoll_wait())
struct epoll_event
{
    unsigned int events; // epoll event bitmask (EPOLLIN, EPOLLOUT, etc.)
    union
    {
        void *ptr;         // user data pointer
        int fd;            // file descriptor
        unsigned int u32;  // user data 32-bit integer
        unsigned long u64; // user data 64-bit integer
    } data;
};

// poll event flags (events / revents bitmask for struct pollfd)
#define POLLIN 0x001     // data available to read
#define POLLPRI 0x002    // urgent data available to read
#define POLLOUT 0x004    // ready to write
#define POLLERR 0x008    // error condition (revents only)
#define POLLHUP 0x010    // hang up (revents only)
#define POLLNVAL 0x020   // invalid fd (revents only)
#define POLLRDNORM 0x040 // equivalent to POLLIN
#define POLLWRNORM 0x100 // equivalent to POLLOUT

// epoll event flags (for struct epoll_event.events)
#define EPOLLIN 0x001           // fd is ready to read
#define EPOLLPRI 0x002          // urgent data available
#define EPOLLOUT 0x004          // fd is ready to write
#define EPOLLERR 0x008          // error condition
#define EPOLLHUP 0x010          // hang up
#define EPOLLRDHUP 0x2000       // peer closed connection or shut down writing
#define EPOLLONESHOT 0x40000000 // trigger only once then disarm
#define EPOLLET 0x80000000      // edge-triggered mode (default is level-triggered)

// epoll_ctl() operations
#define EPOLL_CTL_ADD 1 // add fd to epoll instance
#define EPOLL_CTL_DEL 2 // remove fd from epoll instance
#define EPOLL_CTX_MOD 3 // modify events watched for fd

// byte order conversion (host <-> network)
unsigned short htons(unsigned short hostshort); // 16-bit host to network byte order
unsigned short ntohs(unsigned short netshort);  // 16-bit network to host byte order
unsigned int htonl(unsigned int hostlong);      // 32-bit host to network byte order
unsigned int ntohl(unsigned int netlong);       // 32-bit network to host byte order
#endif