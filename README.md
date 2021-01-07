# X Events Server

A server for detecting X11 events and calling scripts.  Some window managers
have built-in support for this, but if you need this sort of behavior it is
nice to have a window-manager-independent way to do it.

This was a bit of a prototype project.  Currently it only tracks events
required for catching all changes to the title of the focused window (that is,
both focus change events an window title change events), as that is what I
needed it for.  But extending it for capturing additional events would be easy.

If you would like to capture different events, just create a feature request in
a github issue.

I may at some point rewrite this as a python server that the Xlib package.

## Configuring

By default, all events emitted by the server trigger the same executable, which
is called `x-event-hook` (it must be on the `PATH` and marked as executable).

That behavior can be modified by editing `hooks.h` before building.

## Building

Build dependencies are `gcc`, `make,` and the development library for X11 for
your system.

Just run `make`.

## Installation

Just run `sudo make install`, which will install a single binary to
`/usr/local/bin/x-event-server`.

To install somewhere else, just copy the built binary where you want it.

## Running

You will want to cause the server to start when you start an X11 session for
your user.

If you use `startx`, you can just put a line like `x-event-server &` in your
`~/.xinitrc` above where you call your window manager.

## Uninstallation

Just delete the `x-event-server` from wherever you installed it.

## License

This code is in the public domain, as described by the
[Unlicense](https://unlicense.org).
