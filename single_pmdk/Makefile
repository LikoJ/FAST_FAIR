# .PHONY: all clean
# .DEFAULT_GOAL := all

# LIBS=-lrt -lm -lpmemobj
# INCLUDES=-I ./include /home/skian/.local/bin/include
# CFLAGS=-O3 -std=c++11 -g

# output = btree 

# all: main

# main: src/test.cpp
# 	g++ $(CFLAGS) -o btree src/test.cpp $(LIBS)

# clean: 
# 	rm $(output)
CC = g++
AR = ar crv

FLAGS = -O3 -std=c++11 -g

LIBS = -lrt -lm -lpmemobj

OBJECT := btree.o \

TARGET = libfastfairtree.a

$(TARGET) : $(OBJECT)
	$(AR) $@ $^

.cc.o :
	$(CC) -c $(FLAGS) -o $@ $< $(LIBS)

clean :
	rm $(TARGET) *.o