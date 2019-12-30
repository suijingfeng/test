/*
*	History : 2009.02.02, Created by qizhuang.liu.
*/

#ifndef _TEAPOT_H_
#define _TEAPOT_H_

#include "vdk_sample_common.h"



/*
 * Program & Shader
*/

static const char * FrontBackVSFile = "FrontBack.vert";
static const char * FrontBackFGFile = "FrontBack.frag";
GLuint TeapotProgramFrontBack = 0;

static const char * BackFrontVSFile = "BackFront.vert";
static const char * BackFrontFGFile = "BackFront.frag";
GLuint TeapotProgramBackFront = 0;

static const char * FrontFrontVSFile = "FrontFront.vert";
static const char * FrontFrontFGFile = "FrontFront.frag";
GLuint TeapotProgramFrontFront = 0;

static const char * BackBackVSFile = "BackBack.vert";
static const char * BackBackFGFile = "BackBack.frag";
GLuint TeapotProgramBackBack = 0;

static GLfloat TransparencyBias = 0.72f;
static GLfloat TransparencyScale = -1.0f;

/*
 * Matrix
*/

static float ModelView[16];
static float Projection[16];
static float MVP[16];

/*
*	Texture unit & object of render targets.
*/
GLuint	TeapotTexUnitBackground	= 1;
GLuint	TeapotTexUnitFrontFront	= 2;
GLuint	TeapotTexUnitFrontBack	= 3;
GLuint	TeapotTexUnitBackBack	= 4;
GLuint	TeapotTexUnitBackFront	= 5;




/*
typedef struct
{
	GLuint *		program;
	char *			name;
	int				in_use;
	GLenum			glessl_type;
	GLint			glessl_size;
	GLuint			location;
	GLboolean		transpose;
	const void *	ptr;
}
VDKS_Struct_UnifomInfomation;
*/

#endif
