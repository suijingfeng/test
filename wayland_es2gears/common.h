#ifndef EGLUT_H
#define EGLUT_H


#include "EGL/egl.h"



struct eglut_state {
    int window_width, window_height;

   /* initialized by native display */
    EGLNativeDisplayType native_dpy;

    EGLDisplay dpy;
    EGLint major, minor;
    EGLContext context;
    /* initialized by native display */
    EGLNativeWindowType window;
    EGLSurface surface;
};


void WL_EventLoop( void );

struct wl_egl_window * WL_CreateWindowForEgl(const char *title, int w, int h);
struct wl_display * WL_InitDisplay(void);


void gears_idle(void);
void gears_draw(void);

#endif /* EGLUT_H */
