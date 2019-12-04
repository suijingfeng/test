COMPILE = gcc 

INCLUDE_DIR = 
CFLAGS = -Wall
# -fomit-frame-pointer 
SRC = main.c

LIBS = -lm -lX11 -lEGL -lGLESv2

TARGETS = triangle_glfw triangle_x11 pixmap


ALL: triangle1 triangle2 pixmap


triangle1: 
	$(COMPILE) $(CFLAGS) triangle_glfw.c -lGLESv2 -lglfw -o triangle_glfw

triangle2: 
	$(COMPILE) $(CFLAGS) es2tri.c $(LIBS) -o triangle_x11

pixmap:
	$(COMPILE) $(CFLAGS) egl_pixmap.c $(LIBS) -o pixmap

clean:
	rm -f *.o $(TARGETS)

#main:main.o
#	$(COMPILE) -o $(TARGETS) main.o
#.c.o:
#	$(COMPILE) -c $< -o $@
#make
