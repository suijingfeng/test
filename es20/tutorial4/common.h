#ifndef _COMMON_H_
#define _COMMON_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

// Compile a vertex or pixel shader.
int CompileShader(const char * FName, GLuint ShaderNum);

struct CubeTexture
{
	unsigned int * posx;
	unsigned int * negx;
	unsigned int * posy;
	unsigned int * negy;
	unsigned int * posz;
	unsigned int * negz;
	int img_size;
};

// loads a 256x256 ARGB (32bit) format cube map texture dds file.
CubeTexture* LoadCubeDDS(const char * filename);

void DeleteCubeTexture(CubeTexture * cube);

unsigned long GetTimeMillis();

#endif

