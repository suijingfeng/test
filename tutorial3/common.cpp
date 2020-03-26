/****************************************************************************
*
*    Copyright 2012 - 2013 Vivante Corporation, Sunnyvale, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include "common.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

size_t fread_bit32(void * buf, size_t size, FILE * file)
{
    unsigned char cdata[4];
    size_t i, c = 0, t;

    for (i = 0; i < size / sizeof(unsigned int); ++i)
    {
        t = fread(&cdata, 4, 1, file);
        if (t == 0) break;

        c += t;

        ((unsigned int *)buf)[i] = (cdata[3] << 24) + (cdata[2] << 16)
                + (cdata[1] << 8) + cdata[0];
    }

    return c;
}

/***************************************************************************************
***************************************************************************************/

// Computes file length.
static int flength(FILE * Fptr)
{
    int length;
    fseek(Fptr, 0, SEEK_END);
    length = ftell(Fptr);
    fseek(Fptr, 0, SEEK_SET);
    return length;
}

// Compile a vertex or pixel shader.
// returns 0: fail
//         1: success
int CompileShader(const char * FName, GLuint ShaderNum)
{
    FILE * fptr = NULL;
#ifdef UNDER_CE
    static wchar_t buffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, buffer, MAX_PATH);
    while (buffer[i - 1] != L'\\') i--;
    while (*FName != '\0') buffer[i++] = (wchar_t)(*FName++);
    buffer[i] = L'\0';
    fptr = _wfopen(buffer, L"rb");
#else
    fptr = fopen(FName, "rb");
#endif
    if (fptr == NULL)
    {
        fprintf(stderr, "Can not open file: %s\n", FName);
        return 0;
    }

    int length = flength(fptr);

    char * shaderSource = (char*)malloc(sizeof (char) * length);
    if (shaderSource == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return 0;
    }

    fread(shaderSource, length, 1, fptr);

    glShaderSource(ShaderNum, 1, (const char**)&shaderSource, &length);
    glCompileShader(ShaderNum);

    free(shaderSource);
    fclose(fptr);

    GLint compiled = 0;
    glGetShaderiv(ShaderNum, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        // Retrieve error buffer size.
        GLint errorBufSize, errorLength;
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
        if (!infoLog)
        {
            // Retrieve error.
            glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s\n", infoLog);

            free(infoLog);
        }
        return 0;
    }

    return 1;
}

/***************************************************************************************
***************************************************************************************/

static void ConvertARGBtoABGR(unsigned int * data, int size)
{
    for (int i = 0; i < size * size; i++, data++)
    {
        unsigned char a1[4], a2[4];
        memcpy(a1, data, sizeof(unsigned int));
        // Swap R and B.
        a2[0] = a1[2];
        a2[1] = a1[1];
        a2[2] = a1[0];
        a2[3] = a1[3];
        memcpy(data, a2, 4);
    }
}

// loads a 256x256 ARGB (32bit) format cube map texture dds file.
CubeTexture* LoadCubeDDS(const char * filename)
{
    FILE * fptr = NULL;

    fptr = fopen(filename, "rb");

    if (fptr == NULL)
    {
        fprintf(stderr, "Can not open file: %s\n", filename);
        return NULL;
    }

    // read dds file routine.
    CubeTexture * cbTexture = NULL;
    do
    {
        unsigned int size, offset, width;
        unsigned long dummy=0;
        // read first 4 bytes, magic number.
        if (!fread_bit32(&dummy, 4, fptr))
            break;
        // check magic number, should be "DDS ": 0x20534444.
        if (dummy != 0x20534444)
        {
            fprintf(stderr, "File format incorrect.\n");

            break;
        }

        fprintf(stdout, "file identifier: 0x%lx\n", dummy);

        // second 4 bytes: texture offset in file.
        if (!fread_bit32(&dummy, 4, fptr))
            break;
        offset = dummy + 4;

        // no care.
        if (!fread_bit32(&dummy, 4, fptr))
            break;
        // no care.
        if (!fread_bit32(&dummy, 4, fptr))
            break;


	fprintf(stdout, "skip 8 bytes.\n");


        // texture width
        if (!fread_bit32(&width, 4, fptr))
            break;

        size = width * width;

        fprintf(stdout, "width: %d\n", width);


        if (fseek(fptr, offset, SEEK_SET))
            break;

        fprintf(stdout, "sizeof (CubeTexture): %d\n", sizeof (CubeTexture));

        if ((cbTexture = (CubeTexture*)malloc(sizeof (CubeTexture))) == NULL)
            break;

        // read faces
        // positive x
        //  RGBA
        if ((cbTexture->posx = (unsigned int*) malloc( 4 * size)) == NULL)
            break;

        if (fread(cbTexture->posx, 4 , size, fptr) != size)
        {
            break;
            fprintf(stdout, "cbTexture->posx: %d\n", size);
        }

        ConvertARGBtoABGR(cbTexture->posx, width);

        // negative x
        if (fseek(fptr, 87380, SEEK_CUR))   // FIXME, assuming size is 256 and mipmaps included
            break;
        if ((cbTexture->negx = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->negx, sizeof (unsigned int), size, fptr) != size)
            break;
        ConvertARGBtoABGR(cbTexture->negx, width);

        // positive y
        if (fseek(fptr, 87380, SEEK_CUR))   // FIXME, assuming size is 256 and mipmaps included
            break;
        if ((cbTexture->posy = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->posy, sizeof (unsigned int), size, fptr) != size)
            break;
        ConvertARGBtoABGR(cbTexture->posy, width);

        // negative y
        if (fseek(fptr, 87380, SEEK_CUR))   // FIXME, assuming size is 256 and mipmaps included
            break;
        if ((cbTexture->negy = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->negy, sizeof (unsigned int), size, fptr) != size)
            break;
        ConvertARGBtoABGR(cbTexture->negy, width);

        // positive z
        if (fseek(fptr, 87380, SEEK_CUR))   // FIXME, assuming size is 256 and mipmaps included
            break;
        if ((cbTexture->posz = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->posz, sizeof (unsigned int), size, fptr) != size)
            break;
        ConvertARGBtoABGR(cbTexture->posz, width);

        // negative z
        if (fseek(fptr, 87380, SEEK_CUR))   // FIXME, assuming size is 256 and mipmaps included
            break;
        if ((cbTexture->negz = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->negz, sizeof (unsigned int), size, fptr) != size)
            break;
        ConvertARGBtoABGR(cbTexture->negz, width);

        cbTexture->img_size = width;

        fclose(fptr);

        fprintf(stdout, "lodaing %s done.\n", filename);

        return cbTexture;
    }
    while (false);

    // error handle
    fprintf(stderr, "Error while read DDS file.\n");

    fclose(fptr);

    if(cbTexture != NULL) {
    	DeleteCubeTexture(cbTexture);
        cbTexture = NULL;
    }
    return NULL;
}

/***************************************************************************************
***************************************************************************************/

void DeleteCubeTexture(CubeTexture * cube)
{
    if (cube != NULL)
    {
	if(cube->posx != NULL)
        {
            free(cube->posx);
            cube->posx = NULL;
        }

	if(cube->posy != NULL)
        {
            free(cube->posy);
            cube->posy = NULL;
        }

	if(cube->posz != NULL)
        {
            free(cube->posz);
            cube->posz = NULL;
        }

	if(cube->negx != NULL)
        {
            free(cube->negx);
            cube->negx = NULL;
        }

	if(cube->negy != NULL)
        {
            free(cube->negy);
            cube->negy = NULL;
        }

	if(cube->negz != NULL)
        {
            free(cube->negz);
            cube->negz = NULL;
        }

        free(cube);
     }
}
