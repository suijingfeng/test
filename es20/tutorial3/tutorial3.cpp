/*
 * OpenGL ES 2.0 Tutorial 3
 *
 * Draws a glass sphere inside a big sphere (enviroment mapping).
 * The glass sphere shows reflection effects.
 */

#include <GLES2/gl2.h>  /* use OpenGL ES 2.x */
#include <EGL/egl.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "common.h"
#include "commonMath.h"


#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 3"

static int width   =  1280;
static int height  =  720;
static int posX    = 10;
static int posY    = 10;
static int frames  =  50000;
static int fast    =  0;
static int samples = 0;

static const int argCount = 7;
static const char argSpec = '-';
static const char argNames[] = {'x', 'y', 'w', 'h', 's', 'f', 'F'};
static const char argValues[][255] = {
	"x_coord",
	"y_coord",
	"width",
	"height",
	"samples",
	"frames",
	"fast",
};

static const char argDescs[][255] = {
	"x coordinate of the window, default is -1(screen center)",
	"y coordinate of the window, default is -1(screen center)",
	"width  of the window in pixels, default is 0(fullscreen)",
	"height of the window in pixels, default is 0(fullscreen)",
	"sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
	"frames to run, default is 0 (no frame limit, escape to exit)",
	"fast mode",
};

static const int noteCount = 1;
static const char argNotes[][255] = {
	"Exit: [ESC] or the frame count reached."
};
// Global Variables
GLuint programHandleGlass;
GLuint programHandleBgrnd;

// Generic used buffer
GLuint bufs[5];

// hold sphere data
int numVertices, numFaces, numIndices;
GLfloat* sphereVertices = NULL;
GLushort* indices = NULL;

// attribute and uniform handle.
GLint locVertices[2];
GLint locTransformMat[2];
GLint locRadius[2];
GLint locEyePos[2];
GLint locSamplerCb[2];

// render state
State renderState;

D3DXMATRIX transformMatrix;

/***************************************************************************************
***************************************************************************************/

void RenderInit()
{
	Sphere(&sphereVertices, 40, 60, 0, NULL, 0, NULL,
	       &indices, &numFaces, &numIndices, &numVertices);


        typedef void (*proc)();

        /* test code */
        proc p = eglGetProcAddress("glMapBufferOES");
        if( p == NULL)
        {
            fprintf(stderr, " Can not find entry point of glMapBufferOES. \n");
        }

        glClearColor(0.4, 0.4, 0.4, 0.0);

	// Grab location of shader attributes.
	locVertices[0] = glGetAttribLocation(programHandleGlass, "my_Vertex");
	locVertices[1] = glGetAttribLocation(programHandleBgrnd, "my_Vertex");

	locRadius[0] = glGetUniformLocation(programHandleGlass, "my_Radius");
	locRadius[1] = glGetUniformLocation(programHandleBgrnd, "my_Radius");

	locEyePos[0] = glGetUniformLocation(programHandleGlass, "my_EyePos");
	locEyePos[1] = glGetUniformLocation(programHandleBgrnd, "my_EyePos");

	// Transform Matrix is uniform for all vertices here.
	locTransformMat[0] = glGetUniformLocation(programHandleGlass, "my_TransformMatrix");
	locTransformMat[1] = glGetUniformLocation(programHandleBgrnd, "my_TransformMatrix");

	glEnableVertexAttribArray(locVertices[0]);
	glEnableVertexAttribArray(locVertices[1]);

	glGenBuffers(2, bufs);
        if (glGetError() != GL_NO_ERROR)
        {
		// can not handle this error
		fprintf(stderr, "GL Error.\n");
		return;
	}

	// Vertices
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(GLfloat), sphereVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(locVertices[0], 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(locVertices[1], 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);

	if (glGetError() != GL_NO_ERROR)
	{
		// can not handle this error
		fprintf(stderr, "GL Error.\n");
		return;
	}

	// Textures
	// Set s0 for samplerCube location.
	locSamplerCb[0] = glGetUniformLocation(programHandleGlass, "samplerCb");
	locSamplerCb[1] = glGetUniformLocation(programHandleBgrnd, "samplerCb");

	GLuint textureHandle;
	glGenTextures(1, &textureHandle);
	
        CubeTexture* cubeTexData = LoadCubeDDS("stpeters_cross_mipmap_256.dds");

	if (glGetError() != GL_NO_ERROR || cubeTexData == NULL)
	{
		// can not handle this error
		fprintf(stderr, "GL Error.\n");
		return;
	}

	int texSize = cubeTexData->img_size;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posx);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negx);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posy);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negy);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posz);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, texSize, texSize, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negz);
	
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
        if (glGetError() != GL_NO_ERROR)
	{
		// can not handle this error
		fprintf(stderr, "GL Error.\n");
		return;
	}

	// Deletes the texture data, it's now in OpenGL memory.
	DeleteCubeTexture(cubeTexData);

	InitializeRenderState(&renderState, width, height);

	// Set eye position.
	SetEye(&renderState, 0, 0, -5.0f);

	// Enable needed states.
	glEnable(GL_CULL_FACE);

	if (fast)
	{
	    glDisable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
	    glDepthMask(GL_FALSE);
	}
	else
	{
	    glEnable(GL_DEPTH_TEST);
	}
}


// Actual rendering here.
void doRender(void)
{
    SetupTransform(&renderState, &transformMatrix);

    if (!fast)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (fast)
    {
        // Render background sphere.
        glCullFace(GL_BACK);
        glUseProgram(programHandleBgrnd);
        glUniform1i(locSamplerCb[1], 0);
        glUniform1f(locRadius[1], 10.0f);
        glUniform3fv(locEyePos[1], 1, (GLfloat *) &renderState.m_EyeVector);
        glUniformMatrix4fv(locTransformMat[1], 1, GL_FALSE, (GLfloat *) &transformMatrix);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }

    // Render mirror ball.
    glCullFace(GL_FRONT);
    glUseProgram(programHandleGlass);
    glUniform1i(locSamplerCb[0], 0);
    glUniform1f(locRadius[0], 1.0f);
    glUniform3fv(locEyePos[0], 1, (GLfloat *) &renderState.m_EyeVector);
    glUniformMatrix4fv(locTransformMat[0], 1, GL_FALSE, (GLfloat *) &transformMatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);

    if (!fast)
    {
        // Render background sphere.
        glCullFace(GL_BACK);
        glUseProgram(programHandleBgrnd);
        glUniform1i(locSamplerCb[1], 0);
        glUniform1f(locRadius[1], 10.0f);
        glUniform3fv(locEyePos[1], 1, (GLfloat *) &renderState.m_EyeVector);
        glUniformMatrix4fv(locTransformMat[1], 1, GL_FALSE, (GLfloat *) &transformMatrix);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }
}



/***************************************************************************************
***************************************************************************************/

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

			case 'F':
				// Turn on Fast mode.
				fast = 1;
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
//        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   24,
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



void DoCreateEglContextReal( EGLDisplay egl_dpy, EGLConfig egl_conf, EGLContext * const ctxRet)
{
    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLContext ctx;

    // To create a rendering context for the current rendering API.
    // If eglCreateContext succeeds, it initializes the context to the 
    // initial state defined for the current rendering API, and returns 
    // a handle to it. The context can be used to render to any compatible 
    // EGLSurface.
    //
    // Although contexts are specific to a single client API, all contexts 
    // created in EGL exist in a single namespace. This allows many EGL calls 
    // which manage contexts to avoid use of the current rendering API.
    //
    // If share context is not EGL_NO_CONTEXT , then all shareable data, 
    // as defined by the client API (note that for OpenGL and OpenGL ES, 
    // shareable data excludes texture objects named 0) will be shared 
    // by share context, all other contexts share context already shares 
    // with, and the newly created context. An arbitrary number of EGLContexts 
    // can share data in this fashion. The OpenGL and OpenGL ES server context 
    // state for all sharing contexts must exist in a single address space.
    //
    // attrib list specifies a list of attributes for the context. The list 
    // has the same structure as described for eglChooseConfig. If an attribute 
    // is not specified in attrib list, then the default value specified below 
    // is used instead. Most attributes are only meaningful for specific client 
    // APIs, and will generate an error when specified for other types of contexts
    //
    ctx = eglCreateContext( egl_dpy, egl_conf, EGL_NO_CONTEXT, ctx_attribs );
    if (!ctx) {
        fprintf(stderr, " Error: eglCreateContext failed. \n");
        exit(1);
    }

    /* test eglQueryContext() */

    EGLint val;
    // To obtain the value of context attributes
    // eglQueryContext returns in value the value of attribute for ctx.
    //
    // attribute must be set to EGL_CONFIG_ID, EGL_CONTEXT_CLIENT_TYPE, 
    // EGL_CONTEXT_CLIENT_VERSION , or EGL_RENDER_BUFFER .
    //
    // Querying EGL_CONFIG_ID returns the ID of the EGLConfig with respect 
    // to which the context was created.
    //
    // Querying EGL_CONTEXT_CLIENT_TYPE returns the type of client API this
    // context supports (the value of the api parameter to eglBindAPI).
    //
    // Querying EGL_CONTEXT_CLIENT_VERSION returns the version of the client
    // API this context actually supports (which may differ from the version 
    // specified at context creation time). The resulting value is only 
    // meaningful for an OpenGL ES context.
    //
    // Querying EGL_RENDER_BUFFER returns the buffer which client API rendering
    // via this context will use. The value returned depends on properties of 
    // both the context, and the draw surface to which the context is bound:
    //
    // If the context is bound to a pixmap surface, 
    // then EGL_SINGLE_BUFFER will be returned.
    //
    // If the context is bound to a pbuffer surface, 
    // then EGL_BACK_BUFFER will be returned.
    //
    // If the context is bound to a window surface, then either EGL_BACK_BUFFER
    // or EGL_SINGLE_BUFFER may be returned. The value returned depends on both 
    // the buffer requested by the setting of the EGL_RENDER_BUFFER property of 
    // the surface (which may be queried by calling eglQuerySurface ), and on 
    // the client API (not all client APIs support single-buffer rendering to 
    // window surfaces). Some client APIs allow control of whether rendering 
    // goes to the front or back buffer. This client API-specific choice is not 
    // reflected in the returned value, which only describes the buffer that will
    // be rendered to by default if not overridden by the client API.
    //
    // If the context is not bound to a surface, then EGL_NONE will be returned.
    eglQueryContext(egl_dpy, ctx, EGL_CONTEXT_CLIENT_VERSION, &val);

    if(val != 2)
    {
        fprintf(stderr, " Error, EGL_CONTEXT_CLIENT_VERSION");
        exit(1);
    }

    *ctxRet = ctx;
}


/* new window size or exposure */
static void reshape(int width, int height)
{
    glViewport(0, 0, (GLint) width, (GLint) height);
}



int main(int argc, char** argv)
{
    bool pause = false;
    // EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.

    // Parse the command line.
    if (!ParseCommandLine(argc, argv))
    {
        PrintHelp();
        return 0;
    }

    EGLSurface egl_surf; 
    EGLContext egl_ctx; 
    Window hWin;
    
    char * dpyName = NULL;
    Display * x_dpy = XOpenDisplay( dpyName );
    
    if (!x_dpy) {
        fprintf(stderr, "Error: couldn't open display %s\n",
	     dpyName ? dpyName : getenv("DISPLAY"));
        return -1;
    }

    // A display can be obtained by calling eglGetDisplay

    EGLDisplay egl_dpy = eglGetDisplay(x_dpy);
    if (!egl_dpy) {
        fprintf(stderr, "Error: eglGetDisplay() failed. \n");
        return -1;
    }

    // EGL may be initialized on a display by calling eglInitialize
    // EGL_TRUE is returned on success, and major and minor are updated 
    // with the major and minor version numbers of the EGL implementation
    // (for example, in an EGL 1.2 implementation, the values of *major 
    // and *minor would be 1 and 2, respectively). major and minor are 
    // not updated if they are specified as NULL .

    EGLint egl_major, egl_minor;
    if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
        fprintf(stderr, "Error: eglInitialize() failed. \n");
        return -1;
    }
    else
    {
        fprintf(stderr, "Major: %d, Minor: %d \n ", egl_major, egl_minor);
    }


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


    // vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
    // Set window title and show the window.
    // vdkSetWindowTitle(egl.window, TUTORIAL_NAME);
    
    // win = create_x_window(x_dpy, egl_dpy, "OpenGL ES 2.x glass ball", 
    //        posX, posY, width, height, &egl_ctx, &egl_surf);
    
//////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////

    XMapWindow( x_dpy, hWin );

    if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
        fprintf(stderr, "Error: eglMakeCurrent() failed. \n");
        return -1;
    }

    // load and compile vertex/fragment shaders.
    GLuint vertShaderNum, pixelShaderNum, pixelShaderNum2;

    vertShaderNum  = glCreateShader(GL_VERTEX_SHADER);
    if (CompileShader("vs_es20t3.vert", vertShaderNum) == 0)
    {
        return -1;
    }

    pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);
    if (CompileShader("ps_es20t3_glass.frag", pixelShaderNum) == 0)
    {
        fprintf(stderr, "Error: Compile Shaderps_es20t3_glass.frag. \n");
        return -1;
    }


    pixelShaderNum2 = glCreateShader(GL_FRAGMENT_SHADER);
    if (CompileShader("ps_es20t3_bgrnd.frag", pixelShaderNum2) == 0)
    {
        fprintf(stderr, "Error: Compile Shader ps_es20t3_glass.frag. \n");
        return -1;
    }

    GLint linkStatus;

    programHandleGlass = glCreateProgram();
    glAttachShader(programHandleGlass, vertShaderNum);
    glAttachShader(programHandleGlass, pixelShaderNum);
    glLinkProgram(programHandleGlass);
    glGetProgramiv(programHandleGlass, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        fprintf(stderr, "Glass program failed to link.\n");
        return -1;
    }

    programHandleBgrnd = glCreateProgram();
    glAttachShader(programHandleBgrnd, vertShaderNum);
    glAttachShader(programHandleBgrnd, pixelShaderNum2);
    glLinkProgram(programHandleBgrnd);
    glGetProgramiv(programHandleBgrnd, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        fprintf(stderr, "Background program failed to link.\n");
        return -1;
    }

    if (programHandleGlass == 0 || programHandleBgrnd == 0)
    {
        return -1;
    }

    RenderInit();
    
    int frameCount = 0;
    unsigned long start = GetTimeMillis();
    bool done = false;

    // Main loop.
    while ( done == false )
    {
        // Get an event.
      //  XEvent event;
      //  The XNextEvent() function copies the first event from the event queue into 
      //  the specified XEvent structure and then removes it from the queue. 
      //  If the event queue is empty, XNextEvent() flushes the output buffer 
      //  and blocks until an event is received. 
      //  
      //  XNextEvent(x_dpy, &event);
        
       /* 
        switch( event.type )
        {
                case Expose: break;
                case ConfigureNotify:
                        reshape(event.xconfigure.width, event.xconfigure.height);
                        break;
                case KeyPress:
                {
                        int code = XLookupKeysym(&event.xkey, 0);
                        if (code == XK_space) 
                        {
                                pause = !pause;
                        }
                        else
                        {
                                char buffer[10];
                                XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
                                if(buffer[0] == 27) {
                                        done = true;
                                        break;
                                }
                        }

                } break;
                
                default : break;
        }
        */

        if (!pause)
        {
            // Render one frame if there is no event.
            doRender();

            // swap display with drawn surface.
            eglSwapBuffers(egl_dpy, egl_surf);

            frameCount++;

            // fprintf(stdout, " Rendered %d frames.\n", frameCount);
            
            if ((frames > 0) && (frameCount >= frames))
            {
                done = true;
            }
        }
    }

    glFinish();

    float duration = (GetTimeMillis() - start)/1000.0f;
    float fps = 1000.0f * frameCount / duration;

    printf("Rendered %d frames in %f milliseconds: %.2f fps\n",
            frameCount, duration, fps);

    // cleanup
    glDisableVertexAttribArray(locVertices[0]);
    glDisableVertexAttribArray(locVertices[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(2, bufs);

    if (sphereVertices != NULL)
    {
        free(sphereVertices);
    }

    if (indices != NULL)
    {
        free(indices);
    }

    // cleanup
    glDeleteProgram(programHandleGlass);
    glDeleteProgram(programHandleBgrnd);
    glUseProgram(0);

    glDeleteShader(vertShaderNum);
    glDeleteShader(pixelShaderNum);
    glDeleteShader(pixelShaderNum2);


    eglDestroyContext( egl_dpy, egl_ctx );
    eglDestroySurface( egl_dpy, egl_surf );
    eglTerminate( egl_dpy );
    
    XDestroyWindow( x_dpy, hWin );
    XCloseDisplay( x_dpy );
    return 0;
}
