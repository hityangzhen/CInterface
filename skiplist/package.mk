CC = gcc
CFLAGS += -Wall -Werror -g -DDEBUG
TARGET = simple_skiplist

builddir = /home/yiranyaoqiu/CInterface/debug_build

DEBUG_TARGET = $(builddir)/$(TARGET)

$(DEBUG_TARGET):
	$(CC) $(CFLAGS) -o $@ $(TARGET).c




