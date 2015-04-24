#
#	Makefile for OpenGL program from Chapter 1 in Red Book
#
#IS_LINUX = 1

CC = g++
CFLAGS = -D_DEBUG
CPPFLAGS = -g

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@  $<


ifdef IS_LINUX
LIBS = -lGL -lGLU -lGLEW -lglut -lSOIL
else
LIBS = -framework OpenGL -framework GLUT -lglew -lSOIL -framework CoreFoundation
endif

SRCS = viewer.cpp LoadShaders.cpp
OBJS = viewer.o LoadShaders.o

viewer: $(OBJS) LoadShaders.h
	g++ -g -o viewer $(OBJS) $(LIBS)
LoadShaders.h:

viewer.o: viewer.cpp LoadShaders.h
	${CC} ${CFLAGS} -c viewer.cpp

LoadShaders.o: LoadShaders.cpp
	${CC} ${CFLAGS} -c LoadShaders.cpp

clean:
	rm -f viewer *.o
