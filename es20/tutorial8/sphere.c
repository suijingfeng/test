
/*
*	History : 2009.02.02, Created by qizhuang.liu.
*/

#include "vdk_sample_common.h"

/*
 * Model
*/
static const char * PositionFile = "sphere_position.txt";
static float * pPositionFloats = NULL;

static GLuint PositionGLBuffer = 0;
static GLuint PositionVertexAttributeArrayIndex = 0;

static const char * TriangleFile = "sphere_triangles.txt";
static unsigned short *	TriangleData = NULL;
static size_t TriangleVertexCount = 0;
static GLuint TriangleGLBuffer = 0;



/*
 * Shaders
*/

static const char * VSFile = "Env.vert";
static const char * FGFile = "Env.frag";

static GLuint Program = 0;

static float MVP[16];

/*
*	Texture
*/

static const char* PositiveX = "Mars_POSITIVE_X_MIPMAP_LEVEL_0.bmp";
static const char* NegativeX = "Mars_NEGATIVE_X_MIPMAP_LEVEL_0.bmp";

static const char* PositiveY = "Mars_POSITIVE_Y_MIPMAP_LEVEL_0.bmp";
static const char* NegativeY = "Mars_NEGATIVE_Y_MIPMAP_LEVEL_0.bmp";

/*
*	This texture is from dds, which is LHS, that the Z axis is opposite.
*/
static const char* PositiveZ = "Mars_NEGATIVE_Z_MIPMAP_LEVEL_0.bmp";
static const char* NegativeZ = "Mars_POSITIVE_Z_MIPMAP_LEVEL_0.bmp";

static GLuint TextueObject = 0;
static GLuint TexUnit = 0;


VDKS_BOOL SphereInitTexture()
{
	int width = 0;
	int height = 0;

	unsigned char * rgba_pos_x = NULL;
	unsigned char * rgba_neg_x = NULL;

	unsigned char * rgba_pos_y = NULL;
	unsigned char * rgba_neg_y = NULL;

	unsigned char * rgba_pos_z = NULL;
	unsigned char * rgba_neg_z = NULL;

	char szCurDir[MAX_PATH + 1];
	char szTempFile[MAX_PATH + 1];

        VDKS_Func_GetCurrentDir(szCurDir);
	
        glActiveTexture(GL_TEXTURE0 + TexUnit);

	glGenTextures(1, &TextueObject);

	glBindTexture(GL_TEXTURE_CUBE_MAP, TextueObject);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositiveX);
	rgba_pos_x = VDKS_Func_ReadBmp(szTempFile/*PositiveX*/, &width, &height); assert(rgba_pos_x);
	rgba_pos_x = VDKS_Func_TransformBmp(rgba_pos_x, width, height, VDKS_TRUE, VDKS_TRUE);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, NegativeX);
	rgba_neg_x = VDKS_Func_ReadBmp(szTempFile/*NegativeX*/, &width, &height); assert(rgba_neg_x);
	rgba_neg_x = VDKS_Func_TransformBmp(rgba_neg_x, width, height, VDKS_TRUE, VDKS_TRUE);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositiveY);
	rgba_pos_y = VDKS_Func_ReadBmp(szTempFile/*PositiveY*/, &width, &height); assert(rgba_pos_y);
	//rgba_pos_y = VDKS_Func_TransformBmp(rgba_pos_y, width, height, VDKS_TRUE, VDKS_TRUE);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, NegativeY);
	rgba_neg_y = VDKS_Func_ReadBmp(szTempFile/*NegativeY*/, &width, &height); assert(rgba_neg_y);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositiveZ);
	rgba_pos_z = VDKS_Func_ReadBmp(szTempFile/*PositiveZ*/, &width, &height); assert(rgba_pos_z);
	rgba_pos_z = VDKS_Func_TransformBmp(rgba_pos_z, width, height, VDKS_TRUE, VDKS_TRUE);

	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, NegativeZ);
	rgba_neg_z = VDKS_Func_ReadBmp(szTempFile/*NegativeZ*/, &width, &height); assert(rgba_neg_z);
	rgba_neg_z = VDKS_Func_TransformBmp(rgba_neg_z, width, height, VDKS_TRUE, VDKS_TRUE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pos_x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_neg_x);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pos_y);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_neg_y);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pos_z);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_neg_z);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	return VDKS_TRUE;
}


VDKS_BOOL UpdateStates()
{
	/*
	*	Attributes
	*/
	do
	{
		int	index;
		GLint	nr_act_att = 0;
		GLsizei bufsize = 128;
		GLint	location = 0;

                // The command
                //
                // void GetShaderiv( uint shader, enum pname, int *params );
                //
                // returns properties of the shader object named shader in params. 
                // The parameter value to return is specified by pname.
                //
                // If pname is SHADER_TYPE , VERTEX_SHADER is returned if shader 
                // is a vertex shader object, and FRAGMENT_SHADER is returned if 
                // shader is a fragment shader object. 
                //
                // If pname is DELETE_STATUS , TRUE is returned if the shader has 
                // been flagged for deletion and FALSE is returned otherwise. 
                //
                // If pname is COMPILE_STATUS , TRUE is returned if the shader was
                // last compiled sucessfully, and FALSE is returned otherwise. 
                //
                // If pname is INFO_LOG_LENGTH , the length of the info log,
                // including a null terminator, is returned. If there is no info log, 
                // zero is returned. 
                //
                // If pname is SHADER_SOURCE_LENGTH , the length of the concatenation 
                // of the source strings making up the shader source, including a null 
                // terminator, is returned. If no source has been defined, zero is 
                // returned.
                //
                // 
		glGetProgramiv(Program, GL_ACTIVE_ATTRIBUTES, &nr_act_att);

		char * name = (char *) malloc(bufsize);
		if (NULL == name) {
			fprintf(stderr, "out-of-memory \n");
			return VDKS_FALSE;
		}

		for (index = 0; index < nr_act_att; ++index)
		{
			GLsizei length;
			GLenum	type;
			GLint	size = 0;
			glGetActiveAttrib(Program, index, bufsize, &length, &size, &type, name);

			if (!strcmp(name, "position"))
			{
				assert(size == 1);
				assert(type == GL_FLOAT_VEC3);

				location = glGetAttribLocation(Program, name);

				assert( location == PositionVertexAttributeArrayIndex );

				glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
				glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(location);
			}
			else
			{
				fprintf(stderr, "The attribute name is not in handling.\n");
				free(name);
				return VDKS_FALSE;
			}
		}

		free(name);
	}
	while(0);

	/*
	 * Uniforms
	*/
	do
	{
		char * name = malloc(128);
		if (NULL == name) {
			printf("out-of-memory");
			return VDKS_FALSE;
		}

		GLint nr_act_uni = 0;
		glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &nr_act_uni);
	        
                // 2
                // printf("ACTIVE UNIFORMS: %d \n", nr_act_uni);

		glUseProgram(Program);
                
		GLint i;
		for(i = 0; i < nr_act_uni; ++i)
		{
			GLsizei length;
			GLenum type;
			GLint size = 0;
			glGetActiveUniform(Program, i, 128, &length, &size, &type, name);

			if (!strcmp(name, "mvp"))
			{
				assert(size == 1);
				assert(type == GL_FLOAT_MAT4);

				GLint location = glGetUniformLocation(Program, "mvp");

				glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
			}
			else if (!strcmp(name, "env"))
			{
				assert(size == 1);
				assert(type == GL_SAMPLER_CUBE);

				GLint location = glGetUniformLocation(Program, "env");

				glUniform1i(location, TexUnit);
			}
			else
			{
				fprintf(stderr, "The uniform name is not in handling.\n");
				return VDKS_FALSE;
			}
		}

		free(name);

		glUseProgram(0);
	}
	while(0);

	return VDKS_TRUE;
}


VDKS_BOOL SphereInit()
{
	char szCurDir[MAX_PATH + 1];
	char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
	
        VDKS_Func_GetCurrentDir(szCurDir);

	strcpy(szVSFile, szCurDir);
	strcpy(szFGFile, szCurDir);
	strcat(szVSFile, VSFile);
	strcat(szFGFile, FGFile);

	/*
	 * Model Positon
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositionFile);

        size_t PositionFloatsCount = 0;

	if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &pPositionFloats, (SIZE_T *)&PositionFloatsCount))
	{
		fprintf(stderr, "Init : Failed to read position data file. \n");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &PositionGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, PositionFloatsCount * sizeof(float), pPositionFloats, GL_STATIC_DRAW);

	/*
	 * Model Postion Center & Radius
	*/
        float CenterX = 0.0f;
        float CenterY = 0.0f;
        float CenterZ = 0.0f;
        float Radius = 0.0f;
        
	VDKS_Func_ModelCenterRadius(pPositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);

	VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",
                CenterX, CenterY, CenterZ, Radius);

	/*
	 * Matrix
	*/
	do
	{
		float view[3] = {0.0f, 0.0f, 0.1f};
		float focus[3] = {0.0f, 0.0f, 0.0f};
		float up[3] = {0.0f, 1.0f, 0.0f};

		/*
		 * Assume
		*/

                float ModelView[16];
                float Projection[16];


		VDKS_Func_LoadIdentity(ModelView);
		VDKS_Func_LoadIdentity(Projection);
		VDKS_Func_LoadIdentity(MVP);

                VDKS_Func_Matrix_Rotate(ModelView, 135.0, 0.0, 1.0, 0.0);
		// VDKS_Func_Matrix_Rotate(ModelView, 180.0, 0.0, 1.0, 0.0);
		// VDKS_Func_Matrix_Rotate(ModelView, 90.0, 1.0, 0.0, 0.0);

		VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

		VDKS_Func_Matrix_Ortho(
			-1.0f * Radius, Radius,	/*left, right*/
			-1.0f * Radius, Radius,	/*bottom, up*/
			1.0, 400.0,				/*near, far*/
			Projection);

		VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

		//VDKS_Func_Translate(ModelView, -1.0 * CenterX, -1.0 * CenterY, -1.0 * CenterZ);
	}
	while(0);

	/*
	 * Model Triangle
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TriangleFile);
	if (VDKS_TRUE != VDKS_ReadTriangle(szTempFile, &TriangleData, (SIZE_T *)&TriangleVertexCount))
	{
		printf("Init : Failed to read triangle data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &TriangleGLBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TriangleVertexCount * sizeof(unsigned short), TriangleData, GL_STATIC_DRAW);

	/*
	*	Texture
	*/

	SphereInitTexture();

	/*
	* Shaders
	*/
	Program = glCreateProgram();

        printf("Sphere Program: %u. \n", Program );
	/*
	* Manage Location
	*/

	VDKS_Func_LocationManagerInit();

	PositionVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	if (PositionGLBuffer == 0)
	{
		printf("Failed to alloc location.");

		return VDKS_FALSE;
	}

	glBindAttribLocation(Program, PositionVertexAttributeArrayIndex, "position");

	Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);

	if (Program == 0)
	{
		printf("Failed to create a new program.");
		return VDKS_FALSE;
	}

	if (!UpdateStates()) {
		return VDKS_FALSE;
	}

	/*
	 * Miscellaneous GL States
	*/

	VDKS_Macro_CheckGLError();

	return VDKS_TRUE;
}


void SpherePass(void)
{
	if (!UpdateStates())
            return;

	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(Program);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

	glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

	VDKS_Macro_CheckGLError();

	glUseProgram(0);
}

