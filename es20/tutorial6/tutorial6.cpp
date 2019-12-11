/*
 * OpenGL ES 2.0 Tutorial 5
 *
 * Draws a simple triangle with basic vertex and pixel shaders.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef ANDROID_JNI
#include <gc_vdk.h>
#else
#include <utils/Log.h>
#include <EGL/egl.h>
#endif
#include <gc_hal_eglplatform.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <sys/timeb.h>
#include <fcntl.h>

#include <string.h>

#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 6: EGLImage from native pixmap"

// Wrapper to load vetex and pixel shader.
void LoadShaders(const char * vShaderFName, const char * pShaderFName);
// Cleanup the shaders.
void DestroyShaders();


#ifndef ANDROID_JNI
// to hold vdk information.
vdkEGL egl;
#endif
NativePixmapType pixmap;

EGLImageKHR eglimage;

int width  = 320;
int height = 240;
int posX   = -1;
int posY   = -1;
int samples = 0;
int frames = 500;

// Global Variables, attribute and uniform
GLint locVertices     = 0;
GLint locTransformMat = 0;
GLint locTexcoord     = 0;
GLint locSampler = 0;
GLuint gTexObj = 0;

// Global Variables, shader handle and program handle
GLuint vertShaderNum  = 0;
GLuint pixelShaderNum = 0;
GLuint programHandle  = 0;

int argCount = 6;
char argSpec = '-';
char argNames[] = {'x', 'y', 'w', 'h', 's', 'f'};
char argValues[][255] = {
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
};
char argDescs[][255] = {
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
};
int noteCount = 1;
char argNotes[][255] = {
    "Exit: [ESC] or the frame count reached."
};

// Triangle Vertex positions.
const GLfloat vertices[][2] = {
    { -0.5f, -0.5f},
    {  0.5f, -0.5f},
    { -0.5f, 0.5f},
    {  0.5f,  0.5f}
};
const GLfloat texcoords[][2] = {
    { 0.0f, 1.0f},
    { 1.0f, 1.0f},
    { 0.0f, 0.0f},
    { 1.0f, 0.0f}
};

// Start with an identity matrix.
GLfloat transformMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

bool switchPixmapData = true;

/***************************************************************************************
***************************************************************************************/

#define BF_TYPE 0x4D42             /* "MB" */

typedef struct RGB {                      /**** Colormap entry structure ****/
    unsigned char   rgbBlue;          /* Blue value */
    unsigned char   rgbGreen;         /* Green value */
    unsigned char   rgbRed;           /* Red value */
    unsigned char   rgbReserved;      /* Reserved */
} RGB;

typedef struct BMPINFOHEADER {                     /**** BMP file info structure ****/
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
} BMPINFOHEADER;


typedef struct BMPINFO {                      /**** Bitmap information structure ****/
    BMPINFOHEADER   bmiHeader;      /* Image header */
    RGB             bmiColors[256]; /* Image colormap */
} BMPINFO;


typedef struct BMPFILEHEADER {                      /**** BMP file header structure ****/
    unsigned short bfType;           /* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
} BMPFILEHEADER;

/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */
static unsigned short read_word(FILE *fp)
{
    unsigned char b0, b1; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);

    return ((b1 << 8) | b0);
}

/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */
static unsigned int read_dword(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'read_long()' - Read a 32-bit signed integer.
 */
static int read_long(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

unsigned char * GltLoadDIBitmap(const char *filename, BMPINFO *info)
{
    FILE            *fp;          /* Open file pointer */
    unsigned char   *bits;        /* Bitmap pixel bits */
    unsigned int    bitsize;      /* Size of bitmap */
    int             infosize;     /* Size of header information */
    BMPFILEHEADER   header;       /* File header */

    /* Try opening the file; use "rb" mode to read this *binary* file. */
#ifdef UNDER_CE
    wchar_t buffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, buffer, MAX_PATH);
    while (buffer[i - 1] != L'\\') i--;
    while (*filename != '\0') buffer[i++] = (wchar_t)(*filename++);
    buffer[i] = L'\0';
    fp = _wfopen(buffer, L"rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (fp == NULL)
    {
        return (NULL);
    }

    /* Read the file header and any following bitmap information... */
    header.bfType      = read_word(fp);
    header.bfSize      = read_dword(fp);
    header.bfReserved1 = read_word(fp);
    header.bfReserved2 = read_word(fp);
    header.bfOffBits   = read_dword(fp);

    if (header.bfType != BF_TYPE){
        fprintf(stdout, "Not a bitmap file - return NULL...\n");
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - 18;

    info->bmiHeader.biSize          = read_dword(fp);
    info->bmiHeader.biWidth         = read_long(fp);
    info->bmiHeader.biHeight        = read_long(fp);
    info->bmiHeader.biPlanes        = read_word(fp);
    info->bmiHeader.biBitCount      = read_word(fp);
    info->bmiHeader.biCompression   = read_dword(fp);
    info->bmiHeader.biSizeImage     = read_dword(fp);
    info->bmiHeader.biXPelsPerMeter = read_long(fp);
    info->bmiHeader.biYPelsPerMeter = read_long(fp);
    info->bmiHeader.biClrUsed       = read_dword(fp);
    info->bmiHeader.biClrImportant  = read_dword(fp);

    if (infosize > 40){
        if (fread(info->bmiColors, infosize - 40, 1, fp) < 1){
            /* Couldn't read the bitmap header - return NULL... */
            fclose(fp);
            return (NULL);
        }
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if ((bitsize = info->bmiHeader.biSizeImage) == 0)
        bitsize = (info->bmiHeader.biWidth *
                   info->bmiHeader.biBitCount + 7) / 8 *
               abs(info->bmiHeader.biHeight);

    bits = (unsigned char *)malloc( sizeof(unsigned char) * bitsize);

    if ( bits == NULL )
    {
        /* Couldn't allocate memory - return NULL! */
        fclose(fp);
        return (NULL);
    }

    if (fread(bits, 1, bitsize, fp) < bitsize)
    {
        /* Couldn't read bitmap - free memory and return NULL! */
        free(bits);
        fclose(fp);
        return (NULL);
    }

    /* OK, everything went fine - return the allocated bitmap... */
    fclose(fp);
    return (bits);
}



GLuint LoadTexture( const char* FileName, NativePixmapType pixmap )
{
    GLuint result = 0;
    do
    {
        BMPINFO info = { 0 };
        int width, height, Width, Height;
        void* pointer = NULL;
        unsigned char* img = GltLoadDIBitmap(FileName, &info);

        if (img != NULL)
        {
            width =  info.bmiHeader.biWidth;
            height = info.bmiHeader.biHeight;
        }
        else
            return result;

        glFinish();

        vdkGetPixmapInfo(pixmap, &Width, &Height, NULL, NULL, &pointer);
        memcpy(pointer, img, width * height * 4);
        free(img);
        /* Success. */
        result = 1;
    }
    while (0);
    /* Return result. */
    return result;
}

bool RenderInit( void )
{
    EGLint attrib_list [] = {
        EGL_NONE
    };

    glGenTextures(1, &gTexObj);
    // Grab location of shader attributes.
    locVertices = glGetAttribLocation(programHandle, "my_Vertex");
    locTexcoord = glGetAttribLocation(programHandle, "my_Texcoor");
    // Transform Matrix is uniform for all vertices here.
    locTransformMat = glGetUniformLocation(programHandle, "my_TransformMatrix");
    locSampler = glGetUniformLocation(programHandle, "my_Sampler");

    // enable vertex arrays to push the data.
    glEnableVertexAttribArray(locVertices);
    glEnableVertexAttribArray(locTexcoord);

    // set data in the arrays.
    glVertexAttribPointer(locVertices, 2, GL_FLOAT, GL_FALSE, 0, &vertices[0][0]);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, &texcoords[0][0]);
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);
    glUniform1i(locSampler, 0);// gTexObj);

    // fucking here !!!!!!!
    // pixmap = vdkCreatePixmap(egl.display, 320, 240, 32);
    //
    // EGL also supports rendering surfaces whose color buffers are stored 
    // in native pixmaps. Pixmaps differ from windows in that they are typically 
    // allocated in offscreen (non-visible) graphics or CPU memory.
    // 
    // Pixmaps differ from pbuffers in that they do have an associated native 
    // pixmap and native pixmap type, and it may be possible to render to pixmaps 
    // using APIs other than client APIs.\
    //
    // To create a pixmap rendering surface, first create a native platform pixmap,
    // then select an EGLConfig matching the pixel format of that pixmap ( calling
    // eglChooseConfig with an attribute list including EGL_MATCH_NATIVE_PIXMAP
    // returns only EGLConfigs matching the pixmap specified in the attribute list.)
    //
    //  EGLSurface  eglCreatePlatformPixmapSurface( EGLDisplay dpy, 
    //       EGLConfig config, void * native_pixmap, const EGLAttrib * attrib_list);
    //
    // creates an offscreen EGLSurface and returns a handle to it. 
    // Any EGL context created with a compatible EGLConfig can be used to render 
    // into this surface. native pixmap must belong to the same platform as dpy,
    // and EGL considers the returned EGLSurface as belonging to that same platform.
    //
    // The extension that defines the platform to which dpy belongs also defines 
    // the requirements for the native pixmap parameter.
    //
    // attrib list specifies a list of attributes for the pixmap. The list has the
    // same structure as described for eglChooseConfig. Attributes that can be 
    // specified in attrib list include EGL_GL_COLORSPACE , EGL_VG_COLORSPACE and 
    // EGL_VG_ALPHA_FORMAT .
    //
    // It is possible that some platforms will define additional attributes specific
    // to those environments, as an EGL extension. attrib list may be NULL or empty 
    // (first attribute is EGL_NONE ), in which case all attributes assumes their 
    // default value.
    // 
    //
    // EGL_GL_COLORSPACE , EGL_VG_COLORSPACE and EGL_VG_ALPHA_FORMAT have the same 
    // meaning and default values as when used with eglCreatePlatformWindowSurface.
    //
    // The resulting pixmap surface will contain color and ancillary buffers as 
    // specified by config. Buffers which are present in pixmap (normally, just 
    // the color buffer) will be bound to EGL. 
    //
    // Buffers which are not present in pixmap (such as depth and stencil, if 
    // config includes those buffers) will be allocated by EGL in the same fashion 
    // as for a surface created with eglCreatePbufferSurface.
    //
    // If dpy and native pixmap do not belong to the same platform, then undefined
    // behavior occurs. 
    //
    //
    // EGL pixmaps are not actual entities. They are an EGL abstraction of an already 
    // existing native pixmap. A call to eglCreatePixmapSurface does not allocate any 
    // memory or create a pixmap. It only ¡°wraps¡± a pre-existing pixmap in an EGL structure.
    //
    // EGL pixmaps are meant to mirror any pixmap-like concept supported by the 
    // underlying windowing system. The terminology comes from X11.
    //
    // EGL defines several types of drawing surfaces collectively referred to as 
    // EGLSurfaces. These include windows, used for onscreen rendering; pbuffers, 
    // used for offscreen rendering; and pixmaps, used for offscreen rendering 
    // into buffers that may be accessed through native APIs. EGL windows and 
    // pixmaps are tied to native window system windows and pixmaps.
    //
    //  The main differences between pixmaps and windows are:
    //
    //  An EGL implementation doesn't need to support pixmaps (the native windowing 
    //  system might not have an equivalent concept)
    //
    //  Pixmaps are typically single-buffered, whereas a window might be double- or 
    //  triple-buffered.
    //
    //  Pixmaps are offscreen surfaces (although offscreen windows might be possible 
    //  as well)
    //
    //  If the underlying windowing system doesn't support pixmaps, there's no real 
    //  use for EGL pixmaps (and they probably won't be exposed by the EGL implementation). 
    //  They're only provided so you can render to X11 pixmaps (or something similar) 
    //  in case you need to do that for some reason.
    //
    //  One example might be: you want to render a 3D object using the GPU and then
    //  use it as an X11 cursor. In that case you might create a pixmap, create an 
    //  EGL pixmap drawable, draw something, and then use XCreateCursorFromPixmap 
    //  to turn your pixmap into a cursor (this is an example of the kind of "native API" 
    //  the EGL spec is referring to). Examples more useful than this probably exist.
    //
    if (!LoadTexture("tex1.bmp", pixmap))
    {
        fprintf(stderr, "Could not load the tex1.bmp file.\n");
        return 0;
    }

    eglimage = (EGLImageKHR) eglCreateImageKHR(egl.eglDisplay, EGL_NO_CONTEXT, 
            EGL_NATIVE_PIXMAP_KHR, pixmap, attrib_list);
    
    glBindTexture(GL_TEXTURE_2D, gTexObj);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglimage);
    /* Explicit set min/mag filter to only use mipmap level 0. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return (GL_NO_ERROR == glGetError());
}

// Actual rendering here.
void Render()
{
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if(switchPixmapData)
    {
        if (!LoadTexture("tex2.bmp", pixmap))
        {
            printf("Could not load the tex2.bmp file.\n");
            return;
        }
    }
    else
    {
        if (!LoadTexture("tex1.bmp", pixmap))
        {
            printf("Could not load the tex1.bmp file.\n");
            return;
        }
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glFlush ();

    vdkSwapEGL(&egl);

}

void RenderCleanup()
{
    // cleanup
    glDisableVertexAttribArray(locVertices);
    glDisableVertexAttribArray(locTexcoord);
    DestroyShaders();
}

/***************************************************************************************
***************************************************************************************/

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
        return 0;
    }

    int length;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    fseek(fptr, 0 ,SEEK_SET);

    char * shaderSource = (char*)malloc(sizeof (char) * length);
    if (shaderSource == NULL)
    {
        fclose(fptr);
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

        GLint errorBufSize, errorLength;
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
        if (infoLog)
        {

            glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize] = '\0';
            fprintf(stderr, "%s\n", infoLog);

            free(infoLog);
        }
        return 0;
    }

    return 1;
}

/***************************************************************************************
***************************************************************************************/


void LoadShaders(const char * vShaderFName, const char * pShaderFName)
{
    vertShaderNum = glCreateShader(GL_VERTEX_SHADER);
    pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);

    if (CompileShader(vShaderFName, vertShaderNum) == 0)
    {
        printf("%d: PS compile failed.\n", __LINE__);
        return;
    }

    if (CompileShader(pShaderFName, pixelShaderNum) == 0)
    {
        printf("%d: VS compile failed.\n", __LINE__);
        return;
    }

    programHandle = glCreateProgram();

    glAttachShader(programHandle, vertShaderNum);
    glAttachShader(programHandle, pixelShaderNum);

    glLinkProgram(programHandle);
    // Check if linking succeeded.
    GLint linked = false;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        printf("%d: Link failed.\n", __LINE__);
        // Retrieve error buffer size.
        GLint errorBufSize, errorLength;
        glGetShaderiv(programHandle, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (!infoLog)
        {
            // Retrieve error.
            glGetProgramInfoLog(programHandle, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s", infoLog);

            free(infoLog);
        }

        return;
    }
    glUseProgram(programHandle);
}

// Cleanup the shaders.
void DestroyShaders()
{
    if (programHandle) {
        glDeleteShader(vertShaderNum);
        glDeleteShader(pixelShaderNum);
        glDeleteProgram(programHandle);
        glUseProgram(0);
        programHandle = 0;
    }
}


int ParseCommandLine(int argc, char * argv[])
{
    int result = 1;
    // Walk all command line arguments.
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'x':
                // x<x> for x position (defaults to -1).
                if (++i < argc)
                    posX = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'y':
                // y<y> for y position (defaults to -1).
                if (++i < argc)
                    posY = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'w':
                // w<width> for width (defaults to 0).
                if (++i < argc)
                    width = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'h':
                // h<height> for height (defaults to 0).
                if (++i < argc)
                    height = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 's':
                // s<samples> for samples (defaults to 0).
                if (++i < argc)
                    samples = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'f':
                // f<count> for number of frames (defaults to 0).
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

    for (i = 0; i < argCount; ++i)
    {
        printf("\t");
        printf("%c%c %s\t%s\n", argSpec, argNames[i], argValues[i], argDescs[i]);
    }

    for (i = 0; i < noteCount; ++i)
    {
        printf("%s\n", argNotes[i]);
    }
}


static void DoCreateEglContextReal( EGLDisplay egl_dpy, EGLConfig egl_conf, EGLContext * const ctxRet)
{
    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLContext ctx;

    ctx = eglCreateContext( egl_dpy, egl_conf, EGL_NO_CONTEXT, ctx_attribs );
    if (!ctx) {
        fprintf(stderr, " Error: eglCreateContext failed. \n");
        exit(1);
    }

    /* test eglQueryContext() */

    EGLint val;
    eglQueryContext(egl_dpy, ctx, EGL_CONTEXT_CLIENT_VERSION, &val);

    if(val != 2)
    {
        fprintf(stderr, " Error, EGL_CONTEXT_CLIENT_VERSION");
        exit(1);
    }

    *ctxRet = ctx;
}

static void EGL_PickConfig(EGLDisplay egl_dpy, EGLint * const pVid, EGLConfig * const pConfig)
{

    // These names may be passed to eglChooseConfig to specify required attribute properties.
    // All attribute names in attrib list are immediately followed by the corresponding
    // desired value. The list is terminated with EGL_NONE. If an attribute is not specified 
    // in attrib list, then the default value is used (it is said to be specified implicitly).
    // If EGL_DONT_CARE is specified as an attribute value, then the attribute will not be 
    // checked. EGL_DONT_CARE may be specified for all attributes except EGL_LEVEL and 
    // EGL_MATCH_NATIVE_PIXMAP . If attrib list is NULL or empty (first attribute is EGL_NONE ), 
    // then selection and sorting of EGLConfigs is done according to the default criteria
    //
    static const EGLint egl_attribs[] = {
        EGL_SAMPLES, 0,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE,  EGL_DONT_CARE,
        EGL_DEPTH_SIZE,   0,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };


    EGLint num_configs = 0;

    // An EGLConfig describes the format, type and size of the color buffers
    // and ancillary buffers for an EGLSurface. If the EGLSurface is a window,
    // then the EGLConfig describing it may have an associated native visual 
    // type. Names of EGLConfig attributes are EGL_BUFFER_SIZE, EGL_RED_SIZE
    // EGL_GREEN_SIZE, EGL_BLUE_SIZE, EGL_ALPHA_SIZE, EGL_DEPTH_SIZE, 
    // EGL_LEVEL, EGL_SAMPLES, EGL_STENCIL_SIZE, EGL_RENDERABLE_TYPE, etc
    // 
    // These names may be passed to eglChooseConfig to specify required 
    // attribute properties.
    //
    // EGL_CONFIG_ID is a unique integer identifying different EGLConfigs.
    // Configuration IDs must be small positive integers starting at 1 and 
    // ID assignment should be compact; that is, if there are N EGLConfigs 
    // defined by the EGL implementation, their configuration IDs should be 
    // in the range [1, N ].
    //
    // Small gaps in the sequence are allowed, but should only occur when 
    // removing configurations defined in previous revisions of an EGL implementation.
    //
    // to get EGLConfigs that match a list of attributes
    
    if( EGL_TRUE == eglGetConfigs( egl_dpy, NULL, 0, &num_configs ) )
    {
        fprintf(stdout, " Number of config : %d \n", num_configs); 
    }

    EGLConfig config;

    // to get EGLConfigs that match a list of attributes, 
    // only configurations matching attrib list will be returned.
    if ( EGL_FALSE == eglChooseConfig( egl_dpy, egl_attribs, &config, 1, &num_configs ))
    {
        fprintf(stderr, "Error: couldn't get an EGL visual config. \n");
        exit(1);
    }

    assert(config);

    if(num_configs <= 0)
    {
        fprintf(stderr, " No configuration satisfied. \n");
        exit(1);
    }

    //  handle of corresponding native visual 
    //  To get the value of an EGLConfig attribute
    //  If eglGetConfigAttrib succeeds then it returns EGL_TRUE and 
    //  the value for the specified attribute is returned in value.
    //
    if (EGL_TRUE != eglGetConfigAttrib( egl_dpy, config, EGL_NATIVE_VISUAL_ID, pVid ))
    {
        fprintf(stderr, " Error: eglGetConfigAttrib() failed. \n");
        exit(1);
    }

    *pConfig = config;
}


static Window doCreateWindowReal(Display * pDpy, int vis_id, const char * name, int x, int y, int w, int h)
{
    /* The X window visual must match the EGL config */
    int num_visuals;
    XVisualInfo visTemplate;
    // handle of corresponding native visual.
    visTemplate.visualid = vis_id;
    int scrnum = DefaultScreen( pDpy );
    Window root = RootWindow( pDpy, scrnum );

    // The XGetVisualInfo function returns a list of visual structures that
    // have attributes equal to the attributes specified by vinfo_template.
    // If no visual structures match the template using the specified vinfo_mask, 
    // XGetVisualInfo returns a NULL. To free the data returned by this function, 
    // use XFree. 
    XVisualInfo * visInfo = XGetVisualInfo(pDpy, VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo) {
        fprintf(stderr, " Error: couldn't get X visual. \n");
        exit(1);
    }
    else
    {
        fprintf(stdout, " Number of visual : %d. \n", num_visuals);
    }

    /* window attributes */
    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap( pDpy, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    Window wnd = XCreateWindow( pDpy, root, x, y, w, h,
            0, visInfo->depth, InputOutput,
            visInfo->visual, mask, &attr );


    XSizeHints sizehints;
    sizehints.x = x;
    sizehints.y = y;
    sizehints.width  = w;
    sizehints.height = h;
    sizehints.flags = USSize | USPosition;

    // The XSetNormalHints() function sets the size hints structure for the 
    // specified window. Applications use XSetNormalHints() to inform the window 
    // manager of the size or position desirable for that window.
    //
    // In addition, an application that wants to move or resize itself should call 
    // XSetNormalHints() and specify its new desired location and size as well as 
    // making direct Xlib calls to move or resize. This is because window managers 
    // may ignore redirected configure requests, but they pay attention to property 
    // changes. 
    //
    // To set size hints, an application not only must assign values to the appropriate 
    // members in the hints structure but also must set the flags member of the structure 
    // to indicate which information is present and where it came from. A call to 
    // XSetNormalHints() is meaningless, unless the flags member is set to indicate 
    // which members of the structure have been assigned values.
    XSetWMNormalHints( pDpy, wnd, &sizehints);

    // The XSetStandardProperties() function provides a means by which simple 
    // applications set the most essential properties with a single call. 
    //
    // XSetStandardProperties() should be used to give a window manager some information 
    // about your program's preferences. It should not be used by applications that need 
    // to communicate more information than is possible with XSetStandardProperties(). 
    // (Typically, argv is the argv array of your main program.) If the strings are not 
    // in the Host Portable Character Encoding, the result is implementation dependent. 
    // XSetStandardProperties(x_dpy, win, name, name,
    //                      None, (char **)NULL, 0, &sizehints);

    XTextProperty winName;
    winName.value = (unsigned char *)name;
    winName.encoding = XA_STRING;
    winName.format = 8;
    winName.nitems = strlen(name);

    XSetWMName(pDpy, wnd, &winName);

    /*  
        XTextProperty iconName;

        iconName.value = (unsigned char *)name;
        iconName.encoding = XA_STRING;
        iconName.format = 8;
        iconName.nitems = strlen(name);
        */
    // The XSetWMProperties() convenience function provides a single programming interface
    // for setting those essential window properties that are used for communicating with 
    // other clients (particularly window and session managers). 
    //
    // If the window_name argument is non-NULL, XSetWMProperties() calls XSetWMName(),
    // which in turn, sets the WM_NAME property.
    //
    // If the icon_name argument is non-NULL, XSetWMProperties() calls XSetWMIconName(), 
    // which sets the WM_ICON_NAME property.
    //
    // If the argv argument is non-NULL, XSetWMProperties() calls XSetCommand(), 
    // which sets the WM_COMMAND property.
    // XSetWMProperties(x_dpy, win, &winName, &iconName, (char **)NULL, 0, &sizehints, NULL, NULL);

    XFree(visInfo);

    return wnd;
}


// Program entry.
int main(int argc, char** argv)
{
    // EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.

    // Parse the command line.
    if (!ParseCommandLine(argc, argv))
    {
        PrintHelp();
        return 0;
    }

    // Set multi-sampling.
    // configAttribs[1] = samples;

    // Initialize EGL, and GLES.
    // if (!vdkSetupEGL(posX, posY, width, height, configAttribs, NULL, attribListContext, &egl))
    // {
    //    return 1;
    // }


    EGLSurface egl_surf; 
    EGLContext egl_ctx;
    EGLDisplay egl_dpy;
    Window x_win;
    Display * x_dpy;

    char * dpyName = NULL;
    x_dpy = XOpenDisplay( dpyName );
    if (!x_dpy) {
        fprintf(stderr, "Error: couldn't open display %s\n",
	     dpyName ? dpyName : getenv("DISPLAY"));
        return -1;
    }
    
    egl_dpy = eglGetDisplay(x_dpy);
    if (!egl_dpy) {
        fprintf(stderr, "Error: eglGetDisplay() failed. \n");
        return -1;
    }

    EGLint egl_major, egl_minor;
    if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
        fprintf(stderr, "Error: eglInitialize() failed. \n");
        return -1;
    }
    else
    {
        fprintf(stdout, "Major: %d, Minor: %d \n ", egl_major, egl_minor);

        const char *s;
        s = eglQueryString(egl_dpy, EGL_VERSION);
        printf("EGL_VERSION = %s\n", s);

        s = eglQueryString(egl_dpy, EGL_VENDOR);
        printf("EGL_VENDOR = %s\n", s);

        // The EGL_EXTENSIONS string describes the set of supported EGL extensions.
        // The string is zero-terminated and contains a space-separated list of 
        // extension names; extension names themselves do not contain spaces. 
        // If there are no extensions, then the empty string is returned.
        s = eglQueryString(egl_dpy, EGL_EXTENSIONS);
        printf("EGL_EXTENSIONS = %s\n", s);

        // The EGL_CLIENT_APIS string describes which client APIs are supported.
        // It is zero-terminated and contains a space-separated list of API names, 
        // which must include at least one of "OpenGL", "OpenGL_ES" or "OpenVG".
        s = eglQueryString(egl_dpy, EGL_CLIENT_APIS);
        printf("EGL_CLIENT_APIS = %s\n", s);
    }


    // Set window title and show the window.
    // vdkSetWindowTitle(egl.window, TUTORIAL_NAME);

    {
        EGLConfig egl_config;
        EGLint vid;

        EGL_PickConfig(egl_dpy, &vid, &egl_config);

        const char * name = "OpenGL ES 2.x glass ball";
        hWin = doCreateWindowReal(x_dpy, vid, name, posX, posY, width, height);
        
        /* set hints and properties */
        // SetWindowSizeAndTitle(x_dpy, hWin, name, x, y, w, h);

        // api must specify one of the supported client APIs, either EGL_OPENGL_API ,
        // EGL_OPENGL_ES_API , or EGL_OPENVG_API .
        eglBindAPI( EGL_OPENGL_ES_API );

        DoCreateEglContextReal( egl_dpy, egl_config, &egl_ctx);

        // To create an on-screen rendering surface, first create a native platform window
        // whose pixel format corresponds to the format, type, and size of the color buffers
        // required by config. On some implementations, the pixel format of the native 
        // window must match that of the EGLConfig. Other implementations may allow any
        // native window and config to correspond, even if their formats differ
        //
        // Creates an onscreen EGLSurface and returns a handle to it.
        // Any EGL context created with a compatible EGLConfig can be used to render into 
        // this surface. native window must belong to the same platform as dpy, 
        // and EGL considers the returned EGLSurface as belonging to that same platform
        // The EGL extension that defines the platform to which dpy belongs 
        // also defines the requirements for the native window parameter.
        //
        // attrib list may be NULL or empty (first attribute is EGL_NONE ), 
        // in which case all attributes assumes their default value
        egl_surf = eglCreateWindowSurface(egl_dpy, egl_config, hWin, NULL);
        // On failure eglCreatePlatformWindowSurface returns EGL_NO_SURFACE .
        if (egl_surf == EGL_NO_SURFACE) {
            fprintf(stderr, " Error: eglCreateWindowSurface failed. \n");
            exit(1);
        }
        else
        {
            // do sanity checks 
            EGLint val;
            // If the pixel format of native window does not correspond to the format,
            // type, and size of the color buffers required by config, as discussed above,
            // then an EGL_BAD_MATCH error is generated.
            eglQuerySurface(egl_dpy, egl_surf, EGL_WIDTH, &val);
            assert(val == width);
            fprintf(stdout, " Width : %d . \n", val); 

            eglQuerySurface(egl_dpy, egl_surf, EGL_HEIGHT, &val);
            assert(val == height);
            fprintf(stdout, " Height : %d . \n", val); 

            // If config does not support rendering to windows ( the EGL_SURFACE_TYPE
            // attribute does not contain EGL_WINDOW_BIT ), an EGL_BAD_MATCH error is
            // generated.
            //
            // To get the value of an EGLConfig attribute
            // If eglGetConfigAttrib succeeds then it returns EGL_TRUE and 
            // the value for the specified attribute is returned in value.
            //
            if( EGL_TRUE != eglGetConfigAttrib(egl_dpy, egl_config, EGL_SURFACE_TYPE, &val) )
            {
                fprintf(stderr, " Error: eglGetConfigAttrib : %d . \n", val); 
            }

            if( (val & EGL_WINDOW_BIT) == 0)
            {
                fprintf(stderr, " Error: config does not support rendering to windows. \n"); 
            }
        }
    }

    // vdkShowWindow(egl.window);
    XMapWindow( x_dpy, hWin );

    // load and compiler vertex/fragment shaders.
    LoadShaders("vs_es20t6.vert", "ps_es20t6.frag");

    if (programHandle != 0)
    {
        if (!RenderInit())
        {
            goto OnError;
        }
        int frameCount = 0;
        int halfFrames = (int)(frames / 2);
        unsigned int start = vdkGetTicks();
        // Main loop
        for (bool done = false; !done;)
        {
            // Get an event.
            vdkEvent event;
            if (vdkGetEvent(egl.window, &event))
            {
                // Test for Keyboard event.
                if (( event.type == VDK_KEYBOARD) && event.data.keyboard.pressed )
                {
                    // Test for key.
                    switch (event.data.keyboard.scancode)
                    {

                    case VDK_ESCAPE:
                        // Use ESCAPE to quit.
                        done = true;
                        break;
                    default:
                        break;
                    }
                }

                // Test for Close event.
                else if (event.type == VDK_CLOSE)
                {
                    done = true;
                }
            }
            else
            {
                // Render one frame if there is no event.
                Render();
                frameCount++;
                if(frameCount > halfFrames)
                {
                    switchPixmapData = !switchPixmapData;
                }
                if ((frames > 0) && (--frames == 0)) {
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

OnError:

    DestroyShaders();

    // cleanup
    vdkFinishEGL(&egl);

    return 0;
}
