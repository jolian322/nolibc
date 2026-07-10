CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -fno-builtin -nostdlib -g
LDFLAGS = -static -e _start -no-pie

# ── sources ───────────────────────────────────────────────────────────────────
LIB_SRC = libs/jlibc.c
LIB_OBJ = $(LIB_SRC:.c=.o)

UTILS    = io pm sm tty network_client network_server udp_client udp_server httpclient netlink iomux threads futex
THREAD_SRC = utils/threads/pthreads.c utils/threads/futex.c
UTIL_SRC = $(foreach u, $(UTILS), $(wildcard utils/networking/$(u).c utils/$(u).c)) $(THREAD_SRC)
UTIL_OBJ = $(UTIL_SRC:.c=.o)

BINS = $(UTILS) shellexe

# ── targets ───────────────────────────────────────────────────────────────────
all: $(BINS)

# networking utils
network_client network_server udp_client udp_server httpclient netlink: %: utils/networking/%.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# other utils
io pm sm tty iomux: %: utils/%.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
threads: utils/threads/pthreads.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
futex: utils/threads/futex.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
shellexe: shell/main.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# ── compile rules ─────────────────────────────────────────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -Ilibs -c -o $@ $<

# ── phony ─────────────────────────────────────────────────────────────────────
clean:
	rm -f $(LIB_OBJ) $(UTIL_OBJ) shell/main.o $(BINS)

.PHONY: all clean