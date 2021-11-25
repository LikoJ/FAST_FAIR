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