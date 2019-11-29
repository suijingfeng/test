COMPILE = gcc 

TARGET_NAME = main
INCLUDE_DIR = 
CFLAGS = -Wall
# -fomit-frame-pointer 
SRC = main.c

ALL: triangle1 triangle2


triangle1: 
	$(COMPILE) $(CFLAGS) triangle_glfw.c -lGLESv2 -lglfw -o triangle_glfw

triangle2: 
	$(COMPILE) $(CFLAGS) es2tri.c -lGLESv2 -lm -lX11 -lEGL -o triangle_x11

clean:
	rm -f *.o triangle_glfw triangle_x11
#main:main.o
#	$(COMPILE) -o $(TARGETS) main.o
#.c.o:
#	$(COMPILE) -c $< -o $@
#make
