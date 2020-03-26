#include <wayland-client.h>
#include <wayland-egl.h>

#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include "common.h"


struct display {
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_shell *shell;
   uint32_t mask;
};

struct window {
   struct wl_surface *surface;
   struct wl_shell_surface *shell_surface;
   struct wl_callback *callback;
};

static struct display display = {0, };
static struct window window = {0, };


/**
 * announce global object
 *
 * Notify the client of global objects.
 *
 * The event notifies the client that a global object with the
 * given name is now available, and it implements the given version
 * of the given interface.
 * 
 * id: numeric name of the global object
 * interface: interface implemented by the object
 * version: interface version
 */
static void fnAddGlobalObjectAnnouncer( 
        void *data, struct wl_registry *registry, uint32_t id, 
        const char *interface, uint32_t version )
{
    struct display * pD = (struct display *) data;

    if (strcmp(interface, "wl_compositor") == 0)
    {
        pD->compositor = wl_registry_bind( registry, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        pD->shell = wl_registry_bind( registry, id, &wl_shell_interface, 1);
    }
    
    // A registry object has a string name, the interface and an integer id, the id.
    // The id is what is used in further calls, while the interface allows the program
    // to work out which registry object it is. 
    //
    // A proxy for a registry object is obtained by binding the id to a suitable
    // data type (such as wl_compositor_interface). 

    printf( "Id: %d, interdace: %s, version: %d \n", id, interface, version);
}

/**
 * announce removal of global object
 *
 * Notify the client of removed global objects.
 *
 * This event notifies the client that the global identified by
 * name is no longer available. If the client bound to the global
 * using the bind request, the client should now destroy that
 * object.
 *
 * The object remains valid and requests to the object will be
 * ignored until the client destroys it, to avoid races between the
 * global going away and a client sending a request to it.
 * name: numeric name of the global object
 */
static void fnRemoveGlobalObjectAnnouncer(
        void *data, struct wl_registry *registry, uint32_t name)
{
    printf( "Remove Global Object: %d \n", name);
}

// The registry emits an event every time the server adds or removes a global. 
// Listening to these events is done by providing an implementation of a 
// wl_registry_listener. 
static const struct wl_registry_listener sc_registry_listener = {
    fnAddGlobalObjectAnnouncer,
    fnRemoveGlobalObjectAnnouncer
};


// Notify the client when the related request is done.
// serial: request-specific data for the callback
static void fnDoneCB(void *data, struct wl_callback * pCallback, uint32_t serial)
{

    int * pDone = (int *) data;

    printf( "fnDoneCB : %d \n", *pDone );
    
    *pDone = 1;

    printf( "fnDoneCB : %d \n", serial );

    wl_callback_destroy( pCallback );
}

static const struct wl_callback_listener sc_sync_listener = {
    fnDoneCB
};


////////////////////////////////

static struct eglut_state _eglut_state = {
   .window_width = 300,
   .window_height = 300,
   .frame_sync = 1
};

struct eglut_state *_eglut = &_eglut_state;
////////////////////////////////


struct wl_display * WL_InitDisplay(void)
{

    // wl_display_connect connects to a Wayland socket that was previously 
    // opened by a Wayland server. The server socket must be placed in 
    // XDG_RUNTIME_DIR when WAYLAND_DISPLAY (or name, see below) is a simple 
    // name, for this function to find it. The server socket is also allowed 
    // to exist at an arbitrary path; 
    //
    // The name argument specifies the name of the socket or NULL to use the 
    // default (which is "wayland-0"). The environment variable WAYLAND_DISPLAY
    // replaces the default value. If name is an absolute path, then that path
    // is used as the Wayland socket to which the connection is attempted.
    // Note that in combination with the default-value behavior described above,
    // this implies that setting WAYLAND_DISPLAY to an absolute path will 
    // implicitly cause name to take on that absolute path if name is NULL. 
    // If WAYLAND_SOCKET is set, this function behaves like 
    // wl_display_connect_to_fd with the file-descriptor number taken from the 
    // environment variable.
    //
    // Support for interpreting WAYLAND_DISPLAY as an absolute path is a change
    // in behavior compared to wl_display_connect's behavior in versions 1.14 
    // and older of Wayland. 
    //
    // It is no longer guaranteed in versions 1.15 and higher that the Wayland 
    // socket chosen is equivalent to manually constructing a socket pathname
    // by concatenating XDG_RUNTIME_DIR and WAYLAND_DISPLAY. 
    //
    // Manual construction of the socket path must account for the possibility
    // that WAYLAND_DISPLAY contains an absolute path.
    display.display = wl_display_connect(NULL);

    //
    // This establishes a connection to the Wayland server. The most important
    // role of the display, from the client perspective, is to provide the 
    // wl_registry. The registry enumerates the globals available on the server.
    //
    if (!display.display) {
          fprintf(stderr, "EGL-Wayland: failed to initialize native display. \n");
    }

    // This request creates a registry object that allows the client to list 
    // and bind the global objects available from the compositor.
    // 
    // It should be noted that the server side resources consumed in response
    // to a get_registry request can only be released when the client disconnects,
    // not when the client side proxy is destroyed.
    // 
    // Therefore, clients should invoke get_registry as infrequently as possible
    // to avoid wasting memory.
    //
    // The server has control of a number of objects. In Wayland, these are quite 
    // high-level, such as a DRM manager, a compositor, a text input manager and 
    // so on. In earlier versions of Wayland these were regarded as "global" objects,
    // but now they are accessible through a registry.
    //
    // The registry objects exist on the server. The client often needs to get
    // handles to these, as proxy objects. The first handle is to the registry itself,
    // which is done by a dedicated call, wl_display_get_registry.
    struct wl_registry * pRegistry = wl_display_get_registry( display.display );
    //
    // The registry emits an event every time the server adds or removes a global.
    // Listening to these events is done by providing an implementation of a 
    // wl_registry_listener
    //
    // Interfaces like this are used to listen to events from all kinds of resources.
    // Attaching the listener to the registry is done like this:
    //
    // Two listeners are added to this by the call wl_registry_add_listener.
    // The listeners are functions, one for new proxy objects and the other
    // to remove proxy objects. They are both wrapped up in a struct of type
    // wl_registry_listener which actually contains both functions.
    //
    // There isn't much that a client can do until it gets hold of proxies for
    // important things like the compositor. Consequently it makes sense to make
    // a blocking round-trip call to get the registry objects.
    //
    wl_registry_add_listener( pRegistry, &sc_registry_listener, &display );

    // The sync request asks the server to invoke the 'done' request on the 
    // provided wl_callback object. Since requests are handled in-order, 
    // this can be used as a barrier to ensure all previous requests have 
    // been handled. 
    struct wl_callback * pCallback = wl_display_sync( display.display );

    int done = 0;

    wl_callback_add_listener( pCallback, &sc_sync_listener, &done );
 
    while ( !done )
    {
        printf(" done = %d \n", done);
        // The default queue is dispatched by calling wl_display_dispatch().
        // This will dispatch any events queued on the default queue and attempt to
        // read from the display fd if it's empty. Events read are then queued on 
        // the appropriate queues according to the proxy assignment.
        wl_display_dispatch( display.display );
        // 
        // During the wl_display_dispatch, the global_add function is called 
        // for each global on the server. Subsequent calls to wl_display_dispatch 
        // may call global_remove when the server destroys globals. 
        //
        // The name passed into global_add is more like an ID, and identifies this 
        // resource. The interface tells you what API the resource implements, 
        // and distinguishes things like a wl_output from a wl_seat. 
        // The API these resources implement are described with XML files like this:
        //
        // <?xml version="1.0" encoding="UTF-8"?>
        // <!-- For copyright information, see https://git.io/vHyIB -->
        //
        // <protocol name="gamma_control">
        //     <interface name="gamma_control_manager" version="1">
        //         <request name="destroy" type="destructor"/>
        //             
        //         <request name="get_gamma_control">
        //             <arg name="id" type="new_id" interface="gamma_control"/>
        //             <arg name="output" type="object" interface="wl_output"/>
        //         </request>
        //     </interface>
        //
        //     <interface name="gamma_control" version="1">
        //         <enum name="error">
        //             <entry name="invalid_gamma" value="0"/>
        //         </enum>
        //
        //         <request name="destroy" type="destructor"/>
        //         
        //         <request name="set_gamma">
        //             <arg name="red" type="array"/>
        //             <arg name="green" type="array"/>
        //             <arg name="blue" type="array"/>
        //         </request>
        //         
        //         <request name="reset_gamma"/>
        //
        //         <event name="gamma_size">
        //             <arg name="size" type="uint"/>
        //         </event>
        //     </interface>
        // </protocol>
        // 
        // A typical Wayland server implementing this protocol would create a
        // gamma_control_manager global and add it to the registry. 
        //
        // The client then binds to this interface in our global_add function like so:
        //
        // #include "wayland-gamma-control-client-protocol.h"
        // ...
        // 
        // struct wl_output *example;
        // // gamma_control_manager.name is a constant: "gamma_control_manager"
        // if (strcmp(interface, gamma_control_manager.name) == 0)
        // {
        //     struct gamma_control_manager *mgr = 
        //         wl_registry_bind( registry, name, 
        //             &gamma_control_manager_interface, version);
        //     
        //     struct gamma_control *control =
        //         gamma_control_manager_get_gamma_control(mgr, example);
        //
        //     gamma_control_set_gamma(control, ...);
        // }
        // 
        // These functions are generated by running the XML file through wayland-scanner, 
        // which outputs a header and C glue code. These XML files are called “protocol 
        // extensions” and let you add arbitrary extensions to the protocol. 
        //
        // The core Wayland protocols themselves are described with similar XML files.
        //
        // Using the Wayland protocol to create a surface to display pixels with consists
        // of these steps:
        // 
        //  1. Obtain a wl_display and use it to obtain a wl_registry.
        //  2. Scan the registry for globals and grab a wl_compositor and a wl_shm_pool.
        //  3. Use the wl_compositor interface to create a wl_surface.
        //  4. Use the wl_shell interface to describe your surface’s role.
        //  5. Use the wl_shm interface to allocate shared memory to store pixels in.
        //  6. Draw something into your shared memory buffers.
        //  7. Attach your shared memory buffers to the wl_surface.
        //
        // Let’s break this down.
        //
        // The wl_compositor provides an interface for interacting with the compositor,
        // that is the part of the Wayland server that composites surfaces onto the screen.
        // It’s responsible for creating surface resources for clients to use via 
        // wl_compositor_create_surface. This creates a wl_surface resource, which you can
        // attach pixels to for the compositor to render.
        //
        // The role of a surface is undefined by default - it’s just a place to put pixels.
        // In order to get the compositor to do anything with them, you must give the
        // surface a role. Roles could be anything - desktop background, system tray, etc - 
        // but the most common role is a shell surface. 
        //
        // To create these, you take your wl_surface and hand it to the wl_shell interface.
        // You’ll get back a wl_shell_surface resource, which defines your surface’s purpose
        // and gives you an interface to do things like set the window title.
        //
        // Attaching pixel buffers to a wl_surface is pretty straightforward. There are two
        // primary ways of creating a buffer that both you and the compositor can use:
        // EGL and shared memory. EGL lets you use an OpenGL context that renders directly
        // on the GPU with minimal compositor involvement (fast) and shared memory (via wl_shm)
        // allows you to simply dump pixels in memory and hand them to the compositor (flexible).
        // There are many other Wayland interfaces I haven’t covered, giving you everything
        // from input devices (via wl_seat) to clipboard access (via wl_data_source),
        // plus many protocol extensions.
        //
        // Before we wrap this article up, let’s take a brief moment to discuss the server.
        // Most of the concepts here are already familiar to you by now. The Wayland server
        // also utilizes a wl_display, but differently from the client. The display on the
        // server has ownership over the event loop, via wl_event_loop. The event loop of a
        // Wayland server might look like this:
        //
        // struct wl_display *display = wl_display_create();
        //  ...
        // struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
        // 
        // while (true) {
        //     wl_event_loop_dispatch(event_loop, 0);
        // }
        // 
        // The event loop has a lot of helpful utilities for the Wayland server to take 
        // advantage of, including internal event sources, timers, and file descriptor monitoring. 
        // Before starting the event loop the server is going to start obtaining its own resources
        // and creating Wayland globals for them with wl_global_create:
        //
        // struct wl_global *global = wl_global_create(
        //     display, &wl_output_interface, 
        //     1 ,
        //     our_data, wl_output_bind );
        //
        // The wl_output_bind function here is going to be called when a client attempts to bind
        // to this resource via wl_registry_bind, and will look something like this:
        //
        // void wl_output_bind( struct wl_client *client, void *our_data, uint32_t their_version, 
        //      uint32_t id )
        // {
        //     struct wl_resource *resource = wl_resource_create_checked(
        //           client, wl_output_interface, their_version, our_version, id );
        //     // ...send output modes or whatever else you need to do
        // }
        // 
        // Some of the resources a server is going to be managing might include:
        //
        //
        //  DRM state for direct access to outputs
        //  GLES context (or another GL implementation) for rendering
        //  libinput for input devices
        //  udev for hotplugging
        // 
    }

    wl_registry_destroy( pRegistry );

    return display.display;
}


static void wl_draw(void *data, struct wl_callback *callback, uint32_t time);


static const struct wl_callback_listener sc_frame_listener = {
   wl_draw
};


extern void gears_idle(void);
extern void gears_draw(void);

static void wl_draw(void *data, struct wl_callback * pCallback, uint32_t time)
{

    struct window * pWindow = (struct window *)data;

    if ( pCallback ) {
        wl_callback_destroy( pCallback );
        pWindow->callback = NULL;
    }

    gears_draw();

    // Request notification when the next frame is displayed. Useful for throttling
    // redrawing operations, and driving animations. The notification will only be
    // posted for one frame unless requested again.
    //
    // pWindow->callback = wl_surface_frame( pWindow->surface );
    //
    // A Wayland server will render buffers. This will take some time.
    // If an attempt is made by a client to ask for a buffer to be rendered prematurely,
    // it won't happen: the request will be ignored. The server must be able to tell a 
    // client when it is ready to handle another surface commit request.
    //
    // The client needs to register a listener for the server's information. 
    // This is done by first getting a wl_callback by wl_surface_frame 
    // and then adding a listener by wl_callback_add_listener
    //
    // wl_callback_add_listener( pWindow->callback, &sc_frame_listener, pWindow );

    // struct wl_callback * pFrameCallback = wl_surface_frame( pWindow->surface );
    // wl_callback_add_listener(pFrameCallback, &frame_listener, pWindow);

    eglSwapBuffers( _eglut->dpy, _eglut->surface );
}



void WL_EventLoop(void)
{
    while ( 1 ) {
        gears_idle();

	wl_display_dispatch_pending(display.display);
	wl_draw(&window, NULL, 0);
    }
}


struct wl_egl_window * WL_CreateWindowForEgl(const char *title, int w, int h)
{
    // "A surface is a rectangular area that is displayed on the screen."
    // "It has a location, size and pixel contents. (Wayland specification)".
    // So to draw anything, we need a Wayland surface to draw into. We build
    // a surface using a compositor by the call to wl_compositor_create_surface. 
    // 
    window.surface = wl_compositor_create_surface( display.compositor );

    // Ask the compositor to create a new region.
    // A region object describes an area.
    //
    // Region objects are used to describe the opaque and input regions of a surface.
    //
    struct wl_region * pRegion = wl_compositor_create_region( display.compositor );
    wl_region_add( pRegion, 0, 0, w, h );
    wl_surface_set_opaque_region( window.surface, pRegion );
    wl_region_destroy( pRegion );

    // Surfaces can exist on many different devices, and there can be different 
    // Wayland servers for each. For servers with desktop-style interfaces, 
    // Wayland supplies a further surface, a shell surface. 
    //
    // First we have to get a proxy for a shell from the registry. A shell surface
    // is created from a shell by wl_shell_get_shell_surface. Then in order to show
    // a surface on such a device, the surface must be wrapped in a shell surface 
    // which is then set to be a toplevel surface.
    //
    window.shell_surface = wl_shell_get_shell_surface( display.shell, window.surface );
    
    // Then, when it wants to create a window, it calls wl_egl_window_create() with 
    // the surface that it wants to use along with the size to get a wl_egl_window pointer.
    
    // nearly useless ?
    wl_shell_surface_set_toplevel( window.shell_surface );
    
    return wl_egl_window_create( window.surface, w, h );
}


void eglutDestroyWindow( void )
{
    eglMakeCurrent(_eglut->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroySurface(_eglut->dpy, _eglut->surface);

    wl_egl_window_destroy( _eglut->window );

    wl_shell_surface_destroy( window.shell_surface );
    wl_surface_destroy( window.surface );

    if ( window.callback )
        wl_callback_destroy(window.callback);

    eglDestroyContext(_eglut->dpy, _eglut->context);
}

void WL_DefaultKeyboard(unsigned char key);

void WL_DefaultKeyboard(unsigned char key)
{
    if (key == 27)
    {
        eglutDestroyWindow( );

        eglTerminate( _eglut->dpy );

        wl_display_flush(_eglut->native_dpy);
        wl_display_disconnect(_eglut->native_dpy);

        exit(0);
    }
}


// The original X Window model used TCP to communicate information between
// client and server. This is, of course, a slow method of communication, 
// especially if you are on a single computer. Unix sockets can speed this
// up, but really, in this situation memory shared between client and server
// is fastest. 
//
// Unix/Linux has two primary shared memory APIs: the older Sys V model,
// with calls such as shmget, shmat, etc. This has generally fallen out of
// favour, with the current preference being for memory mapped files and
// devices with mmap.
//
// Wayland uses the file-backed shared memory model. There is no expectation
// that a disk file is actually written to, but a file descriptor for a new
// open file is passed from the client to the server. The file is just a
// temporary file created using a call such as mkstemp. 
//
// Code from Weston examples uses the directory XDG_RUNTIME_DIR. (XDG stands
// for the X Desktop Group which is now freedesktop.org). That directory is
// typically /run/user/<user-id>. The Weston examples also 'anonymise' the 
// file by unlinking its name after opening it.
//
//
// Content written to a surface's buffer will stay unchanged until overwritten
// by the client. The compositor is responsible for manipulating and displaying
// this content, so if, say, a portion of the client's window is obscured and
// then uncovered, the compositor will look after redrawing it without the
// client needing to do anything. This is different to the X Window model,
// where the client has to redraw areas 'damaged' by the windows of other clients.
//
// On the other hand, if the client redraws content in an area, then it has to
// inform the compositor of which part to redraw. The wl_surface_commit tells
// the compositor to redraw, while the wl_surface_damage tells the compositor
// which areas to redraw.
//
// In the previous example we redrew the entire window surface and consequently
// damaged the entire window surface as well. We can get an interesting effect
// by damaging a smaller rectangle each time, so that the undamaged area is not
// redrawn by the compositor.
