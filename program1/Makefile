#
#	Makefile for OpenGL program from Chapter 1 in Red Book
#

CC = g++
CFLAGS = -D_DEBUG
CPPFLAGS = -g

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@  $<

LIBS = -framework OpenGL -framework GLUT -lglew

SRCS = program1.cpp LoadShaders.cpp
OBJS = program1.o LoadShaders.o

program1: $(OBJS) LoadShaders.h
	g++ -g -o program1 $(OBJS) $(LIBS)

clean:
	rm -f program1 *.o
