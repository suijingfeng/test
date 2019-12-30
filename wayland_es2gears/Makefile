COMPILE = gcc 

CFLAGS = -Wall -O3
# -fomit-frame-pointer 
SRC =	es2gears.c egl_wayland.c mat_math.c

LIBS = -lm -lwayland-client -lwayland-egl -lEGL -lGLESv2

TARGET = es2gears


#libeglut_wayland_la_CFLAGS = $(WAYLAND_CFLAGS) $(EGL_CFLAGS)
#libeglut_wayland_la_LIBADD = $(WAYLAND_LIBS) $(EGL_LIBS)

ALL: clean es2gears

es2gears:
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)
	
clean:
	rm -f *.o $(TARGET)
