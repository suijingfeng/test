
/*
*	History : 2009.02.01, Created by qizhuang.liu.
*
*	TODO	: Change the read-pixel-copy-texture to rend into texture.
*/

#include "vdk_sample_common.h"


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


int VDKS_Val_WindowsWidth = 800;
int VDKS_Val_WindowsHeight = 800;

char* VDKS_ARG0 = NULL;
/*
*	Outside Routine
*/
extern VDKS_BOOL TeapotInit();
extern void TeapotPass(GLuint uProgram);

extern VDKS_BOOL SphereInit();
extern void SpherePass();

extern VDKS_BOOL BlendInit ();
extern void BlendPass ();
/*
*	Framebuffer & Render Target.
*/

GLuint FrameBuffer = 0;
GLuint RenBufColor = 0;
GLuint RenBufDepth = 0;
GLuint TexObjBackground = 0;
/*
*	Texture object to collect by main file.
*/

extern GLuint TeapotTexUnitBackground;
extern GLuint TeapotTexUnitFrontFront;
extern GLuint TeapotTexUnitFrontBack;
extern GLuint TeapotTexUnitBackBack;
extern GLuint TeapotTexUnitBackFront;


extern GLuint TeapotTexObjBackground;
extern GLuint TeapotTexObjFrontFront;
extern GLuint TeapotTexObjFrontBack;
extern GLuint TeapotTexObjBackFront;
extern GLuint TeapotTexObjBackBack;

/*
*	Teapot Rendering Program Control
*/

extern GLuint TeapotProgramFrontFront;
extern GLuint TeapotProgramFrontBack;
extern GLuint TeapotProgramBackFront;
extern GLuint TeapotProgramBackBack;


/*
*	Save Screen.
*/

int SaveResult = 0;

VDKS_BOOL Init(void)
{
	if (VDKS_TRUE != TeapotInit())
	{
		return VDKS_FALSE;
	}

	if (VDKS_TRUE != SphereInit())
	{
		return VDKS_FALSE;
	}

	if (VDKS_TRUE != BlendInit())
	{
		return VDKS_FALSE;
	}

	return VDKS_TRUE;
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
        EGL_ALPHA_SIZE,   8,
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



VDKS_BOOL SaveScreenMakeTexture(char* Name, GLuint TexUnit, GLuint TexObj)
{
	unsigned int size = sizeof(unsigned char) * VDKS_Val_WindowsWidth * VDKS_Val_WindowsHeight * 4;
	unsigned char * rgba = malloc(size);
	if (NULL == rgba)
	{
		fprintf(stderr, "Error: out-of-memory.\n");
		return VDKS_FALSE;
	}

	glReadPixels(
		0, 0,
		VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		rgba);

	VDKS_Macro_CheckGLError();  /////

	if (TexObj)
	{
		/*
		*	Make texture.
		*/

		glActiveTexture(GL_TEXTURE0 + TexUnit);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, TexObj);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, 
                        0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

		glActiveTexture(GL_TEXTURE0);

	}

	if (SaveResult)
	{
		/*
		*	This will change "rgba'.
		*/
		char szBMPFile[MAX_PATH + 1];
		VDKS_Func_GetCurrentDir(szBMPFile);
		strcat(szBMPFile, "vdksample1_es20_");
		strcat(szBMPFile, Name);
		VDKS_Func_SaveBmp(szBMPFile, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, rgba);
	}

	free(rgba);

	return VDKS_TRUE;
}


int main(int argc, char * argv[])
{
	int i = 0;
	int count = 2000;

	/*
	 * Miscellaneous
	*/
	if (argc == 2)
	{
		count = atoi(argv[1]);
	}
	else if(argc == 4)
	{
		count = atoi(argv[1]);
		VDKS_Val_WindowsWidth = atoi(argv[2]);
		VDKS_Val_WindowsHeight = atoi(argv[3]);
	}
	else if(argc == 5)
	{
		count = atoi(argv[1]);
		VDKS_Val_WindowsWidth = atoi(argv[2]);
		VDKS_Val_WindowsHeight = atoi(argv[3]);
		SaveResult = atoi(argv[4]);;
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
        

        {
            EGLConfig egl_config;
            EGLint vid;

            EGL_PickConfig(egl_dpy, &vid, &egl_config);

            const char * name = "OpenGL ES 2.x glass ball";
            hWin = doCreateWindowReal(x_dpy, vid, name, 100, 100, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight);

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
                assert(val == VDKS_Val_WindowsWidth);
                fprintf(stdout, " Width : %d . \n", val); 

                eglQuerySurface(egl_dpy, egl_surf, EGL_HEIGHT, &val);
                assert(val == VDKS_Val_WindowsHeight);
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

        XMapWindow( x_dpy, hWin );

        if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
            fprintf(stderr, "Error: eglMakeCurrent() failed. \n");
            return -1;
        }


	/*
	 * App Data
	*/

	if (VDKS_TRUE != Init())
	{
		fprintf(stderr, "main : Failed to init case.\n");
		return 1;
	}

        for(i = 0; i < count; ++i)
        {
            /* Background */

            SpherePass();

            // SaveScreenMakeTexture( "Background.bmp", TeapotTexUnitBackground, TeapotTexObjBackground);

            // FrontFront

            TeapotPass(TeapotProgramFrontFront);

            // SaveScreenMakeTexture( "FrontFront.bmp", TeapotTexUnitFrontFront, TeapotTexObjFrontFront);

            // FrontBack

            TeapotPass(TeapotProgramFrontBack);

            // SaveScreenMakeTexture( "FrontBack.bmp", TeapotTexUnitFrontBack, TeapotTexObjFrontBack);

            // BackFront

            TeapotPass(TeapotProgramBackFront);

            // SaveScreenMakeTexture( "BackFront.bmp", TeapotTexUnitBackFront, TeapotTexObjBackFront);

            // BackBack 

            TeapotPass(TeapotProgramBackBack);

            // SaveScreenMakeTexture("BackBack.bmp", TeapotTexUnitBackBack, TeapotTexObjBackBack);

            // Blend 

            BlendPass();

            // SaveScreenMakeTexture("result.bmp", 0, 0);

            // VdkEgl.eglSwapBuffers(VdkEgl.eglDisplay, VdkEgl.eglSurface);
            // swap display with drawn surface.
            eglSwapBuffers(egl_dpy, egl_surf);
            VDKS_Macro_CheckGLError();
        }

        eglDestroyContext( egl_dpy, egl_ctx );
        eglDestroySurface( egl_dpy, egl_surf );
        eglTerminate( egl_dpy );

        XDestroyWindow( x_dpy, hWin );
        XCloseDisplay( x_dpy );

	return 0;
}
