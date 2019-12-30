/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 **************************************************************************/

/*
 * Draw a triangle with X/EGL and OpenGL ES 2.x
 */

#define USE_FULL_GL 1


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include <GLES2/gl2.h>  /* use OpenGL ES 2.x */
#include <EGL/egl.h>


#define FLOAT_TO_FIXED(X)   ((X) * 65535.0)



static GLfloat view_rotx = 0.0, view_roty = 0.0;

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;


static void
make_z_rot_matrix(GLfloat angle, GLfloat *m)
{
   float c = cos(angle * M_PI / 180.0);
   float s = sin(angle * M_PI / 180.0);
   int i;
   for (i = 0; i < 16; i++)
      m[i] = 0.0;
   m[0] = m[5] = m[10] = m[15] = 1.0;

   m[0] = c;
   m[1] = s;
   m[4] = -s;
   m[5] = c;
}

static void make_scale_matrix(GLfloat xs, GLfloat ys, GLfloat zs, GLfloat *m)
{
   unsigned int i;
   for (i = 0; i < 16; ++i)
      m[i] = 0.0;

   m[0] = xs;
   m[5] = ys;
   m[10] = zs;
   m[15] = 1.0;
}


static void mul_matrix(GLfloat *prod, const GLfloat *a, const GLfloat *b)
{
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  p[(col<<2)+row]
   GLfloat p[16];
   unsigned int i;
   for (i = 0; i < 4; ++i) {
      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
   memcpy(prod, p, sizeof(p));
#undef A
#undef B
#undef PROD
}


static void
draw(void)
{
   static const GLfloat verts[3][2] = {
      { -1, -1 },
      {  1, -1 },
      {  0,  1 }
   };
   static const GLfloat colors[3][3] = {
      { 1, 0, 0 },
      { 0, 1, 0 },
      { 0, 0, 1 }
   };
   GLfloat mat[16], rot[16], scale[16];

   /* Set modelview/projection matrix */
   make_z_rot_matrix(view_rotx, rot);
   make_scale_matrix(0.5, 0.5, 0.5, scale);
   mul_matrix(mat, rot, scale);
   glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   {
      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
      glEnableVertexAttribArray(attr_pos);
      glEnableVertexAttribArray(attr_color);

      glDrawArrays(GL_TRIANGLES, 0, 3);

      glDisableVertexAttribArray(attr_pos);
      glDisableVertexAttribArray(attr_color);
   }
}


/* new window size or exposure */
static void reshape(int width, int height)
{
    glViewport(0, 0, (GLint) width, (GLint) height);
}


static void create_shaders(void)
{
   static const char *fragShaderText =
      "precision mediump float;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_FragColor = v_color;\n"
      "}\n";

   static const char *vertShaderText =
      "uniform mat4 modelviewProjection;\n"
      "attribute vec4 pos;\n"
      "attribute vec4 color;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_Position = modelviewProjection * pos;\n"
      "   v_color = color;\n"
      "}\n";

   GLuint fragShader, vertShader, program;
   GLint stat;

   fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: fragment shader did not compile!\n");
      exit(1);
   }

   vertShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
   glCompileShader(vertShader);
   glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: vertex shader did not compile!\n");
      exit(1);
   }

   program = glCreateProgram();
   glAttachShader(program, fragShader);
   glAttachShader(program, vertShader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &stat);
   if (!stat) {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(program, 1000, &len, log);
      printf("Error: linking:\n%s\n", log);
      exit(1);
   }

   glUseProgram(program);

   if (1) {
      /* test setting attrib locations */
      glBindAttribLocation(program, attr_pos, "pos");
      glBindAttribLocation(program, attr_color, "color");
      glLinkProgram(program);  /* needed to put attribs into effect */
   }
   else {
      /* test automatic attrib locations */
      attr_pos = glGetAttribLocation(program, "pos");
      attr_color = glGetAttribLocation(program, "color");
   }

   u_matrix = glGetUniformLocation(program, "modelviewProjection");
   printf("Uniform modelviewProjection at %d\n", u_matrix);
   printf("Attrib pos at %d\n", attr_pos);
   printf("Attrib color at %d\n", attr_color);
}


static void init(void)
{
   typedef void (*proc)();

   /* test code */
   proc p = eglGetProcAddress("glMapBufferOES");
   if( p == NULL)
   {
       fprintf(stderr, " Can not find entry point of glMapBufferOES. \n");
   }

   glClearColor(0.4, 0.4, 0.4, 0.0);

   create_shaders();
}


/*
 * Create an RGB, double-buffered X window.
 * Return the window and context handles.
 */
static void make_x_window( Display *x_dpy, EGLDisplay egl_dpy,
              const char *name, int x, int y, int width, int height,
              Window *winRet,  EGLContext *ctxRet, EGLSurface *surfRet )
{
    // These names may be passed to eglChooseConfig to specify required attribute properties.
    static const EGLint attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };


    EGLContext ctx;
    EGLConfig config;
    EGLint vid;


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
    // in the range [1, N].
    //
    // Small gaps in the sequence are allowed, but should only occur when 
    // removing configurations defined in previous revisions of an EGL implementation.
    //
    // to get EGLConfigs that match a list of attributes
    EGLint num_configs = 0;
    
    if( EGL_TRUE == eglGetConfigs(egl_dpy, NULL, 0, &num_configs) )
    {
        fprintf(stdout, "number of config : %d \n", num_configs); 
    }

    if (EGL_FALSE == eglChooseConfig( egl_dpy, attribs, &config, 1, &num_configs ))
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
    if (EGL_TRUE != eglGetConfigAttrib( egl_dpy, config, EGL_NATIVE_VISUAL_ID, &vid ))
    {
        fprintf(stderr, " Error: eglGetConfigAttrib() failed. \n");
        exit(1);
    }
    

    Window win;

    {
        /* The X window visual must match the EGL config */
        int num_visuals;
        XVisualInfo visTemplate;
        // handle of corresponding native visual.
        visTemplate.visualid = vid;
        int scrnum = DefaultScreen( x_dpy );
        Window root = RootWindow( x_dpy, scrnum );

        // The XGetVisualInfo function returns a list of visual structures that
        // have attributes equal to the attributes specified by vinfo_template.
        // If no visual structures match the template using the specified vinfo_mask, 
        // XGetVisualInfo returns a NULL. To free the data returned by this function, 
        // use XFree. 
        XVisualInfo * visInfo = XGetVisualInfo(x_dpy, VisualIDMask, &visTemplate, &num_visuals);
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
        attr.colormap = XCreateColormap( x_dpy, root, visInfo->visual, AllocNone);
        attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
        unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

        win = XCreateWindow( x_dpy, root, 0, 0, width, height,
                0, visInfo->depth, InputOutput,
                visInfo->visual, mask, &attr );

        XFree(visInfo);
    }

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = x;
        sizehints.y = y;
        sizehints.width  = width;
        sizehints.height = height;
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
        XSetWMNormalHints(x_dpy, win, &sizehints);

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

        XSetWMName(x_dpy, win, &winName);

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
    }


    // api must specify one of the supported client APIs, either EGL_OPENGL_API ,
    // EGL_OPENGL_ES_API , or EGL_OPENVG_API .
    eglBindAPI(EGL_OPENGL_ES_API);

    // To create a rendering context for the current rendering API.
    // If eglCreateContext succeeds, it initializes the context to the initial state
    // defined for the current rendering API, and returns a handle to it. The context 
    // can be used to render to any compatible EGLSurface.
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
    ctx = eglCreateContext(egl_dpy, config, EGL_NO_CONTEXT, ctx_attribs );
    if (!ctx) {
        fprintf(stderr, " Error: eglCreateContext failed. \n");
        exit(1);
    }

   /* test eglQueryContext() */
    {
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
        assert(val == 2);
    }


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
    EGLSurface ret_suf = eglCreateWindowSurface(egl_dpy, config, win, NULL);
    // On failure eglCreatePlatformWindowSurface returns EGL_NO_SURFACE .
    if (ret_suf == EGL_NO_SURFACE) {
        fprintf(stderr, " Error: eglCreateWindowSurface failed. \n");
        exit(1);
    }

   
    /* sanity checks */
    {
        EGLint val;
        // If the pixel format of native window does not correspond to the format,
        // type, and size of the color buffers required by config, as discussed above,
        // then an EGL_BAD_MATCH error is generated.
        eglQuerySurface(egl_dpy, ret_suf, EGL_WIDTH, &val);
        assert(val == width);
        
        eglQuerySurface(egl_dpy, ret_suf, EGL_HEIGHT, &val);
        assert(val == height);
        
        // If config does not support rendering to windows ( the EGL_SURFACE_TYPE
        // attribute does not contain EGL_WINDOW_BIT ), an EGL_BAD_MATCH error is
        // generated.
        //
        // To get the value of an EGLConfig attribute
        // If eglGetConfigAttrib succeeds then it returns EGL_TRUE and 
        // the value for the specified attribute is returned in value.
        //
        if(EGL_TRUE != eglGetConfigAttrib(egl_dpy, config, EGL_SURFACE_TYPE, &val) )
        {
            fprintf(stderr, " Error: eglGetConfigAttrib : %d . \n", val); 
        }

        if( (val & EGL_WINDOW_BIT) == 0)
        {
            fprintf(stderr, " Error: config does not support rendering to windows. \n"); 
        }
    }

    *surfRet = ret_suf;
    *winRet = win;
    *ctxRet = ctx;
}


static void event_loop(Display *dpy, Window win, EGLDisplay egl_dpy, EGLSurface egl_surf)
{
   while (1) {
      int redraw = 0;
      XEvent event;

      XNextEvent(dpy, &event);

      switch (event.type) {
      case Expose:
         redraw = 1;
         break;
      case ConfigureNotify:
         reshape(event.xconfigure.width, event.xconfigure.height);
         break;
      case KeyPress:
         {
            char buffer[10];
            int code;
            code = XLookupKeysym(&event.xkey, 0);
            if (code == XK_Left) {
               view_roty += 5.0;
            }
            else if (code == XK_Right) {
               view_roty -= 5.0;
            }
            else if (code == XK_Up) {
               view_rotx += 5.0;
            }
            else if (code == XK_Down) {
               view_rotx -= 5.0;
            }
            else {
               XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
               if (buffer[0] == 27) {
                  /* escape */
                  return;
               }
            }
         }
         redraw = 1;
         break;
      default:
         ; /*no-op*/
      }

      if (redraw) {
         draw();
         eglSwapBuffers(egl_dpy, egl_surf);
      }
   }
}


static void usage(void)
{
   printf("Usage:\n");
   printf("  -display <displayname>  set the display to run on\n");
   printf("  -info                   display OpenGL renderer info\n");
}


int main(int argc, char *argv[])
{
   const int winWidth = 640, winHeight = 480;
   Display * x_dpy;
   Window win;
   EGLSurface egl_surf;
   EGLContext egl_ctx;
   EGLDisplay egl_dpy;
   char *dpyName = NULL;
   GLboolean printInfo = GL_FALSE;
   EGLint egl_major, egl_minor;
   int i;
   const char *s;

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-display") == 0) {
            dpyName = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-info") == 0) {
            printInfo = GL_TRUE;
        }
        else {
             usage();
             return -1;
        }
    }

    x_dpy = XOpenDisplay(dpyName);

    if (!x_dpy) {
        fprintf(stderr, "Error: couldn't open display %s\n",
	     dpyName ? dpyName : getenv("DISPLAY"));
        return -1;
    }

    // A display can be obtained by calling eglGetDisplay

    egl_dpy = eglGetDisplay(x_dpy);
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
    if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
        fprintf(stderr, "Error: eglInitialize() failed. \n");
        return -1;
    }

    // If dpy is EGL_NO_DISPLAY , then the EGL_VERSION string describes 
    // the supported client version. If dpy is a valid, initialized display, 
    // then the EGL_VERSION string describes the supported EGL version for 
    // dpy.
    //
    // The client version indicates that all EGL entry points which are 
    // needed for the supported client APIs are available at runtime,
    // while the display version indicates which EGL functionality is
    // supported for a display.
    //
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

    make_x_window( x_dpy, egl_dpy,
                 "OpenGL ES 2.x tri", 0, 0, winWidth, winHeight,
                 &win, &egl_ctx, &egl_surf);

    XMapWindow( x_dpy, win );

    // eglMakeCurrent binds ctx to the current rendering thread and 
    // to the draw and read surfaces.
    //
    if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
        fprintf(stderr, "Error: eglMakeCurrent() failed. \n");
        return -1;
    }

    if (printInfo) {
       printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
       printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
       printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
       printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
    }

    init();

   /* Set initial projection/viewing transformation.
    * We can't be sure we'll get a ConfigureNotify event when the window
    * first appears.
    */
    reshape(winWidth, winHeight);

    event_loop( x_dpy, win, egl_dpy, egl_surf );

    eglDestroyContext( egl_dpy, egl_ctx );
    eglDestroySurface( egl_dpy, egl_surf );
    eglTerminate( egl_dpy );

    XDestroyWindow(x_dpy, win);

    XCloseDisplay(x_dpy);

    return 0;
}
