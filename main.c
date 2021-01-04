#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/* I'm choosing not to use _NET_ACTIVE_WINDOW at all because it doesn't seem to
   have well-defined behavior of when it is updated vs when focus events
   happen.  A more robust solution seems to be to just find all windows and
   watch for the appropriate FocusIn events on all of them. */
// Atom atom_net_active_window = -1;
// Window root;
//
// Window get_active_window(Display *dpy){
//     Atom actual_type;
//     int actual_format;
//     unsigned long nitems;
//     unsigned long bytes_after_return;
//     unsigned char *prop;
//
//     int ret = XGetWindowProperty(
//         dpy, root, atom_net_active_window, 0, 1, False, AnyPropertyType,
//         &actual_type, &actual_format, &nitems, &bytes_after_return, &prop
//     );
//
//     if(ret != Success){
//         fprintf(stderr, "failed to get active window\n");
//         return 0;
//     }
//
//     Window out = *((Window*)prop);
//
//     XFree(prop);
//
//     return out;
// }

char *event_type_string(int type){
    switch(type){
        case KeyPress: return "KeyPress";
        case KeyRelease: return "KeyRelease";
        case ButtonPress: return "ButtonPress";
        case ButtonRelease: return "ButtonRelease";
        case MotionNotify: return "MotionNotify";
        case EnterNotify: return "EnterNotify";
        case LeaveNotify: return "LeaveNotify";
        case FocusIn: return "FocusIn";
        case FocusOut: return "FocusOut";
        case KeymapNotify: return "KeymapNotify";
        case Expose: return "Expose";
        case GraphicsExpose: return "GraphicsExpose";
        case NoExpose: return "NoExpose";
        case VisibilityNotify: return "VisibilityNotify";
        case CreateNotify: return "CreateNotify";
        case DestroyNotify: return "DestroyNotify";
        case UnmapNotify: return "UnmapNotify";
        case MapNotify: return "MapNotify";
        case MapRequest: return "MapRequest";
        case ReparentNotify: return "ReparentNotify";
        case ConfigureNotify: return "ConfigureNotify";
        case ConfigureRequest: return "ConfigureRequest";
        case GravityNotify: return "GravityNotify";
        case ResizeRequest: return "ResizeRequest";
        case CirculateNotify: return "CirculateNotify";
        case CirculateRequest: return "CirculateRequest";
        case PropertyNotify: return "PropertyNotify";
        case SelectionClear: return "SelectionClear";
        case SelectionRequest: return "SelectionRequest";
        case SelectionNotify: return "SelectionNotify";
        case ColormapNotify: return "ColormapNotify";
        case ClientMessage: return "ClientMessage";
        case MappingNotify: return "MappingNotify";
        case GenericEvent: return "GenericEvent";
    }
    return "unknown event type";
}

char *focus_event_mode(int mode){
    switch(mode){
        case NotifyNormal: return "NotifyNormal";
        case NotifyWhileGrabbed: return "NotifyWhileGrabbed";
        case NotifyGrab: return "NotifyGrab";
        case NotifyUngrab: return "NotifyUngrab";
    }
    return "unknown";
}

char *focus_event_detail(int detail){
    switch(detail){
        case NotifyAncestor: return "NotifyAncestor";
        case NotifyVirtual: return "NotifyVirtual";
        case NotifyInferior: return "NotifyInferior";
        case NotifyNonlinear: return "NotifyNonlinear";
        case NotifyNonlinearVirtual: return "NotifyNonlinearVirtual";
        case NotifyPointer: return "NotifyPointer";
        case NotifyPointerRoot: return "NotifyPointerRoot";
        case NotifyDetailNone: return "NotifyDetailNone";
    }
    return "unknown";
}

int watch_focus_on_children(Display *dpy, Window w){
    // printf("watch_focus_on_children(%lu)\n", w);
    Window root;
    Window parent;
    Window *children;
    unsigned int nchildren;
    int ret = XQueryTree(dpy, w, &root, &parent, &children, &nchildren);
    // if(ret != Success){
    //     printf("ret=%d\n", ret);
    //     fprintf(stderr, "failed to query window %lu\n", w);
    //     return 0;
    // }
    //// for some reason I get a BadRequest code but it works if I ignore it.
    (void)ret;

    int retval = 0;

    for(int i = 0; i < nchildren; i++){
        Window child = children[i];
        // watch this window
        XSelectInput(dpy, child, FocusChangeMask);
        // recurse to its children
        ret = watch_focus_on_children(dpy, child);
        if(ret){
            retval = 1;
            break;
        }
    }

    XFree(children);

    return retval;
}

int x_error_handler(Display *dpy, XErrorEvent *xerror){
    /* since our only X calls while running are XSelectInput, and those can
       fail in a race condition where the window is destroyed... just ignore
       all errors. */

    // char buf[1024];
    // XGetErrorText(dpy, xerror->error_code, buf, sizeof(buf));
    // printf("ignoring error: %.*s\n", (int)sizeof(buf), buf);

    // this value is always ignored:
    return 0;
}

int run(Display *dpy){
    // EnterWindowMask is for mouse movements, which we don't care about.
    // SubstructureNotifyMask will detect window creations and deletions.
    // FocusChangeMask will detect window focus, but only on the window you
    // apply it to.

    int nscreens = ScreenCount(dpy);
    for(int i = 0; i < nscreens; i++){
        Window root = RootWindow(dpy, i);
        // set event mask on the root window, for tracking new windows.
        XSelectInput(dpy, root, FocusChangeMask | SubstructureNotifyMask);
        // start watching all existing windows for focus change events
        int ret = watch_focus_on_children(dpy, root);
        if(ret) return 1;
    }

    // ignore errors after we are finished initial configuration.
    XSetErrorHandler(x_error_handler);

    XEvent xev;
    XFocusChangeEvent *xfocus;
    XCreateWindowEvent *xcreate;

    while(XNextEvent(dpy, &xev) == Success){
        switch(xev.type){
            case FocusIn:
                xfocus = (XFocusChangeEvent*)&xev;
                /* Holy shit, are focus events are complicated.  It makes sense
                   since focus/unfocus operations get replayed over an entire
                   tree of windows.  For the full treatment, see:
                       www.x.org/releases/current/doc/libX11/libX11/libX11.html#Input_Focus_Events
                */
                // printf(
                //     "focus in: window=%lu, mode=%s, detail=%s\n",
                //     xfocus->window,
                //     focus_event_mode(xfocus->mode),
                //     focus_event_detail(xfocus->detail)
                // );

                /* Since we just want to catch the leaf FocusIn events, we can
                   filter by detail==NotifyNonlinear. */
                if(xfocus->detail != NotifyNonlinear) break;

                /* It seems that there is some sort of race condition (at least
                   under ratpoison) which means that the leaf FocusIn events
                   somtimes appear as mode==NotifyNormal and sometimes appear
                   as mode==NotifyWhileGrabbed.  So we just detect either. */
                if(xfocus->mode != NotifyNormal
                        && xfocus->mode != NotifyWhileGrabbed) break;
                printf("focus: %lu\n", xfocus->window);
                break;

            case CreateNotify:
                xcreate = (XCreateWindowEvent*)&xev;
                // printf("create window: %lu\n", xcreate->window);
                // watch the new window for focus changes
                XSelectInput(dpy, xcreate->window, FocusChangeMask);
                break;

            // ignore these silently
            case FocusOut:
            case DestroyNotify:
            case MapNotify:
            case UnmapNotify:
            case ConfigureNotify:
                break;
            default:
                fprintf(stderr, "unhandled %s event\n", event_type_string(xev.type));
        }
    }

    return 0;
}

int main(){
    Display *dpy = XOpenDisplay(NULL);
    if(!dpy){
        fprintf(stderr, "failed to open display\n");
        return 1;
    }

    int exitcode = run(dpy);

    XCloseDisplay(dpy);

    return exitcode;
}
