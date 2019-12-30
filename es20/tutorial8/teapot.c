/*
*	History : 2009.02.02, Created by qizhuang.liu.
*/

#include "teapot.h"

static GLuint TeapotGLBuffer = 0;

/*
 * Model
*/
static const char * PositionFile = "teapot_position.txt";
static float * PositionFloats = NULL;
static size_t PositionFloatsCount = 0;
static GLuint PositionGLBuffer = 0;

static GLuint PositionVertexAttributeArrayIndex = 0;

static const char * TextureCoord_0_File = "teapot_texcoord_0.txt";
static float * TextureCoord_0_Floats = NULL;
static size_t TextureCoord_0_FloatsCount = 0;
static GLuint TextureCoord_0_GLBuffer = 0;

static GLuint TextureCoord_0_VertexAttributeArrayIndex = 0;

static const char * TriangleFile = "teapot_triangles.txt";

static size_t TeapotVertexCount = 0;


GLuint	TeapotTexObjBackground	= 0;
GLuint	TeapotTexObjFrontFront	= 0;
GLuint	TeapotTexObjFrontBack	= 0;
GLuint	TeapotTexObjBackFront	= 0;
GLuint	TeapotTexObjBackBack	= 0;

static GLuint TexObjBase = 0;
static GLuint TexUnitBase = 6;

static VDKS_Struct_UnifomInfomation UnifomsInfomation[] =
{
	/* FrontFront */
	{
		&TeapotProgramFrontFront,
		"transparency_bias",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyBias
	},
	{
		&TeapotProgramFrontFront,
		"transparency_scale",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyScale
	},
	{
		&TeapotProgramFrontFront,
		"mvp",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT_MAT4,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		MVP
	},
	{
		&TeapotProgramFrontFront,
		"base",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_SAMPLER_2D,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TexUnitBase
	},
	/* FrontBack */
	{
		&TeapotProgramFrontBack,
		"transparency_bias",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyBias
	},
	{
		&TeapotProgramFrontBack,
		"transparency_scale",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyScale
	},
	{
		&TeapotProgramFrontBack,
		"mvp",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT_MAT4,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		MVP
	},
	{
		&TeapotProgramFrontBack,
		"base",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_SAMPLER_2D,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TexUnitBase
	},
	/* BackBack */
	{
		&TeapotProgramBackBack,
		"transparency_bias",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyBias
	},
	{
		&TeapotProgramBackBack,
		"transparency_scale",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyScale
	},
	{
		&TeapotProgramBackBack,
		"mvp",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT_MAT4,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		MVP
	},
	{
		&TeapotProgramBackBack,
		"base",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_SAMPLER_2D,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TexUnitBase
	},
	/* BackFront */
	{
		&TeapotProgramBackFront,
		"transparency_bias",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyBias
	},
	{
		&TeapotProgramBackFront,
		"transparency_scale",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TransparencyScale
	},
	{
		&TeapotProgramBackFront,
		"mvp",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FLOAT_MAT4,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		MVP
	},
	{
		&TeapotProgramBackFront,
		"base",
		0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_SAMPLER_2D,
		1,
		0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
		GL_FALSE,
		&TexUnitBase
	},
};

static int UnifomsInfomationCount = sizeof(UnifomsInfomation) / sizeof(UnifomsInfomation[0]);

/*
*	Texture Image
*/

static char * TexImgBaseFile = "Hex_MIPMAP_LEVEL_0.bmp";
/*
*	Passes Data
*/

/*
typedef struct
{
	char *			name;
	int				in_use;
	GLenum			glessl_type;
	GLint			glessl_size;
	GLuint *		gl_bufobj_name;
	GLuint *		general_index;
	GLint			size_of_type;
	GLenum			type;
	GLboolean		normalized;
	GLsizei			stride;
	const void *	ptr;
}
VDKS_Struct_AttributeInformation;
*/

static VDKS_Struct_AttributeInformation pAttributesInformation[] =
{
	{
		"position",
		0, /*need to set.*/
		GL_FLOAT_VEC3,
		1,
		&PositionGLBuffer,
		&PositionVertexAttributeArrayIndex, /*general_index, need to set.*/
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0 /*ptr, we use buffer object.*/
	},
	{
		"tex_coord0",
		0, /*need to set.*/
		GL_FLOAT_VEC2,
		1,
		&TextureCoord_0_GLBuffer,
		&TextureCoord_0_VertexAttributeArrayIndex,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0 /*ptr, we use buffer object.*/
	}
};

static int AttributesInformationCount = sizeof(pAttributesInformation) / sizeof(pAttributesInformation[0]);



/*
*	Make program & set attributes' locations.
*/
GLuint TeapotMakeProgramPresetLocation(
	const char* VSFile, const char* FGFile,
	VDKS_Struct_AttributeInformation* pAttrInfo, int cntAttrInfo )
{
	GLuint Program = glCreateProgram();
	char szCurDir[MAX_PATH + 1];
	char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];

        VDKS_Func_GetCurrentDir(szCurDir);

	strcpy(szVSFile, szCurDir);
	strcpy(szFGFile, szCurDir);
	strcat(szVSFile, VSFile);
	strcat(szFGFile, FGFile);

	/*
	*	Set assumed attribute location before glLinkProgram.
	*/

	VDKS_Func_Program_PresetAttributesLocations( Program, pAttrInfo, cntAttrInfo);

	Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);
	if (Program == 0)
	{
		fprintf(stderr, "Failed to create a new program.\n");
	}

	return Program;
}

static void TeapotInitTextureUnit(GLuint Tex)
{
	glActiveTexture(GL_TEXTURE0 + Tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
        glActiveTexture(GL_TEXTURE0);
}

/*
*	Buffer Attributes Data in to buffer object, and alloc locations & set usage of the bufobj.
*/
VDKS_BOOL TeapotInit(void)
{
	char szCurDir[MAX_PATH + 1];
	char szTempFile[MAX_PATH + 1];

	VDKS_Func_GetCurrentDir(szCurDir);
	/*
	* Positon
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositionFile);
	if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &PositionFloats, (SIZE_T *)&PositionFloatsCount))
	{
		fprintf(stderr, "Init : Failed to read position data file. \n");
		return VDKS_FALSE;
	}
        else
        {
                fprintf(stdout, "TeapotInit : Read %lu Floats from %s. \n",
                        PositionFloatsCount, szTempFile);
        }

	glGenBuffers(1, &PositionGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, PositionFloatsCount * sizeof(float), PositionFloats, GL_STATIC_DRAW);

	/*
	*	TexCoords.
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TextureCoord_0_File);
	if (VDKS_TRUE != VDKS_ReadFloats (szTempFile, &TextureCoord_0_Floats, (SIZE_T *)&TextureCoord_0_FloatsCount))
	{
		fprintf(stderr, "Init : Failed to read position data file.\n");
		return VDKS_FALSE;
	}
        else
        {
                fprintf(stdout, "TeapotInit : Read %lu Floats from %s. \n",
                        TextureCoord_0_FloatsCount, szTempFile);
        }

	glGenBuffers(1, &TextureCoord_0_GLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, TextureCoord_0_GLBuffer);
	glBufferData(GL_ARRAY_BUFFER, TextureCoord_0_FloatsCount * sizeof(float), TextureCoord_0_Floats, GL_STATIC_DRAW);

	/*
	 * Triangles
	*/
        strcpy(szTempFile, szCurDir);
        strcat(szTempFile, TriangleFile);

        unsigned short * pTeapotData = NULL;

	if (VDKS_TRUE != VDKS_ReadTriangle(szTempFile, &pTeapotData, (SIZE_T *)&TeapotVertexCount))
	{
		fprintf(stderr, "Init : Failed to read triangle data file.\n");
		return VDKS_FALSE;
	}
        else
        {
                fprintf(stdout, "TeapotInit : Read %lu Vertex from %s. \n",
                        TeapotVertexCount, szTempFile);
        }

	glGenBuffers(1, &TeapotGLBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TeapotGLBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TeapotVertexCount * sizeof(unsigned short), pTeapotData, GL_STATIC_DRAW);

	/*
	 * Debug, Model Center & Radius
	*/
        float CenterX = 0.0f;
        float CenterY = 0.0f;
        float CenterZ = 0.0f;
        float Radius = 0.0f;
        
	VDKS_Func_ModelCenterRadius(PositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);

	VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",
                CenterX, CenterY, CenterZ, Radius);

	/*
	 * Matrix
	*/
	do
	{
		float view[3] = {0.0, 0.0, 200};
		float focus[3] = {0.0, 0.0, 0.0};
		float up[3] = {0.0, 1.0, 0.0};

		VDKS_Func_LoadIdentity(ModelView);
		VDKS_Func_LoadIdentity(Projection);
		VDKS_Func_LoadIdentity(MVP);

		VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

		VDKS_Func_Matrix_Ortho(
			-1.0f * Radius, Radius, /*left, right*/
			-1.0f * Radius, Radius, /*bottom, up*/
			1.0, 400.0, /*near, far*/
			Projection);

		VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

		//VDKS_Func_Translate(ModelView, -1.0f * CenterX, -1.0f * CenterY, -1.0f * CenterZ);
	}
	while(0);

	/*
	*	Program(s)
	*/

	VDKS_Func_LocationManagerInit();

	PositionVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	assert (PositionVertexAttributeArrayIndex);

	TextureCoord_0_VertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	assert (TextureCoord_0_VertexAttributeArrayIndex);

	/*
	*	As we have set the location name, so we can set the approprate 
        *	index and usage of the buffer object in the pre-final
	*	vertex attribute array(s?) set.
	*/

	VDKS_Func_BufferObject_SetUsage(pAttributesInformation, AttributesInformationCount);

	TeapotProgramFrontBack = TeapotMakeProgramPresetLocation(
		FrontBackVSFile,
		FrontBackFGFile,
		pAttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramBackFront = TeapotMakeProgramPresetLocation(
		BackFrontVSFile,
		BackFrontFGFile,
		pAttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramFrontFront = TeapotMakeProgramPresetLocation(
		FrontFrontVSFile,
		FrontFrontFGFile,
		pAttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramBackBack = TeapotMakeProgramPresetLocation(
		BackBackVSFile,
		BackBackFGFile,
		pAttributesInformation,
		AttributesInformationCount
		);

	/*
	*	Texture Unit
	*/
	TeapotInitTextureUnit(TeapotTexUnitBackground);
	TeapotInitTextureUnit(TeapotTexUnitFrontFront);
	TeapotInitTextureUnit(TeapotTexUnitFrontBack);
	TeapotInitTextureUnit(TeapotTexUnitBackFront);
	TeapotInitTextureUnit(TeapotTexUnitBackBack);
	TeapotInitTextureUnit(TexUnitBase);

	/*
	*	Texture Object
	*/

	glGenTextures(1, &TeapotTexObjBackground);
	assert (TeapotTexObjBackground != 0);

	glGenTextures(1, &TeapotTexObjFrontFront);
	assert (TeapotTexObjFrontFront != 0);

	glGenTextures(1, &TeapotTexObjFrontBack);
	assert (TeapotTexObjFrontBack != 0);

	glGenTextures(1, &TeapotTexObjBackFront);
	assert (TeapotTexObjBackFront != 0);

	glGenTextures(1, &TeapotTexObjBackBack);
	assert (TeapotTexObjBackBack != 0);

	/*
	*	Texture "base"
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TexImgBaseFile);
	do
	{
		int w, h;


		unsigned char * img = VDKS_Func_ReadBmp((char *)szTempFile, &w, &h);
		if (NULL == img)
		{
			return VDKS_FALSE;
		}
                else
                {
                        printf(" %s: %dx%d readed \n", szTempFile, w, h );
                }

/*               
                int wi, hi;

		for (hi = 0; hi < h; hi++)
		{
			for (wi = 0; wi < w; wi++)
			{
				img[hi * w * 4 + wi * 4 + 0] = img[hi * w * 4 + wi * 4 + 3];
				img[hi * w * 4 + wi * 4 + 1] = img[hi * w * 4 + wi * 4 + 3];
				img[hi * w * 4 + wi * 4 + 2] = img[hi * w * 4 + wi * 4 + 3];
				img[hi * w * 4 + wi * 4 + 3] = img[hi * w * 4 + wi * 4 + 3];
				img[hi * w * 4 + wi * 4 + 3] = 255;
			}
		}
*/
		// VDKS_Func_SaveBmp("test.bmp",w, h, img);

		glGenTextures(1, &TexObjBase);
		assert(TexObjBase);

		glActiveTexture(GL_TEXTURE0 + TexUnitBase);

		glBindTexture(GL_TEXTURE_2D, TexObjBase);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

		// glGenerateMipmap(GL_TEXTURE_2D);

		// glActiveTexture(GL_TEXTURE0 + TexUnitBase);
	}
	while(0);

	return VDKS_TRUE;
}

void TeapotPassProgram (GLuint uProgram)
{
	/*
	*	Get Active Attributes
	*/

        printf( "Program: %u \n", uProgram);
	VDKS_Func_Program_QueryActiveAttributesCheckConsistent(	uProgram,
                pAttributesInformation, AttributesInformationCount );
	int i;

	/*
	*	Pre-binding the general indices.
	*/
	for(i = 0; i < AttributesInformationCount; ++i)
	{
		glBindAttribLocation(uProgram, 
                        *(pAttributesInformation[i].general_index), 
                         pAttributesInformation[i].name);

                // Enable the vertex attribute array locations.

                if (pAttributesInformation[i].in_use == 1)
		{
			glEnableVertexAttribArray(*(pAttributesInformation[i].general_index));
		}

	}


	/*
	*	Get Active Uniforms.
	*/

	VDKS_Func_Program_ValidateUniformsGetLocations(	uProgram, UnifomsInfomation, UnifomsInfomationCount );

	VDKS_Func_Program_SettingUniforms( uProgram, UnifomsInfomation, UnifomsInfomationCount );
}


void TeapotPass (GLuint uProgram)
{
	GLint program_status = 0;

	TeapotPassProgram(uProgram);

	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(uProgram);

	glValidateProgram(uProgram);

	glGetProgramiv(uProgram, GL_VALIDATE_STATUS, &program_status);

	if (program_status != GL_TRUE)
	{
		fprintf(stderr, "The program is not valid now.\n");
		return;
	}

	glUseProgram(uProgram);

	/*
	*	Other init routine may edit our GL_ELEMENT_ARRAY_BUFFER binding.
	*/
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TeapotGLBuffer);

	glDrawElements(GL_TRIANGLES, TeapotVertexCount, GL_UNSIGNED_SHORT, 0);

	glFlush();

	glUseProgram(0);
}

