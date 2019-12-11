COMPILE = gcc 

CFLAGS = -Wall
# -fomit-frame-pointer 
SRC =	es2gears.c \
	eglut.c \
	eglut.h \
	eglutint.h \
	eglut_x11.c

LIBS = -lm -lX11 -lEGL -lGLESv2

TARGET = es2gears


ALL: es2gears

es2gears:
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)
	
clean:
	rm -f *.o $(TARGET)
