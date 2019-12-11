COMPILE = gcc 

CFLAGS = -Wall
# -fomit-frame-pointer 
SRC =	es2gears.c \
	eglut.c \
	eglut.h \
	eglutint.h \
	eglut_wayland.c

LIBS = -lm -lwayland-client -lwayland-egl -lEGL -lGLESv2

TARGET = es2gears


#libeglut_wayland_la_CFLAGS = $(WAYLAND_CFLAGS) $(EGL_CFLAGS)
#libeglut_wayland_la_LIBADD = $(WAYLAND_LIBS) $(EGL_LIBS)

ALL: es2gears

es2gears:
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)
	
clean:
	rm -f *.o $(TARGET)
