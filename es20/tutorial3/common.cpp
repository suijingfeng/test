#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#include "common.h"

size_t fread_bit32(void * buf, size_t size, FILE * file)
{
    unsigned char cdata[4];
    size_t i, c = 0, t;

    for (i = 0; i < size / sizeof(unsigned int); i++)
    {
        t = fread(&cdata, sizeof(unsigned int), 1, file);
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
    FILE * fptr = fopen(FName, "rb");
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
    else
    {
        fprintf(stdout, " %d Bytes Allocated. \n", length);
    }


    size_t readed = fread(shaderSource, 1, length, fptr);

    fprintf(stdout, " %d Bytes Readed. \n", (int)readed);

    glShaderSource(ShaderNum, 1, (const char**)&shaderSource, NULL);
    
    glCompileShader(ShaderNum);


    GLint compiled = 0;
    glGetShaderiv(ShaderNum, GL_COMPILE_STATUS, &compiled);
    
    if (0 == compiled)
    {
        fprintf(stderr, "Error: Compile Shader %s. \n", FName);
        // Retrieve error buffer size.
        GLint errorBufSize = 0, errorLength = 0;
        
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        fprintf(stderr, "errorBufSize: %d. \n", errorBufSize);

        char * infoLog = (char*) malloc( errorBufSize * sizeof(char) + 1);

        fprintf(stdout, " %ld Bytes Allocated. \n", errorBufSize * sizeof(char) + 1);


        if (!infoLog)
        {
            // Retrieve error.
            glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s\n", infoLog);

            free(infoLog);
        }
    }

    
    free(shaderSource);
    fclose(fptr);

    return compiled;
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
        memcpy(data, a2, sizeof(unsigned int));
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
        unsigned int dummy, size, offset, width;

        // read first 4 bytes, magic number.
        if (!fread_bit32(&dummy, sizeof (unsigned int), fptr))
            break;
        // check magic number, should be "DDS ": 0x20534444.
        if (dummy != 0x20534444UL)
        {
            fprintf(stderr, "File format incorrect.\n");
            break;
        }

        // second 4 bytes: texture offset in file.
        if (!fread_bit32(&dummy, sizeof (unsigned int), fptr))
            break;
        offset = dummy + 4;

        // no care.
        if (!fread_bit32(&dummy, sizeof (unsigned int), fptr))
            break;
        // no care.
        if (!fread_bit32(&dummy, sizeof (unsigned int), fptr))
            break;

        // texture width
        if (!fread_bit32(&width, sizeof (unsigned int), fptr))
            break;
        size = width * width;

        if (fseek(fptr, offset, SEEK_SET))
            break;

        if ((cbTexture = (CubeTexture*)malloc(sizeof (CubeTexture))) == NULL)
            break;

        // read faces
        // positive x
        if ((cbTexture->posx = (unsigned int*)malloc(sizeof (unsigned int) * size)) == NULL)
            break;
        if (fread(cbTexture->posx, sizeof (unsigned int), size, fptr) != size)
            break;
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
        return cbTexture;
    }
    while (false);

    // error handle
    fprintf(stderr, "Error while read DDS file.\n");

    fclose(fptr);
    DeleteCubeTexture(cbTexture);

    return NULL;
}

/***************************************************************************************
***************************************************************************************/

void DeleteCubeTexture(CubeTexture * cube)
{
    if (cube != NULL)
    {
        void * pointers[] = {
            (void*)cube->posx,
            (void*)cube->posy,
            (void*)cube->posz,
            (void*)cube->negx,
            (void*)cube->negy,
            (void*)cube->negz,
            (void*)cube
        };

        for (unsigned int i = 0; i < (sizeof (pointers) / sizeof (pointers[0])); i++)
        {
            if (pointers[i] != NULL)
            {
                free(pointers[i]);
            }
        }
    }
}


unsigned long GetTimeMillis(void)
{
#if defined(_WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);
    
    assert(freq.LowPart != 0 || freq.HighPart != 0);

    if (count.QuadPart < MAXLONGLONG / 1000000) {
        assert(freq.QuadPart != 0);
        return count.QuadPart * 1000000 / freq.QuadPart;
    } else {
        assert(freq.QuadPart >= 1000000);
        return count.QuadPart / (freq.QuadPart / 1000000);
    }

#elif defined(__unix__) || defined(__linux) || defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
    struct timespec currTime;
    clock_gettime(CLOCK_MONOTONIC, &currTime);
    return (uint64_t)currTime.tv_sec * 1000000 + ((uint64_t)currTime.tv_nsec / 1000);

#elif defined(__EPOC32__)
    struct timespec currTime;
    /* Symbian supports only realtime clock for clock_gettime. */
    clock_gettime(CLOCK_REALTIME, &currTime);
    return (uint64_t)currTime.tv_sec * 1000000 + ((uint64_t)currTime.tv_nsec / 1000);

#elif defined(__APPLE__)
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return (uint64_t)currTime.tv_sec * 1000000 + (uint64_t)currTime.tv_usec;

#else
#error "Not implemented for target OS"
#endif
}
