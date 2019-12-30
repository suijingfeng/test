COMPILE = gcc

TARGET_NAME = tutorial8_es20
INCLUDE_DIR = 
CFLAGS = -Wall 
LIBS= -lGLESv2 -lm -lX11 -lEGL
# -fomit-frame-pointer 
SRC = main.c sphere.c teapot.c blend.c vdk_sample_common.c


tutorial8: 
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET_NAME)

clean:
	rm -f *.o $(TARGET_NAME)
