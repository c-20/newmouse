# newmouse
forward /dev/input/mice from a linux CLI to a linux X11


newmouse.c reads /dev/input/mice and writes a html file. that html file should be at /dev/shm/newmouse.html, and a symbolic link must be set in the apache http directory, so that the client can access the file using libcurl. not using /dev/shm will result in endless slow writes and probably eventual disk damage/failure.


performance is less than perfect. future choices:
- try libmicrohttpd for the newmouse server
- use io (cgi-bin) style setup instead of writing a html file
- use sockets directly


apparently Chrome ignores XSendEvent

the alternative, XTestFakeButtonEvent, requires libxtst-dev and -lXtst at compile

. nowmouse.c builds nowmouse.e

./nowmouse.e runs the receiver, and controls the X11 mouse

newmouse.c is compiled using cmdgcc / cmdo, which are not yet available

newmouse.c also uses code.h, which can be found in other repositories

alternatively, just re#define the keywords used and compile manually
