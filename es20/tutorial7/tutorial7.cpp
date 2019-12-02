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


/*
 * OpenGL ES 2.0 Tutorial 7
 *
 * Draws a simple triangle with basic vertex and pixel shaders into
 * a MSAA texture. Use this texture in the second pass to render onto
 * the screen.
 */

#ifdef UNDER_CE
#include <windows.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 7: MSAA texture"

#ifndef ANDROID_JNI
#include <gc_vdk.h>
/* to hold vdk information */
vdkEGL egl;
#endif

int width       = 0;
int height      = 0;
int texWidth    = 1024;
int texHeight   = 768;
int posX        = 0;
int posY        = 0;
int samples     = 0;
int texSamples  = 0;
int frames      = 0;


/* Global Variables, attribute and uniform */
GLint locVertices      = 0;
GLint locColors        = 0;
GLint locTransformMat  = 0;
GLint locVertices2     = 0;
GLint locTexture       = 0;
GLint locTextureUV     = 0;
GLint locTransformMat2 = 0;

/* Global Variables, shader handle and program handle */
GLuint vertShaderNum    = 0;
GLuint pixelShaderNum   = 0;
GLuint programHandle[2] = {0, 0};
GLuint texA;
GLuint texB;

GLuint framebuffer;
GLuint depthbuffer;
static GLuint imageTexture;

int argCount = 6;
char argSpec = '-';
char argNames[] = {'x', 'y', 'w', 'h', 's', 'f'};
char argValues[][255] =
{
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
};

char argDescs[][255] =
{
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
};

int noteCount = 1;
char argNotes[][255] =
{
    "Exit: [ESC] or the frame count reached."
};

/* Triangle Vertex positions */
const GLfloat TriVertices[3][2] =
{
    { -0.5f, -0.5f},
    {  0.0f,  0.5f},
    {  0.5f, -0.5f}
};

/* Triangle Vertex colors */
const GLfloat TriColor[3][3] = {
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f}
};

/* Start with an identity matrix */
GLfloat transformMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

GLfloat transformQuadMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/* Quad Vertex positions */
const GLfloat QuadVertices[4][2] =
{
    { -0.75f, -0.75f},
    {  0.75f, -0.75f},
    { -0.75f,  0.75f},
    {  0.75f,  0.75f}
};

/* Quad Vertex texture coordinates */
const GLfloat texUV[4][2] =
{
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f},
};

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC  glRenderbufferStorageMultisampleEXTProc;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXTProc;

/***************************************************************************************
***************************************************************************************/

int RenderInit()
{
    int maxTexSamples;
    const char* extensionString = (const char*)glGetString(GL_EXTENSIONS);

    if (strstr(extensionString, "GL_EXT_multisampled_render_to_texture") != NULL)
    {
        glRenderbufferStorageMultisampleEXTProc = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)
            eglGetProcAddress("glRenderbufferStorageMultisampleEXT");

        glFramebufferTexture2DMultisampleEXTProc = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)
            eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
    }

    if ((glRenderbufferStorageMultisampleEXTProc == NULL) ||
        (glFramebufferTexture2DMultisampleEXTProc == NULL))
    {
        printf("%s[%d] Do NOT support extension GL_EXT_multisampled_render_to_texture.\n",
               __FUNCTION__, __LINE__);
        return 0;
    }

    /* Triangle rendering path. */
    /* Grab location of shader attributes.*/
    locVertices = glGetAttribLocation(programHandle[0], "my_Vertex");
    locColors   = glGetAttribLocation(programHandle[0], "my_Color");

    /* Transform Matrix is uniform for all vertices here.*/
    locTransformMat = glGetUniformLocation(programHandle[0], "my_TransformMatrix");

    /* Quad rendering path. */
    locVertices2 = glGetAttribLocation(programHandle[1], "my_Vertex");
    locTextureUV = glGetAttribLocation(programHandle[1], "my_texUV");
    locTexture   = glGetUniformLocation(programHandle[1], "my_Texture");

    /* Transform Matrix is uniform for all vertices here.*/
    locTransformMat2 = glGetUniformLocation(programHandle[1], "my_TransformMatrix");

    /* Create an offscreen framebuffer with a renderbuffer as color buffer */
    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);

    /* Set filtering modes.*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Upload the data to the texture.*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &framebuffer);
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxTexSamples);
    texSamples = (texSamples > maxTexSamples) ? maxTexSamples : texSamples;

    /* Create multisampled color renderbuffer */
    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);

    printf("%s[%d] %p samples:%d texSamples:%d\n", __FUNCTION__, __LINE__,
           glRenderbufferStorageMultisampleEXTProc, samples, texSamples);
    glRenderbufferStorageMultisampleEXTProc(GL_RENDERBUFFER, texSamples, GL_DEPTH_COMPONENT16, texWidth, texHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return 1;
}

void drawQuadUsingTexture()
{
    glUseProgram(programHandle[1]);

    /* enable vertex arrays to push the data.*/
    glEnableVertexAttribArray(locVertices2);
    glEnableVertexAttribArray(locTextureUV);

    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glVertexAttribPointer(locVertices2, 2, GL_FLOAT, GL_FALSE, 0, &QuadVertices[0][0]);
    glVertexAttribPointer(locTextureUV, 2, GL_FLOAT, GL_FALSE, 0, &texUV[0][0]);
    glUniformMatrix4fv(locTransformMat2, 1, GL_FALSE, transformQuadMatrix);
    glUniform1i(locTexture, 0);

    /* Clear background.*/
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* disable vertex arrays.*/
    glDisableVertexAttribArray(locVertices2);
    glDisableVertexAttribArray(locTextureUV);
}

/* Actual rendering here.*/
void Render()
{
    static float angle = 0.0;

    /* Draw a triangle onto texA. */
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Attach texA to the FBO.*/
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
    glFramebufferTexture2DMultisampleEXTProc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imageTexture, 0, texSamples);

    /* Handle unsupported cases */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("%s[%d] Framebuffer is NOT complete.\n", __FUNCTION__, __LINE__);
        return;
    }

    /* Draw triangle.*/
    glUseProgram(programHandle[0]);

    /* enable vertex arrays to push the data.*/
    glEnableVertexAttribArray(locVertices);
    glEnableVertexAttribArray(locColors);

    /* set data in the arrays.*/
    glVertexAttribPointer(locVertices, 2, GL_FLOAT, GL_FALSE, 0, &TriVertices[0][0]);
    glVertexAttribPointer(locColors, 3, GL_FLOAT, GL_FALSE, 0, &TriColor[0][0]);
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);

    /* Clear background.*/
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Set up rotation matrix rotating by angle around y axis.*/
    transformMatrix[0] = transformMatrix[10] = (GLfloat)cos(angle);
    transformMatrix[2] = (GLfloat)sin(angle);
    transformMatrix[8] = -transformMatrix[2];
    angle += 0.1f;
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(locVertices);
    glDisableVertexAttribArray(locColors);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    drawQuadUsingTexture();

    /* flush all commands.*/
    glFlush ();

#ifndef ANDROID_JNI
    /* swap display with drawn surface.*/
    vdkSwapEGL(&egl);
#endif
}

void RenderCleanup()
{
    /* cleanup */
    glDisableVertexAttribArray(locVertices);
    glDisableVertexAttribArray(locColors);
    glDeleteFramebuffers(1, &framebuffer);
}

/***************************************************************************************
***************************************************************************************/

/* Compile a vertex or pixel shader.*/
/* returns 0: fail
           1: success */
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
        fprintf(stderr, "Cannot open file '%s'\n", FName);
        return 0;
    }

    int length;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    fseek(fptr, 0 ,SEEK_SET);

    char * shaderSource = (char*)malloc(sizeof (char) * length);
    if (shaderSource == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return 0;
    }

    if (!fread(shaderSource, length, 1, fptr))
    {
        fprintf(stderr, "fread error.\n");
        return 0;
    }

    glShaderSource(ShaderNum, 1, (const char**)&shaderSource, &length);
    glCompileShader(ShaderNum);

    free(shaderSource);
    fclose(fptr);

    GLint compiled = 0;
    glGetShaderiv(ShaderNum, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        /* Retrieve error buffer size.*/
        GLint errorBufSize, errorLength;
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
        if (!infoLog)
        {
            /* Retrieve error.*/
            glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s\n", infoLog);
            free(infoLog);
        }

        fprintf(stderr, "Error compiling shader '%s'\n", FName);
        return 0;
    }

    return 1;
}

/***************************************************************************************
***************************************************************************************/

/* Wrapper to load vertex and pixel shader.*/
void LoadShaders(const char * vShaderFName, const char * pShaderFName, unsigned int index)
{
    vertShaderNum = glCreateShader(GL_VERTEX_SHADER);
    pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);

    if (CompileShader(vShaderFName, vertShaderNum) == 0)
    {
        return;
    }

    if (CompileShader(pShaderFName, pixelShaderNum) == 0)
    {
        return;
    }

    programHandle[index] = glCreateProgram();
    glAttachShader(programHandle[index], vertShaderNum);
    glAttachShader(programHandle[index], pixelShaderNum);
    glLinkProgram(programHandle[index]);

    /* Check if linking succeeded.*/
    GLint linked = false;
    glGetProgramiv(programHandle[index], GL_LINK_STATUS, &linked);
    if (!linked)
    {
        /* Retrieve error buffer size.*/
        GLint errorBufSize, errorLength;
        glGetShaderiv(programHandle[index], GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (!infoLog)
        {
            /* Retrieve error.*/
            glGetProgramInfoLog(programHandle[index], errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s", infoLog);

            free(infoLog);
        }

        fprintf(stderr, "Error linking program\n");
        return;
    }
}

/* Cleanup the shaders.*/
void DestroyShaders()
{
    glDeleteShader(vertShaderNum);
    glDeleteShader(pixelShaderNum);
    glDeleteProgram(programHandle[0]);
    glDeleteProgram(programHandle[1]);
    glUseProgram(0);
}

/***************************************************************************************
***************************************************************************************/
#ifndef ANDROID_JNI

int ParseCommandLine(int argc, char * argv[])
{
    int result = 1;

    /* Walk all command line arguments.*/
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'x':
                /* x<x> for x position (defaults to -1).*/
                if (++i < argc)
                    posX = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'y':
                /* y<y> for y position (defaults to -1).*/
                if (++i < argc)
                    posY = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'w':
                /* w<width> for width (defaults to 0).*/
                if (++i < argc)
                    width = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'h':
                /* h<height> for height (defaults to 0).*/
                if (++i < argc)
                    height = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 's':
                /* s<samples> for samples (defaults to 0).*/
                if (++i < argc)
                    samples = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 't':
                /* s<samples> for samples (defaults to 0).*/
                if (++i < argc)
                    texSamples = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'f':
                /* f<count> for number of frames (defaults to 0).*/
                if (++i < argc)
                    frames = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            default:
                result = 0;
                break;
            }

            if (result == 0)
            {
                break;
            }

        }
        else
        {
            result = 0;
            break;
        }
    }

    return result;
}

void PrintHelp()
{
    int i;
    printf("Usage: ");
    printf("command + [arguments]\n");
    printf("Argument List:\n");

    for (i = 0; i < argCount; i++)
    {
        printf("\t");
        printf("%c%c %s\t%s\n", argSpec, argNames[i], argValues[i], argDescs[i]);
    }

    for (i = 0; i < noteCount; i++)
    {
        printf("%s\n", argNotes[i]);
    }
}

/***************************************************************************************
***************************************************************************************/

/* Program entry.*/
int main(int argc, char** argv)
{
    bool pause = false;

    /* EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.*/
    EGLint configAttribs[] =
    {
        EGL_SAMPLES,      0,
        EGL_RED_SIZE,     5,
        EGL_GREEN_SIZE,   6,
        EGL_BLUE_SIZE,    5,
        EGL_ALPHA_SIZE,   EGL_DONT_CARE,
        EGL_DEPTH_SIZE,   16,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE,
    };

    EGLint attribListContext[] =
    {
        /* Needs to be set for es2.0 as default client version is es1.1.*/
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    /* Parse the command line.*/
    if (!ParseCommandLine(argc, argv))
    {
        PrintHelp();
        return 0;
    }

    /* Set multi-sampling.*/
    configAttribs[1] = samples;

    /* Initialize VDK, EGL, and GLES.*/
    if (!vdkSetupEGL(posX, posY, width, height, configAttribs, NULL, attribListContext, &egl))
    {
        printf("%s[%d]\n", __FUNCTION__, __LINE__);
        return 1;
    }

    /* Set window title and show the window.*/
    vdkSetWindowTitle(egl.window, TUTORIAL_NAME);

    /* load and compiler vertex/fragment shaders.*/
    LoadShaders("vs_es20t7a.vert", "ps_es20t7a.frag", 0);
    LoadShaders("vs_es20t7b.vert", "ps_es20t7b.frag", 1);

    if ((programHandle != 0) && RenderInit())
    {
        vdkShowWindow(egl.window);

        int frameCount = 0;
        unsigned int start = vdkGetTicks();

        /* Main loop */
        for (bool done = false; !done;)
        {
            /* Get an event.*/
            vdkEvent event;
            if (vdkGetEvent(egl.window, &event))
            {
                /* Test for Keyboard event.*/
                if ((event.type == VDK_KEYBOARD)
                && event.data.keyboard.pressed
                )
                {
                    /* Test for key.*/
                    switch (event.data.keyboard.scancode)
                    {
                    case VDK_SPACE:
                        /* Use SPACE to pause.*/
                        pause = !pause;
                        break;

                    case VDK_ESCAPE:
                        /* Use ESCAPE to quit.*/
                        done = true;
                        break;

                    default:
                        break;
                    }
                }
                /* Test for Close event.*/
                else if (event.type == VDK_CLOSE)
                {
                    done = true;
                }
            }
            else if (!pause)
            {
                /* Render one frame if there is no event.*/
                Render();
                ++ frameCount;

                if ((frames > 0) && (--frames == 0))
                {
                    done = true;
                }

            }
        }

        glFinish();
        unsigned int end = vdkGetTicks();
        float fps = frameCount / ((end - start) / 1000.0f);
        printf("%d frames in %d ticks -> %.3f fps\n", frameCount, end - start, fps);

        RenderCleanup();
    }

    /* cleanup */
    DestroyShaders();
    vdkFinishEGL(&egl);

    return 0;
}
#endif
