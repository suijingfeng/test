COMPILE = g++ 

TARGET_NAME = tutorial3_es20
INCLUDE_DIR = 
CFLAGS = -Wall 
LIBS= -lGLESv2 -lm -lX11 -lEGL
# -fomit-frame-pointer 
SRC = tutorial3.cpp common.cpp commonMath.cpp


tutorial3: 
	$(COMPILE) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET_NAME)

clean:
	rm -f *.o $(TARGET_NAME)
	
#main:main.o
#	$(COMPILE) -o $(TARGETS) main.o
#.c.o:
#	$(COMPILE) -c $< -o $@
#make
