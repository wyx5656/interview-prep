SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
BIN:=main
CC:=gcc
FLAGS:=-g -w -lcrypt -lssl -lcrypto -ll8w8jwt -lmbedtls -lmbedx509 -lmbedcrypto

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS)
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) $(BIN)
