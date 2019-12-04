/*
 * =====================================================================================
 *
 *       Filename:  egl_pixmap.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/03/2019 03:25:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sui Jingfeng , suijingfeng@loongson.com
 *   Organization:  CASIA(2014-2017)
 *
 * =====================================================================================
 */
// main.c
// cc main.c -lX11 -lEGL -lGL
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>

void die(const char * errstr, ...)
{
    va_list ap;
    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(1);
}

int main(int argc, char * argv[])
{
    Display * display = XOpenDisplay(NULL);
    if (!display) 
        die("Can't create xlib display.\n");
    
    int screen = XDefaultScreen(display);
    
    GC gc = XDefaultGC(display, screen);
    
    Window root_window = XRootWindow(display, screen);
    unsigned long black_pixel = XBlackPixel(display, screen);
    unsigned long white_pixel = XWhitePixel(display, screen);
    Window window = XCreateSimpleWindow(display, root_window, 
            0, 0, 640, 480, 0, black_pixel, white_pixel);
    
    if (!window)
        die("Can't create window.\n");
    
    int res = XSelectInput(display, window, ExposureMask);

    if (!res) 
        die("XSelectInput failed.\n");
    
    Pixmap pixmap = XCreatePixmap(display, window, 400, 400, 24);
    
    if (!pixmap)
        die("Can't create pixmap.\n");
    
    EGLDisplay egldisplay = eglGetDisplay(display);
    
    if (EGL_NO_DISPLAY == egldisplay)
        die("Can't cate egl display.\n");
    
    res = eglInitialize(egldisplay, NULL, NULL);
    
    if (!res)
        die("eglInitialize failed.\n");
    
    EGLConfig config;
    int num_configs;
    
    static int attrib_list[] = {
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         0,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE,       EGL_PIXMAP_BIT,
        EGL_NONE
    };

    res = eglChooseConfig(egldisplay, attrib_list, &config, 1, &num_configs);
    
    if (!res)
        die("eglChooseConfig failed.\n");
    if (0 == num_configs)
        die("No appropriate egl config found.\n");
    
    EGLSurface surface =
        eglCreatePixmapSurface(egldisplay, config, pixmap, NULL);
    
    if (EGL_NO_SURFACE == surface)
        die("Can't create egl pixmap surface.\n");
    
    res = eglBindAPI( EGL_OPENGL_ES_API );
    
    if (!res)
        die("eglBindApi failed.\n");
    
    EGLContext context = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, NULL);
    
    if (EGL_NO_CONTEXT == config) die("Can't create egl context.\n");
    
    res = eglMakeCurrent(egldisplay, surface, surface, context);
    
    if (!res)
        die("eglMakeCurrent failed.\n");
    
    res = XMapWindow(display, window);
    
    if (!res) 
        die("XMapWindow failed.\n");
    
    while (1)
    {
        XEvent event;
        res = XNextEvent(display, &event);
        if (Expose != event.type)
            continue;
        
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glFinish();
        
        res = eglWaitGL();
        if (!res)
            die("eglWaitGL failed.\n");
        
        res = XCopyArea(display, pixmap, window, gc, 0, 0, 400, 400, 0, 0);
        
        if (!res)
            die("XCopyArea failed.\n");
    }
}
