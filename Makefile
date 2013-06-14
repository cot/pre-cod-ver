MAIN = NBody
OUTPUT = n-body-sim
SRCPATH = ./src/
SHAREDPATH = ./libs/src/
SHAREDINCPATH = ./libs/include/
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL -I$(SHAREDINCPATH)  -I$(SHAREDINCPATH)GL

#CC = clang++
CC= mpic++
CFLAGS = $(COMPILERFLAGS) -g $(INCDIRS) -lpthread
LIBS = -lX11 -lglut -lGL -lGLU -lm

debug: CC+= -g -pg
debug: all

all : $(SpawnProg) $(MAIN)

$(MAIN).o : $(SRCPATH)$(MAIN).cpp
glew.o    : $(SHAREDPATH)glew.c
GLTools.o    : $(SHAREDPATH)GLTools.cpp
GLBatch.o    : $(SHAREDPATH)GLBatch.cpp
GLTriangleBatch.o    : $(SHAREDPATH)GLTriangleBatch.cpp
GLShaderManager.o    : $(SHAREDPATH)GLShaderManager.cpp
math3d.o    : $(SHAREDPATH)math3d.cpp

$(SpawnProg): SpawnProg.c
	$(CC) $(CFLAGS) -o SpawnProg $(LIBDIRS) $(INCDIRS) SpawnProg.c $(LIBS)

$(MAIN) : $(MAIN).o glew.o SpawnProg
	$(CC) $(CFLAGS) -o $(OUTPUT) $(LIBDIRS) $(SRCPATH)$(MAIN).cpp  $(SHAREDPATH)glew.c $(SHAREDPATH)GLTools.cpp $(SHAREDPATH)GLBatch.cpp $(SHAREDPATH)GLTriangleBatch.cpp $(SHAREDPATH)GLShaderManager.cpp $(SHAREDPATH)math3d.cpp $(LIBS)

clean:
	rm -f *.o _coord*
