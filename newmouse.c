#include "code.h"
#include <stdio.h>
#include <fcntl.h>
#define newmouse main
#define NEWMOUSEPATH      "/dev/shm/newmouse.html"
#define NEWMOUSEWRITEPATH "/dev/shm/newmousewrite.html"
#define NEWMOUSEHEAD      "<html>\n<head>\n  <title>"
#define NEWMOUSETITLE     "NEWMOUSE: ACTIVE"
#define NEWMOUSEBODY      "</title>\n</head>\n<body>"
#define NEWMOUSESTOP      "\n</body>\n</html>\n"


VD newmousewrite(IN x, IN y, IN c) {
  FILE *newwrite = fopen(NEWMOUSEWRITEPATH, "w");
  IF (!newwrite) { LOG("NEWMOUSEWRITE OPEN FAIL\n"); RT; }
  IF (fprintf(newwrite, "%s", NEWMOUSEHEAD)  LT 0)
    { LOG("NEWMOUSEWRITE ERROR 1\n"); RT; }
  EF (fprintf(newwrite, "%s", NEWMOUSETITLE) LT 0)
    { LOG("NEWMOUSEWRITE ERROR 2\n"); RT; }
  EF (fprintf(newwrite, "%s", NEWMOUSEBODY)  LT 0)
    { LOG("NEWMOUSEWRITE ERROR 3\n"); RT; }
  EF (fprintf(newwrite, "<b>X: %d</b><br />\n", x) LT 0)
    { LOG("NEWMOUSEWRITE ERROR 4X\n"); RT; }
  EF (fprintf(newwrite, "<b>Y: %d</b><br />\n", y) LT 0)
    { LOG("NEWMOUSEWRITE ERROR 4Y\n"); RT; }
  EF (fprintf(newwrite, "<b>C: %d</b>\n",       c) LT 0)
    { LOG("NEWMOUSEWRITE ERROR 4C\n"); RT; }
  EF (fprintf(newwrite, "%s", NEWMOUSESTOP)  LT 0)
    { LOG("NEWMOUSEWRITE ERROR 5\n"); RT; }
  EL { // write succeeded -- replace file
    IF (fclose(newwrite) NQ 0)
      { LOG("NEWMOUSEWRITE CLOSE FAIL\n"); RT; }
    EF (rename(NEWMOUSEWRITEPATH, NEWMOUSEPATH) NQ 0)
      { LOG("NEWMOUSEWRUTE REPLACE FAIL\n"); RT; }
  }
}

IN newmouse($) {
  IN mousex = 0;
  IN mousexmin = 0;
  IN mousexmax = 2000;
  IN mousey = 0;
  IN mouseymin = 0;
  IN mouseymax = 2000;
  // TODO: RECEIVE SCREEN DIMENSIONS?
  // OR SET FORWARDING REGION -- STILL NEED DIMENSIONS
  IN mousec = 0;
  CCS mice = "/dev/input/mice";
  IN micef = open(mice, O_RDWR);
  IF (micef EQ -1) {
    LOG("MICE OPEN FAIL\n");
    RT 1;
  }
  UCH data[3];
  LOOP {
    IN bytes = read(micef, data, sizeof(data));
    IF (bytes) {
      IN left   =  data[0] & 0x1;
      IN right  = (data[0] & 0x2) >> 1;
      IN middle = (data[0] & 0x4) >> 2;
      IN x      = (IN)(SCH)data[1];
      IN y      = (IN)(SCH)data[2];
      IN c = left + (right * 10) + (middle * 100);
      mousex ADDS x;
      IF (mousex LT mousexmin) { mousex = mousexmin; }
      IF (mousex GT mousexmax) { mousex = mousexmax; }
      mousey SUBS y; // y is inverted!
      IF (mousey LT mouseymin) { mousey = mouseymin; }
      IF (mousey GT mouseymax) { mousey = mouseymax; }
      mousec = c;
      newmousewrite(mousex, mousey, mousec);
      LOG4("%s XYC %d %d %d.\n", NEWMOUSEPATH, mousex, mousey, mousec);
    }
  }
}
