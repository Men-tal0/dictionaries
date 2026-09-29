TARGET = servers
SRCS   = qlite3.c servers.c
CC     = gcc
CFLAGS = -Wall -g
# 链接库：SQLite 和 socket 需要 pthread
LIBS   = -lsqlite3
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)
clean:
	 rm -f $(TARGET)
.PHONY: clean
