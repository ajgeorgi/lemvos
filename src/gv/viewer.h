#ifndef _VIEWER_
#define _VIEWER_

/* Viewer for GV 19.08.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "widget.h"
#include "aperal.h"
#include "commen.h"

#define GV_APPL_CLASS "Georgi-Bootbau"
#define GV_APPLICATION "GV"
#define GV_VERSION "0.1.3"

typedef int (*PaintMethod)(void* data);

int viewerInit(int argc, char **argv, CommonErrorHandler error_handler);

int viewerSetDrawingColor(color_type color);
color_type viewerGetDrawingColor();

int viewerDrawText3D(EPoint *p, const char* text, size_t length);
int viewerDrawLine3D(const EPoint *p1, const EPoint *p2);

int viewerDrawLine(int x1, int y1, int x2, int y2);

#define VIEWER_TEXT_STICK_RIGHT (-1)
#define VIEWER_TEXT_STICK_TOP (-1)
int viewerDrawText(int x, int y, const char* text);

void viewerClear();
void viewerSetTitle(const char* title, const char* modelName);
void viewerResetView();

void viewerQuit(int exitCode, const char *text);

void viewerPrintError(int err, const char* text, ...);


int viewerDrawRectangle(int x, int y, int width, int height);

int loopViewer(int *runlevel);

void viewerAddPaintMethod(PaintMethod paint, void *data);

void viewerGetWindowSize(int *width, int *height);

int viewerStringSize(const char *string, 
                     int length, 
                     int* width, 
                     int* height, 
                     int* character_width, 
                     int* character_height, 
                     int *baseline);

int viewerFillRectanlge(int x, int y, int width, int height, color_type color);
void viewerClearRectangle(int x, int y, int width, int height);

void viewerInvalidate();

void viewerFlush();

void viewerDrawDot(const EPoint *x);

#define GV_FONT_SIZE_NORMAL 16
#define GV_FONT_SIZE_SMALL 14

int viewerSetFontSize(int size);
int viewerGetFontSize();

int viewerSetLineWidth(int line_width);

#endif /* _VIEWER_ */

