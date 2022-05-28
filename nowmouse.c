#if 0
gcc=gcc
src=nowmouse.c
out=nowmouse.e
libs=-lX11\ -lXtst\ -lcurl
$gcc -o $out $src $libs
return
#else
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#define READDELAYms   1
#define DELAY(ms)     usleep(ms * 1000)
#define SRCTYPE       "http:/" "/"
#define SRCHOST       "192.168.1.208"
#define SRCPAGE       "newmouse.html"
#define NEWMOUSESRC   SRCTYPE SRCHOST "/" SRCPAGE
#define TITLEPREFIX   "<title>"
#define TITLESUFFIX   '<' // simple, no < allowed in title anyway!
#define DETAILPREFIX  "<b>"
#define DETAILSUFFIX  '<' // simpler / prevent invalid chars
#define CLICKLEFT     1
#define CLICKRIGHT    10
#define CLICKMIDDLE   100
#define LOG           printf
#define XTESTLEFTBUTTON  1

int firstread = 1;
int mousex = 0;
int mousey = 0;
int mousec = 0;
int *targetvalue = NULL;
char status = 'R';
char *matchword = TITLEPREFIX;
int matchoffset = 0;
static size_t newmouserecv(void *chunk, size_t size, size_t nmemb, void *none) {
  if (size != 1) {
    printf("EXPECTED BYTE DATA\n");
    return size * nmemb; // fake read
  }
  char *data = (char *)chunk;
  int ix = -1;
  while (++ix < nmemb) {
    if (status == 'R') {
//      printf("%c", data[ix]);
      if (data[ix] == matchword[matchoffset]) {
        if (matchword[++matchoffset] == '\0')
          { status = 'T'; }
      } else { matchoffset = 0; } // bad match / other tag
    } else if (status == 'T') {
      if (data[ix] == TITLESUFFIX)
        { status = 'D'; matchword = DETAILPREFIX; }
      else if (firstread) { printf("%c", data[ix]); }
    } else if (status == 'D') {
      if (firstread) { printf("\n"); firstread = 0; }
//      printf("%c", data[ix]);
      if (data[ix] == matchword[matchoffset]) {
        if (matchword[++matchoffset] == '\0') { status = 'N'; }
      } else { matchoffset = 0; } // bad match / other tag
    } else if (status == 'N') {
      if      (data[ix] == 'X') { targetvalue = &mousex; status = 'V'; }
      else if (data[ix] == 'Y') { targetvalue = &mousey; status = 'V'; }
      else if (data[ix] == 'C') { targetvalue = &mousec; status = 'V'; }
      else { printf("UNIDENTIFIED DETAIL: %c\n", data[ix]); status = 'Z'; }
    } else if (status == 'V') { // only reach state V with valid targetvalue
      if (data[ix] == DETAILSUFFIX) { status = 'D'; } // match more values
      else if (data[ix] >= '0' && data[ix] <= '9') {
        *targetvalue = (*targetvalue * 10) + (data[ix] - '0');
      } else if (data[ix] == '-') {
        printf("NEGATIVE VALUE!\n"); status = 'Z';
      } else if (data[ix] != ':' && data[ix] != ' ') {
        printf("INVALID VALUE CHARACTER\n"); status = 'Z';
      } // else ':' or ' ', which are ignored
    } else if (status == 'Z') { // finished reading because error
//      printf("(%c)", data[ix]);
    } else { printf("INVALID STATUS!\n"); }
  }
  return nmemb; // chunk is read, wait for next chunk
}

int main(int argc, char **argv) {
  int oldxpos = 0;
  int oldypos = 0;
  int oldclickstate = 0;
  // TODO: source current position and send with dimensions
  Display *display = XOpenDisplay(0);
  Window rootwindow = XRootWindow(display, 0);
  XSelectInput(display, rootwindow, KeyReleaseMask);
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *recv = curl_easy_init();
  if (!recv) { printf("CURL INIT ERROR\n"); }
  else {
    curl_easy_setopt(recv, CURLOPT_URL, NEWMOUSESRC);
    curl_easy_setopt(recv, CURLOPT_WRITEFUNCTION, newmouserecv);
    curl_easy_setopt(recv, CURLOPT_WRITEDATA, NULL); // no buffer
    curl_easy_setopt(recv, CURLOPT_USERAGENT, "newmouse-agent/1.0");
  }
  while (recv) {
    CURLcode result = curl_easy_perform(recv);
    if (result != CURLE_OK) {
      printf("RECV ERROR\n");
    } else if (status == 'Z') {
      printf("PARSE ERROR\n");
    } else if (status != 'D') {
      printf("DETAIL ERROR\n");
    } else {
      // check status as well
      int xpos = mousex;
      int ypos = mousey;
      int clickstate = mousec;
      if (xpos != oldxpos || ypos != oldypos) {
        printf("X: %d, Y: %d\n", mousex, mousey);
        XWarpPointer(display, None, rootwindow, 0, 0, 0, 0, xpos, ypos);
        oldxpos = xpos; oldypos = ypos;
      }
      if (clickstate != oldclickstate) {
        if ((clickstate & CLICKLEFT) != (oldclickstate & CLICKLEFT)) {
          if (clickstate & CLICKLEFT) {
            LOG("LEFT CLICK PRESSED\n");
            XTestFakeButtonEvent(display, XTESTLEFTBUTTON, True, CurrentTime);
          } else {
            LOG("LEFT CLICK RELEASED\n");
            XTestFakeButtonEvent(display, XTESTLEFTBUTTON, False, CurrentTime);
          }
        }
        if ((clickstate & CLICKRIGHT) != (oldclickstate & CLICKRIGHT)) {
          if (clickstate & CLICKRIGHT) {
            LOG("RIGHT CLICK PRESSED\n");
          } else { LOG("RIGHT CLICK RELEASED\n"); }
        }
        if ((clickstate & CLICKMIDDLE) != (oldclickstate & CLICKMIDDLE)) {
          if (clickstate & CLICKMIDDLE) {
            LOG("MIDDLE CLICK PRESSED\n");
          } else { LOG("MIDDLE CLICK RELEASED\n"); }
        }
        oldclickstate = clickstate;
      }
      XFlush(display);
      targetvalue = NULL;
      mousex = mousey = mousec = 0;
      matchword = TITLEPREFIX;
      status = 'R'; // ready for next read
      DELAY(READDELAYms);
    }
  }
  if (recv) { curl_easy_cleanup(recv); }
  curl_global_cleanup();
  XCloseDisplay(display);
  return 0;
}
#endif
