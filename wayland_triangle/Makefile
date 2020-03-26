COMPILE = gcc 

CFLAGS = -Wall
# -fomit-frame-pointer 
SRC = simple-egl.c xdg-shell-protocol.c

LIBS = -lm -lwayland-client -lwayland-egl -lwayland-cursor -lEGL -lGLESv2

TARGET = triangle_wl_egl


#libeglut_wayland_la_CFLAGS = $(WAYLAND_CFLAGS) $(EGL_CFLAGS)
#libeglut_wayland_la_LIBADD = $(WAYLAND_LIBS) $(EGL_LIBS)

ALL: clean triangle_wl_egl

triangle_wl_egl:
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)
	
clean:
	rm -f *.o $(TARGET)
