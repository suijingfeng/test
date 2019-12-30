void WL_EventLoop(void)
{
    struct pollfd pollfd;

    pollfd.fd = wl_display_get_fd(display.display);
    pollfd.events = POLLIN;
    pollfd.revents = 0;

    while (1)
    {
        /* If we need to flush but can't, don't do anything at all which could
         * push further events into the socket. */
        if ( !(pollfd.events & POLLOUT) )
        {
           wl_display_dispatch_pending(display.display);

           gears_idle();
           /* Client wants to redraw, but we have no frame event to trigger the
            * redraw; kickstart it by redrawing immediately. */
           if ( !window.callback ) {
             wl_draw(&window, NULL, 0);
           }
        }

        int ret = wl_display_flush(display.display);
        if (ret < 0 && errno != EAGAIN)
           break; /* fatal error; socket is broken */
        else if (ret < 0 && errno == EAGAIN)
           pollfd.events |= POLLOUT; /* need to wait until we can flush */
        else
           pollfd.events &= ~POLLOUT; /* successfully flushed */

        if (poll(&pollfd, 1, -1) == -1)
           break;

        if (pollfd.revents & (POLLERR | POLLHUP))
           break;

        if (pollfd.events & POLLOUT) {
	   if (!(pollfd.revents & POLLOUT))
              continue; /* block until we can flush */
           pollfd.events &= ~POLLOUT;
        }

        if (pollfd.revents & POLLIN) {
           ret = wl_display_dispatch(display.display);
           if (ret == -1)
               break;
        }

        ret = wl_display_flush(display.display);
        if (ret < 0 && errno != EAGAIN)
           break; /* fatal error; socket is broken */
        else if (ret < 0 && errno == EAGAIN)
           pollfd.events |= POLLOUT; /* need to wait until we can flush */
        else
           pollfd.events &= ~POLLOUT; /* successfully flushed */
    }
}

