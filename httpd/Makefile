CC = gcc

CFLAGS =-pedantic -Werror -Wall -Wextra -Wvla

SRCS = src/utils/string/string.c src/config/config.c src/server/server.c src/main.c src/daemon/daemon.c

OBJS = $(SRCS:.c=.o)



httpd: $(OBJS)
	$(CC) -o $@ $^
	
debug: $(OBJS)
	$(CC) $(CFLAGS) -g -fsanitize=address -o httpd $(SRCS)

check:
	sh tests/test.sh

clean:
	$(RM) $(OBJS) httpd
