Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))
BIN:=CloudiskServer
CC := gcc
CFLAGS = -w -g -lmysqlclient -lcrypt -ll8w8jwt -lmbedtls -lmbedx509 -lmbedcrypto
ALL: $(Outs) 
	$(CC) -o $(BIN) $^ $(LIBS) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean rebuild ALL

clean:
	$(RM) $(Outs) $(BIN)
rebuild: clean ALL
