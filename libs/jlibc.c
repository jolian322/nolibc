/*********
	  linux syscalls wrapper x86_64 gcc
**********/
#include "jlibc.h"
// static int global_argc;
// static char **global_argv;
// static char **global_envp;

/* testing */
#define SHT_DYNSYM 11

typedef struct
{
	// ELF header structure used at the beginning of ELF files to describe the file format, architecture, entry point, and offsets to program headers and section headers
	unsigned char e_ident[16];
	unsigned short e_type;
	unsigned short e_machine;
	unsigned int e_version;
	unsigned long e_entry;
	unsigned long e_phoff;
	unsigned long e_shoff;
	unsigned int e_flags;
	unsigned short e_ehsize;
	unsigned short e_phentsize;
	unsigned short e_phnum;
	unsigned short e_shentsize;
	unsigned short e_shnum;
	unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
	// program header structure used in ELF files to describe segments that are loaded into memory (e.g., PT_LOAD) or contain dynamic linking information (PT_DYNAMIC)
	unsigned int p_type; // PT_LOAD, PT_DYNAMIC, etc
	unsigned int p_flags;
	unsigned long p_offset; // offset in file
	unsigned long p_vaddr;	// virtual address
	unsigned long p_paddr;
	unsigned long p_filesz;
	unsigned long p_memsz;
	unsigned long p_align;
} Elf64_Phdr;

typedef struct
{
	// section header structure used in ELF files to describe sections like .dynsym, .dynstr, etc.
	unsigned int sh_name;		// index into string table
	unsigned int sh_type;		// SHT_SYMTAB, SHT_DYNSYM, etc
	unsigned long sh_flags;		// SHF_ALLOC, SHF_EXECINSTR, etc
	unsigned long sh_addr;		// virtual address in memory (0 for sections that are not loaded into memory, e.g., .symtab)
	unsigned long sh_offset;	// offset of the section in the file
	unsigned long sh_size;		// size of the section in the file
	unsigned int sh_link;		// for symtab: index of string table section
	unsigned int sh_info;		// for symtab: one greater than the symbol table index of the last local symbol (i.e., the index of the first global symbol)
	unsigned long sh_addralign; // alignment requirement of the section (e.g., 4 for .dynsym, 1 for .dynstr)
	unsigned long sh_entsize;	// size of each entry (for fixed-size sections)
} Elf64_Shdr;

typedef struct
{
	// symbol table entry structure used in .dynsym section of ELF files
	unsigned int st_name;	 // offset into string table
	unsigned char st_info;	 // type + binding
	unsigned char st_other;	 // visibility (usually 0)
	unsigned short st_shndx; // section index
	unsigned long st_value;	 // address (or offset for ET_DYN)
	unsigned long st_size;	 // size of symbol (e.g., function size, or 0 for variables)
} Elf64_Sym;

void walk_vdso(unsigned long base)
{
	// walk the vDSO ELF image at the given base address and print the names and addresses of all symbols in the .dynsym section
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)base;

	// verify magic
	if (ehdr->e_ident[0] != 0x7f ||
		ehdr->e_ident[1] != 'E' ||
		ehdr->e_ident[2] != 'L' ||
		ehdr->e_ident[3] != 'F')
	{
		_write_str(1, "bad magic\n");
		return;
	}
	_write_str(1, "magic: ELF ok\n");

	// section header array starts at base + e_shoff
	Elf64_Shdr *shdrs = (Elf64_Shdr *)(base + ehdr->e_shoff);

	// the section name string table (shstrtab) — for section names
	// we need it to find ".dynsym" / ".dynstr" but we can also just check sh_type
	char *shstrtab = (char *)(base + shdrs[ehdr->e_shstrndx].sh_offset);

	// find DYNSYM section and its linked string table
	Elf64_Shdr *dynsym_shdr = NULL;
	Elf64_Shdr *dynstr_shdr = NULL;

	for (int i = 0; i < ehdr->e_shnum; i++)
	{
		if (shdrs[i].sh_type == SHT_DYNSYM)
		{
			dynsym_shdr = &shdrs[i];
			// sh_link for SHT_DYNSYM is always the index of its string table
			dynstr_shdr = &shdrs[dynsym_shdr->sh_link];
			break;
		}
	}

	if (dynsym_shdr == NULL)
	{
		_write_str(1, "no dynsym found\n");
		return;
	}

	// symbol table and its string table
	Elf64_Sym *syms = (Elf64_Sym *)(base + dynsym_shdr->sh_offset);
	char *dynstr = (char *)(base + dynstr_shdr->sh_offset);
	long sym_count = dynsym_shdr->sh_size / sizeof(Elf64_Sym);

	_write_str(1, "symbols:\n");

	for (long i = 0; i < sym_count; i++)
	{
		Elf64_Sym *sym = &syms[i];

		// skip empty names
		if (sym->st_name == 0)
			continue;

		char *name = dynstr + sym->st_name;

		// st_value for ET_DYN is relative to load base
		unsigned long addr = base + sym->st_value;

		_write_str(1, "  ");
		_write_str(1, name);
		_write_str(1, " -> ");
		_write_ptr(1, (void *)addr);
		_write_char(1, '\n');
	}
}

void _exit(int error_code)
{
	// exit with status code
	__asm__ volatile(
		"syscall;"
		:
		: "a"(SYS_exit),
		  "D"(error_code));
	__builtin_unreachable();
}
ssize_t _read(unsigned int fd, void *buff, size_t count)
{
	// attempts to read from a file descriptor writes to buff returns amount of bytes read
	ssize_t ret;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "D"(fd),		// rdi
		  "S"(buff),	// rsi
		  "d"(count),	// rdx
		  "a"(SYS_read) // rax
		: "rcx", "r11", "memory");
	return ret;
}
ssize_t _write(unsigned int fd, void *buff, size_t count)
{
	// attempts to write to a file descriptor from buff returns amount of bytes wrote
	ssize_t ret;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "D"(fd),		 // rdi
		  "S"(buff),	 // rsi
		  "d"(count),	 // rdx
		  "a"(SYS_write) // rax
		: "rcx", "r11", "memory");
	return ret;
}
int _open(const char *path, int flags, umode_t mode)
{
	// attempts to open a file
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "D"(path),	// rdi
		  "S"(flags),	// rsi
		  "d"(mode),	// rdx
		  "a"(SYS_open) // rax
		: "rcx", "r11", "memory");
	return ret;
}
int _ioctl(int fd, unsigned int cmd, unsigned long arg)
{
	// control device
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "D"(fd),		 // rdi
		  "S"(cmd),		 // rsi
		  "d"(arg),		 // rdx
		  "a"(SYS_ioctl) // rax
		: "rcx", "r11", "memory");
	return ret;
}

int _close(int fd)
{
	// attempts to close an open file descriptor
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "D"(fd),		 // rdi
		  "a"(SYS_close) // rax
		: "rcx", "r11");
	return ret;
}
pid_t _getpid()
{
	// get the process ID
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_getpid)
		: "rcx", "r11");
	return ret;
}
pid_t _getppid()
{
	// get the parent process ID
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_getppid)
		: "rcx", "r11");
	return ret;
}
pid_t _fork()
{
	// creates a child process
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_fork)
		: "rcx", "r11");
	return ret;
}
pid_t _wait4(pid_t pid, int *stat_addr, int options, struct rusage *ru)
{
	//  waits for process to change state (exit or stop) and returns its pid
	/*
	pid value	Meaning
	-1			Wait for any child process
	0			Wait for any child whose process group ID equals the caller's process group ID
	> 0			Wait for the specific child with that process ID
	< -1		Wait for any child whose process group ID equals the absolute value of pid (e.g., pid = -123 waits for any child in process group 123)

	*/
	int ret = -1;
	register long r10 __asm__("r10") = (long)ru;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_wait4), "D"(pid) // rdi
		  ,
		  "S"(stat_addr) // rsi
		  ,
		  "d"(options) // rdx
		  ,
		  "r"(r10) // r10
		: "rcx", "r11", "memory");
	return ret;
}
int _rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, size_t sigsetsize)
{
	// examine and change a signal action
	/*
	signum		The signal number (e.g., SIGINT, SIGTERM, SIGUSR1).
	act			Pointer to a struct sigaction describing the new action (handler, mask, flags). Can be NULL if you only want to query the current action.
	oldact		Pointer to a struct sigaction where the kernel will store the previous action. Can be NULL if you don’t need it.
	sigsetsize	Must be sizeof(sigset_t). The kernel uses this for versioning and safety.
	*/
	int ret = -1;
	register long r10 __asm__("r10") = (long)sigsetsize;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_rt_sigaction) // rax
		  ,
		  "D"(signum) // rdi
		  ,
		  "S"(act) // rsi
		  ,
		  "d"(oldact) // rdx
		  ,
		  "r"(r10) // r10
		: "rcx", "r11", "memory");

	return ret;
}

int _select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{

	int ret = -1;
	register long r10 __asm__("r10") = (long)exceptfds;
	register long r8 __asm__("r8") = (long)timeout;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_select) // rax
		  ,
		  "D"(nfds) // rdi
		  ,
		  "S"(readfds) // rsi
		  ,
		  "d"(writefds) // rdx
		  ,
		  "r"(r10), // r10,
		  "r"(r8)	// r8
		: "rcx", "r11", "memory");

	return ret;
}
__attribute__((naked)) void _rt_sigreturn()
{
	__asm__ volatile(
		"mov $15, %%rax\n" //
		"syscall\n"
		:
		:
		: "rcx", "r11", "memory"

	);
	__builtin_unreachable();
}
/* strings */
size_t _strlen(const char *string)
{
	// returns size of a string
	const char *ptr = string;
	for (; *ptr != '\0'; ptr++)
		;
	return (size_t)(ptr - string);
}
void _write_str(int fd, const char *str)
{
	// write a null-terminated string to file descriptor
	_write(fd, (void *)str, _strlen(str));
}
void _write_int(int fd, int num)
{
	char buffer[12]; // enough for -2147483648 (11 chars + null)
	int i = 0;

	if (num == 0)
	{
		buffer[i++] = '0';
	}
	else
	{
		if (num < 0)
		{
			buffer[i++] = '-';
			// Convert to unsigned to avoid overflow on INT_MIN
			unsigned int unum = -(unsigned int)num;
			int start = i;
			do
			{
				buffer[i++] = '0' + (unum % 10);
				unum /= 10;
			} while (unum > 0);
			// Reverse the digits
			for (int j = start, k = i - 1; j < k; j++, k--)
			{
				char temp = buffer[j];
				buffer[j] = buffer[k];
				buffer[k] = temp;
			}
		}
		else
		{
			// Positive number
			unsigned int unum = num;
			int start = i;
			do
			{
				buffer[i++] = '0' + (unum % 10);
				unum /= 10;
			} while (unum > 0);
			// Reverse digits
			for (int j = start, k = i - 1; j < k; j++, k--)
			{
				char temp = buffer[j];
				buffer[j] = buffer[k];
				buffer[k] = temp;
			}
		}
	}
	buffer[i] = '\0';
	_write(fd, buffer, i);
}

void _write_char(int fd, char c)
{
	// write a single character to file descriptor
	_write(fd, &c, 1);
}
void _write_ptr(int fd, void *ptr)
{
	const char *hex_chars = "0123456789abcdef";
	char buffer[2 + sizeof(void *) * 2 + 1]; // "0x" + hex digits + null
	int i = 0;

	buffer[i++] = '0';
	buffer[i++] = 'x';

	if (ptr == NULL)
	{
		buffer[i++] = '0';
	}
	else
	{
		unsigned long addr = (unsigned long)ptr;
		int start = i;

		do
		{
			buffer[i++] = hex_chars[addr % 16];
			addr /= 16;
		} while (addr > 0);

		// Reverse the hex digits (not the "0x" prefix)
		for (int j = start, k = i - 1; j < k; j++, k--)
		{
			char temp = buffer[j];
			buffer[j] = buffer[k];
			buffer[k] = temp;
		}
	}

	buffer[i] = '\0';
	_write(fd, buffer, i);
}
void _write_byte_hex(int fd, char num)
{
	char buffer[3]; // 2 hex digits + null
	const char *hex_digits = "0123456789abcdef";
	buffer[0] = hex_digits[(num >> 4) & 0x0F]; // high nibble
	buffer[1] = hex_digits[num & 0x0F];		   // low nibble
	buffer[2] = '\0';
	_write(fd, buffer, 2);
}
int _rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset, size_t sigsetsize)
{
	// examine and change blocked signals
	/*
	how				Specifies the behavior of the function. It can be one of the following:
		SIG_BLOCK: 		The set of blocked signals is the union of the current set and the set argument.
		SIG_UNBLOCK: 	The signals in set are removed from the current set of blocked signals.
		SIG_SETMASK: 	The set of blocked signals is replaced by the set argument.
	set
		Pointer to a sigset_t that specifies the signals to be blocked, unblocked, or set as the new mask, depending on the value of how.
	oldset
		Pointer to a sigset_t where the kernel will store the previous set of blocked signals. Can be NULL if you don’t need it.
	sigsetsize
		Must be sizeof(sigset_t). The kernel uses this for versioning and safety.
	*/
	int ret = -1;

	register long r10 __asm__("r10") = (long)sigsetsize;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_rt_sigprocmask), "D"(how) // rdi
		  ,
		  "S"(set) // rsi
		  ,
		  "d"(oldset) // rdx
		  ,
		  "r"(r10) // r10
		: "rcx", "r11", "memory");
	return (int)ret;
}
int _rt_sigsuspend(const sigset_t *mask, size_t sigsetsize)
{
	// temporarily replace the signal mask and suspend execution until a signal is received
	/*
	mask
		Pointer to a sigset_t that specifies the new set of blocked signals while the process is suspended. The kernel will replace the current signal mask with this new mask for the duration of the suspension. When a signal is received, the kernel will restore the original signal mask before invoking the signal handler.
	sigsetsize
		Must be sizeof(sigset_t). The kernel uses this for versioning and safety.
	*/
	int ret = -1;
	register long r10 __asm__("r10") = (long)sigsetsize;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_rt_sigsuspend) // rax
		  ,
		  "D"(mask) // rdi
		  ,
		  "r"(r10) // r10
		: "rcx", "r11", "memory");
	return (int)ret;
}
int _rt_sigpending(sigset_t *set, size_t sigsetsize)
{
	// examine pending signals
	/*
	set
		Pointer to a sigset_t where the kernel will store the set of pending signals for the process. A signal is considered pending if it has been sent to the process but has not yet been delivered (e.g., because it is blocked).
	sigsetsize
		Must be sizeof(sigset_t). The kernel uses this for versioning and safety.
	*/
	int ret = -1;
	register long r10 __asm__("r10") = (long)sigsetsize;

	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_rt_sigpending) // rax
		  ,
		  "D"(set) // rdi
		  ,
		  "r"(r10) // r10
		: "rcx", "r11", "memory");
	return (int)ret;
}

int _kill(pid_t pid, int sig)
{
	/* send a signal to a process */
	int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_kill) // rax
		  ,
		  "D"(pid) // rdi
		  ,
		  "S"(sig) // rsi
		: "rcx", "r11", "memory");
	return (int)ret;
}
unsigned int _alarm(unsigned int seconds)
{
	/* set an alarm clock for delivery of a signal */
	unsigned int ret = -1;
	__asm__ volatile(
		"syscall;"
		: "=a"(ret)
		: "a"(SYS_alarm) // rax
		  ,
		  "D"(seconds) // rdi
		: "rcx", "r11", "memory");
	return ret;
}
long _countSeperators(const char *str, char sep)
{
	long count = 0;
	for (const char *p = str; *p != '\0'; p++)
	{
		if (*p == sep)
		{
			count++;
		}
	}
	return count;
}
long _charsTillSep(const char *str, char sep)
{
	long count = 0;
	for (const char *p = str; *p != '\0'; p++)
	{
		if (*p == sep)
		{
			return count;
		}
		count++;
	}
	return count; // If sep not found, return length of string
}
void _replaceChars(const char *str, char oldChar, char newChar)
{
	for (char *p = (char *)str; *p != '\0'; p++)
	{
		if (*p == oldChar)
		{
			*p = newChar;
		}
	}
}
pid_t _dup(pid_t oldpid)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)	   // Capture the result from rax
		: "a"(SYS_dup) // rax
		  ,
		  "D"(oldpid)			 // rdi
		: "rcx", "r11", "memory" // Clobbers
	);
	return ret;
}
int _dup2(int oldfd, int newfd)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)		// Capture the result from rax
		: "a"(SYS_dup2) // rax
		  ,
		  "D"(oldfd) // rdi
		  ,
		  "S"(newfd)			 // rsi
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
int _setpgid(pid_t pid, pid_t pgid)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)		   // Capture the result from rax
		: "a"(SYS_setpgid) // rax
		  ,
		  "D"(pid) // rdi
		  ,
		  "S"(pgid)				 // rsi
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
void *_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *ret;
	register long r10 __asm__("r10") = (long)flags;
	register long r8 __asm__("r8") = (long)fd;
	register long r9 __asm__("r9") = (long)offset;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_mmap), // rax
		  "D"(addr),	 // rdi
		  "S"(length),	 // rsi
		  "d"(prot),	 // rdx
		  "r"(r10),		 // r10
		  "r"(r8),		 // r8
		  "r"(r9)		 // r9
		: "rcx", "r11", "memory");
	return ret;
}
int _munmap(void *addr, size_t len)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_munmap), // rax
		  "D"(addr),	   // rdi
		  "S"(len)		   // rsi
		: "rcx", "r11", "memory");
	return (int)ret;
}
void *_brk(void *addr)
{
	void *ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_brk), // syscall number (commonly 12 on x86-64)
		  "D"(addr)		// rdi = new break address
		: "rcx", "r11", "memory");
	return ret; // returns the new program break (kernel returns it in rax)
}
long _execve(const char *filename, char *const argv[], char *const envp[])
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)		  // Capture the result from rax
		: "a"(SYS_execve) // rax
		  ,
		  "D"(filename) // rdi
		  ,
		  "S"(argv) // rsi
		  ,
		  "d"(envp)				 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return ret; // This only executes if the syscall fails
}
ssize_t _writev(unsigned long fd, const struct iovec *vec, unsigned long vlen)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_writev) // rax
		  ,
		  "D"(fd) // rdi
		  ,
		  "S"(vec) // rsi
		  ,
		  "d"(vlen)				 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (ssize_t)ret;
}
int _socket(int domain, int type, int protocol)
{
	// create an endpoint for communication and return a file descriptor for the socket (domain specifies the communication domain, e.g., AF_INET for IPv4, AF_UNIX for local sockets; type specifies the socket type, e.g., SOCK_STREAM for TCP, SOCK_DGRAM for UDP; protocol specifies a particular protocol to be used with the socket, or 0 to select the default protocol for the given domain and type)
	// protocol is usually 0 to select the default protocol for the given domain and type (e.g., IPPROTO_TCP for AF_INET + SOCK_STREAM, IPPROTO_UDP for AF_INET + SOCK_DGRAM), but can be set to a specific protocol number if needed
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_socket) // rax
		  ,
		  "D"(domain) // rdi
		  ,
		  "S"(type) // rsi
		  ,
		  "d"(protocol)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	// bind a socket to an address (e.g., for TCP servers to specify the port to listen on) or for UDP to specify the source port (can also be used to bind to a specific network interface)
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_bind) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(addr) // rsi
		  ,
		  "d"(addrlen)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}

int _listen(int sockfd, int backlog)
{
	// marks a socket as passive and ready to accept incoming connections, with a specified backlog for the connection queue (backlog is the maximum number of pending connections that can be queued up before the kernel starts rejecting new connection attempts)
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_listen) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(backlog)			 // rsi
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
int _accept4(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen, int flags)
{
	// accept a connection on a socket and return a new file descriptor for the connection (flags can be used to set O_NONBLOCK and/or O_CLOEXEC on the new fd)
	long ret;
	register long r10 __asm__("r10") = (long)flags;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_accept4) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(addr) // rsi
		  ,
		  "d"(addrlen) // rdx
		  ,
		  "r"(r10)				 // r10 = flags
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
int _connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	// initiate a connection on a socket (for TCP) or set default peer address (for UDP)
	// returns 0 on success, or -1 on error (e.g., if the connection is refused or the address is unreachable). For TCP sockets, this will initiate the three-way handshake to establish a connection with the server. For UDP sockets, this simply sets the default destination address for send() and allows recv() to filter incoming packets from that peer.
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_connect) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(addr) // rsi
		  ,
		  "d"(addrlen)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
ssize_t _sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	// send a message on a socket to a specific destination (used for UDP) or to the connected peer (for TCP, dest_addr and addrlen are ignored)
	long ret;
	register long r10 __asm__("r10") = (long)flags;
	register long r8 __asm__("r8") = (long)dest_addr;
	register long r9 __asm__("r9") = (long)addrlen;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_sendto) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(buf) // rsi
		  ,
		  "d"(len) // rdx
		  ,
		  "r"(r10) // r10 = flags
		  ,
		  "r"(r8) // r8 = dest_addr
		  ,
		  "r"(r9)				 // r9 = addrlen
		: "rcx", "r11", "memory" // Clobbers
	);
	return (ssize_t)ret;
}
ssize_t _sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
	// send a message on a socket using a struct msghdr to specify multiple buffers (iovec), destination address, and flags (similar to sendto but more flexible for scatter-gather I/O and ancillary data)
	long ret;
	register long r10 __asm__("r10") = (long)flags;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_sendmsg) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(msg) // rsi
		  ,
		  "d"(flags)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (ssize_t)ret;
}
int _mprotect(void *addr, size_t len, int prot)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_mprotect) // rax
		  ,
		  "D"(addr) // rdi
		  ,
		  "S"(len) // rsi
		  ,
		  "d"(prot)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
}
ssize_t _recvfrom(int sockfd, void *buf, size_t size, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	// receive a message from a socket, potentially capturing the source address (for datagram sockets) or from the connected peer (for stream sockets). flags can be used to specify special behavior (e.g., MSG_DONTWAIT for non-blocking receive, MSG_PEEK to peek at the incoming message without removing it from the queue, etc.). src_addr and addrlen are used for datagram sockets to store the source address of the received message; for stream sockets, they are ignored.
	long ret;
	register long r8 __asm__("r8") = (long)src_addr;
	register long r9 __asm__("r9") = (long)addrlen;
	register long r10 __asm__("r10") = (long)flags;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_recvfrom) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(buf) // rsi
		  ,
		  "d"(size) // rdx
		  ,
		  "r"(r10) // r10 = flags
		  ,
		  "r"(r8) // r8 = src_addr
		  ,
		  "r"(r9)				 // r9 = addrlen
		: "rcx", "r11", "memory" // Clobbers
	);
	return (ssize_t)ret;
}
__attribute__((naked))
pid_t
_clone(
	unsigned long flags, // rdi
	unsigned long *stack, // rsi
	int *parent_tid,	 // rdx
	int *child_tid,		 // rcx  must move to r10
	unsigned long tls)	 // r8
{
	__asm__(
		"mov %rcx, %r10\n" // child_tid: C passes in rcx, syscall wants r10
		"mov $56, %eax\n"  // SYS_clone = 56
		"syscall\n"
		"test %rax, %rax\n" // rax=0 means child, rax=tid means parent
		"jnz 1f\n"			// parent skip to normal return
		// child path:
		"xor %rbp, %rbp\n" // clear frame pointer (no parent frame)
		"ret\n"			   // pop thread_fn from new stack  jump there
		"1:\n"
		"ret\n" // parent: return tid in rax
	);
}
extern long _futex(uint32_t *uaddr, int op, uint32_t val, const struct timespec *utime, uint32_t *uaddr2, int val3)
{
	long ret;
	register long r9 __asm__("r9") = (long)val3;
	register long r8 __asm__("r8") = (long)uaddr2;
	register long r10 __asm__("r10") = (long)utime;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_futex) // rax
		  ,
		  "D"(uaddr) // rdi
		  ,
		  "S"(op) // rsi
		  ,
		  "d"(val) // rdx
		  ,
		  "r"(r10) // r10 = flags
		  ,
		  "r"(r8), // r8 = src_addr
		  "r"(r9)
		: "rcx", "r11", "memory" // Clobbers
	);
	return ret;
}
int _tgkill(pid_t tgid, pid_t tid, int sig)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_tgkill) // rax
		  ,
		  "D"(tgid) // rdi
		  ,
		  "S"(tid) // rsi
		  ,
		  "d"(sig) // rdx

		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
};
pid_t _set_tid_address(int *tidptr)
{
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_set_tid_address) // rax
		  ,
		  "D"(tidptr) // rdi

		: "rcx", "r11", "memory" // Clobbers
	);
	return (pid_t)ret;
};
ssize_t _recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	long ret;

	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_recvmsg) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(msg) // rsi
		  ,
		  "d"(flags)			 // rdx
		: "rcx", "r11", "memory" // Clobbers
	);
	return (ssize_t)ret;
}
int _shutdown(int sockfd, int how)
{
	// shut down part of a full-duplex connection (e.g., for TCP sockets, how can be SHUT_RD to disallow further receives, SHUT_WR to disallow further sends, or SHUT_RDWR to disallow both)
	long ret;
	__asm__ volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYS_shutdown) // rax
		  ,
		  "D"(sockfd) // rdi
		  ,
		  "S"(how)				 // rsi
		: "rcx", "r11", "memory" // Clobbers
	);
	return (int)ret;
};
int _strncmp(const char *s1, const char *s2, size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		if (s1[i] != s2[i])
		{
			return (unsigned char)s1[i] - (unsigned char)s2[i];
		}
		if (s1[i] == '\0')
		{
			return 0;
		}
	}
	return 0;
}
char *_getENV(const char *name, char **envp)
{
	size_t name_len = _strlen(name);
	for (char **env = envp; *env != NULL; env++)
	{
		if (_countSeperators(*env, '=') == 1 && _charsTillSep(*env, '=') == name_len)
		{
			// potential match
			if (_strncmp(*env, name, name_len) == 0)
			{
				// found match
				return *env + name_len + 1; // return pointer to value (after '=')
			}
		}
	}
	return NULL; // not found
}
int strcmp(const char *a, const char *b)
{
	while (*a && *a == *b)
	{
		a++;
		b++;
	}
	return (unsigned char)*a - (unsigned char)*b;
}
void *memset(void *s, int c, size_t n)
{
	unsigned char *p = s;
	while (n--)
		*p++ = (unsigned char)c;
	return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;
	while (n--)
		*d++ = *s++;
	return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;
	if (d < s)
	{
		while (n--)
			*d++ = *s++;
	}
	else
	{
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	}
	return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
	const unsigned char *p = a, *q = b;
	while (n--)
	{
		if (*p != *q)
			return *p - *q;
		p++;
		q++;
	}
	return 0;
}

char *strncat(char *dst, const char *src, size_t n)
{
	char *d = dst;
	while (*d)
		d++;
	while (n-- && *src)
		*d++ = *src++;
	*d = '\0';
	return dst;
}
void _start_c(long argc, char **argv, char **envp)
{
	// global_argc = (int)argc;
	// global_argv = argv;
	// global_envp = envp;
	// unsigned long *auxv = (unsigned long *)(envp);
	// while (*auxv != NULL) auxv++;
	// auxv++;

	// unsigned long vdso_addr = 0;

	// while (auxv[0] != 0)
	// {
	//     if (auxv[0] == 33) // AT_SYSINFO_EHDR
	//         vdso_addr = auxv[1];
	//     auxv += 2;
	// }

	// if (vdso_addr == 0)
	// {
	//     _write_str(1, "no vdso\n");
	//     _exit(1);
	// }

	// walk_vdso(vdso_addr);
	int ret = main((int)argc, argv, envp);
	_exit(ret);
}

// naked: no prologue/epilogue, rsp is exactly as the kernel left it
__attribute__((naked)) void _start(void)
{
	__asm__ volatile(
		"mov (%rsp), %rdi\n"		  // argc  (dereference rsp)
		"lea 8(%rsp), %rsi\n"		  // argv = rsp + 8
		"lea 16(%rsp,%rdi,8), %rdx\n" // envp = rsp + 16 + argc*8
		"call _start_c\n");
	__builtin_unreachable();
}

unsigned short htons(unsigned short hostshort)
{
	// check if machine is already big endian
	// if so return unchanged
	unsigned short test = 0x0001;
	if (*(unsigned char *)&test == 0x00)
	{
		return hostshort; // already big endian
	}
	// swap bytes
	return ((hostshort & 0xFF00) >> 8) |
		   ((hostshort & 0x00FF) << 8);
}

unsigned short ntohs(unsigned short netshort)
{
	// exact same operation as htons
	// swapping is its own inverse
	return htons(netshort);
}

unsigned int htonl(unsigned int hostlong)
{
	unsigned short test = 0x0001;
	if (*(unsigned char *)&test == 0x00)
	{
		return hostlong; // already big endian
	}
	// swap all 4 bytes
	return ((hostlong & 0xFF000000) >> 24) |
		   ((hostlong & 0x00FF0000) >> 8) |
		   ((hostlong & 0x0000FF00) << 8) |
		   ((hostlong & 0x000000FF) << 24);
}

unsigned int ntohl(unsigned int netlong)
{
	// same as htonl
	return htonl(netlong);
}