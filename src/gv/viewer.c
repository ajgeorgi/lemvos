#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Core.h>
#include <stdio.h>
#include <error.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <stdarg.h>
#include <stdlib.h>

#include <math.h>

/* Viewer for GV 19.08.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "viewer.h"
#include "configuration.h"
#include "popup.h"
#include "projection.h"
#include "modelstore.h"

#define FONT_SIZE_NORMAL _STR(GV_FONT_SIZE_NORMAL)
#define FONT_SIZE_SMALL  _STR(GV_FONT_SIZE_SMALL)
#define LINE_STYLE LineSolid,CapNotLast,JoinMiter
#define LINE_ATTRIBUTES _viewer_line_width,LINE_STYLE
#define GV_FONT "-misc-liberation mono-medium-r-normal-*-" FONT_SIZE_NORMAL "-*-*-*-*-*-iso8859-1"
#define GV_FONT_SMALL "-misc-liberation mono-medium-r-normal-*-" FONT_SIZE_SMALL "-*-*-*-*-*-iso8859-1"
#define LINE_WIDTH 2
#define EVENT_MASK ExposureMask | VisibilityChangeMask | KeyReleaseMask| SubstructureRedirectMask | SubstructureNotifyMask | \
                   Button1MotionMask | Button3MotionMask| Button4MotionMask | Button5MotionMask | KeyPressMask | \
                   StructureNotifyMask | OwnerGrabButtonMask | ButtonPressMask | ButtonReleaseMask
           
// extern int XWidthMMOfScreen(
//     Screen*             /* screen */
// );
// 
// extern int XWidthOfScreen(
//     Screen*             /* screen */
// );
           
// XtAppContext _viewerKontext = NULL;
Display* _viewerDisplay = NULL;
Window _viewerMainWindow = 0;
Window _displayRootWindow = 0;
GC _viewerGC = NULL;
// static GC _viewer_UI_GC = NULL;

extern Bool _widgetHandleWidgets(XEvent *event);

static PaintMethod _viewerPaintMethod = NULL;
static void* _viewerPaintMethodData = NULL;
static char viewer_model_name[64];

static void repaint();

static CommonErrorHandler _viewer_error_handler = NULL;

typedef struct {
    int font_size;
    int max_char_width;
    int max_char_height;                
    int font_baseline;
    XFontStruct *fontstruct;    
} _viewer_font_t;

static _viewer_font_t *viewer_font = NULL;
static _viewer_font_t viewer_normal_font;
static _viewer_font_t viewer_small_font;

static int _viewer_line_width = LINE_WIDTH;
static long _viewer_background_color = 0x505050;


#define VIEWERSTATE_NEEDS_REPLOT (1<<0)
#define VIEWERSTATE_STARTPOINT_SET (1<<1)
#define VIEWERSTATE_WINDOW_CLEARED (1<<2)

static struct {
    unsigned int state;

    int width;
    int height;
    
    int button1_press_x;
    int button1_press_y;
    int button2_press_x;
    int button2_press_y;
    
} _viewerState;


static int *_viewerRunlevel = NULL;
static int _viewerExitCode = 0;

void viewerPrintError(int err, const char* text, ...)
{
    char textBuffer[256];
    textBuffer[0] = 0;
    
    va_list param;
    va_start(param, text);    
    vsnprintf(textBuffer, sizeof(textBuffer), text, param);    
    va_end(param);    
    
    if (_viewer_error_handler)
    {
        _viewer_error_handler(err,textBuffer);
        
        _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
    }
    else
    {
        fputs(textBuffer, stderr);
    }
}

Visual *getVisual(Window w, int* depth_return)
{
  XWindowAttributes attributes;
  
  if (XGetWindowAttributes(_viewerDisplay, w, &attributes))
  {
    *depth_return = attributes.depth;
    return attributes.visual;
  }
  
  return NULL;
}

void viewerGetWindowSize(int *width, int *height)
{
    if (width)
    {
        *width = _viewerState.width;
    }
    
    if (height)
    {
        *height = _viewerState.height;
    }    
}

void viewerPopupQuitChoice(int choice, void* userData)
{
    if (POPUP_CHOICE_YES == choice)
    {
        _viewerExitCode = (long)userData;        
        if (_viewerRunlevel)
        {
            *_viewerRunlevel = 0;
        }
        else
        {
            exit(_viewerExitCode);
        }        
    }
}

void viewerQuit(int exitCode, const char *text)
{
    long ecode = (long)exitCode;
    char qtext[300];
    snprintf(qtext,sizeof(qtext),"Leave your model:\n\"%s\"\nfrom %s and exit?\n\n%s",
             commenNameFromModel(viewer_model_name),
             GV_APPLICATION,
             text
            );
    
    userRequest(qtext, "Quit", "Stay", viewerPopupQuitChoice, (void*)ecode);
}

static int createGC()
{
    XGCValues gcvalues = {
            GXcopy, // int function;   /* logical operation */
            AllPlanes, // unsigned long plane_mask;       /* plane mask */
            aperalGetColor(GAPERAL_FOREGROUND), // unsigned long foreground;       /* foreground pixel */
            aperalGetColor(GAPERAL_BACKGROUND), // unsigned long background;       /* background pixel */
            LINE_ATTRIBUTES, // int line_width; /* line width (in pixels) */
            // LineSolid, // int line_style; /* LineSolid, LineOnOffDash, LineDoubleDash */
            //CapNotLast, // int cap_style;  /* CapNotLast, CapButt, CapRound, CapProjecting */
            //JoinMiter, // int join_style; /* JoinMiter, JoinRound, JoinBevel */
            FillSolid, // int fill_style; /* FillSolid, FillTiled, FillStippled FillOpaqueStippled*/
            EvenOddRule, // int fill_rule;  /* EvenOddRule, WindingRule */
            ArcChord, // int arc_mode;   /* ArcChord, ArcPieSlice */
            0, // Pixmap tile;    /* tile pixmap for tiling operations */
            0, // Pixmap stipple; /* stipple 1 plane pixmap for stippling */
            0, // int ts_x_origin;        /* offset for tile or stipple operations */
            0, // int ts_y_origin;
            0, // Font font;      /* default text font for text operations */
            ClipByChildren, // int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
            FALSE, // Bool graphics_exposures;        /* boolean, should exposures be generated */
            0, // int clip_x_origin;      /* origin for clipping */
            0, // int clip_y_origin;
            0, // Pixmap clip_mask;       /* bitmap clipping; other calls for rects */
            2, // int dash_offset;        /* patterned/dashed line information */
            0, // char dashes;
    } ;
    
    if (NULL == _viewerGC)
    {
        _viewerGC = XCreateGC(_viewerDisplay, _viewerMainWindow, 
                      GCFunction | GCPlaneMask | GCFillStyle | GCFillRule | GCForeground | GCBackground | GCLineWidth | GCSubwindowMode | GCLineStyle,
                      &gcvalues);
        
        if (_viewerGC)
        {
            viewer_normal_font.fontstruct = XLoadQueryFont(_viewerDisplay, GV_FONT);

            if (viewer_normal_font.fontstruct)
            {
                 viewer_font = &viewer_normal_font;
                
                 XSetFont(_viewerDisplay, _viewerGC, viewer_normal_font.fontstruct->fid);

                 viewer_normal_font.font_size = GV_FONT_SIZE_NORMAL;
                 viewer_normal_font.max_char_width = viewer_normal_font.fontstruct->max_bounds.rbearing - viewer_normal_font.fontstruct->min_bounds.lbearing;
                 viewer_normal_font.max_char_height = viewer_normal_font.fontstruct->max_bounds.ascent + viewer_normal_font.fontstruct->max_bounds.descent;   
                 viewer_normal_font.font_baseline = viewer_normal_font.fontstruct->max_bounds.ascent;
                 
                 LOG("Font: char W=%i, H=%i, bl=%i\n",viewer_normal_font.max_char_width ,viewer_normal_font.max_char_height, viewer_normal_font.font_baseline );
            }

            viewer_small_font.fontstruct = XLoadQueryFont(_viewerDisplay, GV_FONT_SMALL);

            if (viewer_small_font.fontstruct)
            {
                 // XSetFont(_viewerDisplay, _viewerGC, viewer_small_font->fid);

                 viewer_small_font.font_size = GV_FONT_SIZE_SMALL;
                 viewer_small_font.max_char_width = viewer_small_font.fontstruct->max_bounds.rbearing - viewer_small_font.fontstruct->min_bounds.lbearing;
                 viewer_small_font.max_char_height = viewer_small_font.fontstruct->max_bounds.ascent + viewer_small_font.fontstruct->max_bounds.descent;   
                 viewer_small_font.font_baseline = viewer_small_font.fontstruct->max_bounds.ascent;
                 
                 LOG("Font: char W=%i, H=%i, bl=%i\n",viewer_small_font.max_char_width ,viewer_small_font.max_char_height, viewer_small_font.font_baseline );
            }
            
        }
    } 

    if (NULL == _viewerGC)
    {
        viewerPrintError(0, "GC failed\n");
        
        error(0, 0, "GC failed\n");
        return -1;
    }    
    
    return 0;
}
    
int viewerInit(int argc, char **argv, CommonErrorHandler error_handler)
{
    (void)argc;
    (void)argv;
    
    viewer_model_name[0] = 0;
    memset(&_viewerState,0,sizeof(_viewerState));
    _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;

    _viewer_error_handler = error_handler;
    
    _viewerDisplay = XOpenDisplay(":0.0");

    if (_viewerDisplay == NULL)
    {
        ERROR1("No display\n");
        
        return -1;
    }

    const int defaultScreen = DefaultScreen(_viewerDisplay);
    
    int displayWidth = DisplayWidth(_viewerDisplay, defaultScreen) - 200;
    int displayHeight = DisplayHeight(_viewerDisplay, defaultScreen) - 200;
    int depth = DefaultDepth(_viewerDisplay, defaultScreen);

    Screen *screen = ScreenOfDisplay(_viewerDisplay,defaultScreen);
    Visual *visual = NULL;
    _displayRootWindow = RootWindowOfScreen(screen);
    
    for (int i = 0; i < screen->ndepths; i++)
    {
        if (visual)
        {
            break;
        }

        LOG("Screen.depth: d=%i\n",screen->depths[i].depth);
        for (int j = 0; j < screen->depths[i].nvisuals; j++)
        {
            LOG("  visual: id=%lu, class=%i, map_entris=%i, bits_per_rgb=%i, RGB=(%lx,%lx,%lx)\n",
                    screen->depths[i].visuals[j].visualid, 
                    screen->depths[i].visuals[j].class, 
                    screen->depths[i].visuals[j].map_entries, 
                    screen->depths[i].visuals[j].bits_per_rgb,
                    screen->depths[i].visuals[j].red_mask,
                    screen->depths[i].visuals[j].green_mask,
                    screen->depths[i].visuals[j].blue_mask);       
                    
            if (24 <= screen->depths[i].depth)
            {
                visual = &screen->depths[i].visuals[j];
                depth = screen->depths[i].depth;
            
                break;
            }
        }
    }
    
    if (NULL == visual)
    {
        error(0, 0, "No visual\n");
        
        return -1;
    }

    XSetWindowAttributes attributes;
    
    memset(&attributes,0,sizeof(attributes));
    attributes.background_pixmap = None;
    attributes.background_pixel = aperalGetColor(GAPERAL_BACKGROUND);
    attributes.border_pixmap = None;
    attributes.border_pixel = 0;
    attributes.bit_gravity =  0;
    attributes.win_gravity = 0;
    attributes.backing_store = NotUseful;
    attributes.backing_planes = AllPlanes;
    attributes.backing_pixel = 0;
    attributes.save_under = FALSE;
    attributes.event_mask = EVENT_MASK;
    attributes.do_not_propagate_mask = 0;
    attributes.override_redirect = FALSE;
    attributes.colormap = None;
    attributes.cursor = None;
    
    _viewerMainWindow = XCreateWindow(_viewerDisplay, 
                                      _displayRootWindow, 
                                      100, 100 , //int x, int y, 
                                      200, 200, // unsigned int width, unsigned int height,
                                      0, 
                                      depth, 
                                      InputOutput, 
                                      visual, 
                            CWColormap | CWBackPixel | CWSaveUnder | CWOverrideRedirect | CWBackingStore | CWBackingPlanes | 
                            CWEventMask | CWDontPropagate,
                                      &attributes);
/*
    int er =  XSelectInput(_viewerDisplay, _viewerMainWindow, attributes.event_mask);
    if (0>er)
    {
        perror("XSelectInput");
    }
*/
    XAddToSaveSet(_viewerDisplay, _displayRootWindow);


    XClassHint *chints = XAllocClassHint();
    chints->res_name = GV_APPLICATION;
    chints->res_class = GV_APPL_CLASS;
   
    XWMHints hints;
    
    memset(&hints,0,sizeof(hints));
    
    hints.flags = StateHint | InputHint;
    hints.input = TRUE;
    hints.initial_state = NormalState;
    hints.icon_pixmap = 0;
    hints.icon_window = 0;
    hints.icon_x = 0;
    hints.icon_y = 0;
    hints.icon_mask = AllPlanes;
    hints.window_group = 0;
    
    XmbSetWMProperties(_viewerDisplay, _viewerMainWindow, 
                            GV_APPLICATION, 
                            NULL, 
                            argv, argc, 
                            NULL, 
                            &hints, 
                            chints);

    XFree(chints);

    popupInit();
    
    viewerSetTitle("Empty",NULL);

    projectionInit();
    
    XMapWindow(_viewerDisplay, _viewerMainWindow);
    
    XResizeWindow(_viewerDisplay, _viewerMainWindow, displayWidth, displayHeight);
       
    createGC();    
    
    LOG("mainWindow = %lu, root=%lu\n",_viewerMainWindow,
            _displayRootWindow);
    
    return 0;
}

void viewerSetTitle(const char* title, const char *modelName)
{
    char name[256];
    snprintf(name,sizeof(name),"%s (%s)-Lemvos-Model: %s",GV_APPLICATION,GV_VERSION,title);
    
    char *text = name;
    XTextProperty text_prop;
    XStringListToTextProperty(&text, 1, &text_prop);

    XSetWMName(_viewerDisplay, _viewerMainWindow, &text_prop);
    
    if (modelName)
    {
        strncpy(viewer_model_name,modelName,sizeof(viewer_model_name));
    }
    else
    {
        viewer_model_name[0] = 0;
    }
    
    // fprintf(stderr,"Title set: %s\n",title);
}

void viewerResetView()
{
    _viewer_background_color = aperalGetColor(GAPERAL_BACKGROUND);
    _viewer_line_width = configInt(configGetConfig("viewer.line_width"),LINE_WIDTH);
    _viewer_background_color = configULong(configGetConfig("viewer.background_color"),_viewer_background_color);

    projectionReset();

    // fprintf(stderr,"Background = %lX\n",_viewer_background_color);
    
    // Set line width here
    XSetLineAttributes(_viewerDisplay, _viewerGC, LINE_ATTRIBUTES);
    
    // May be make sure there is more contrast???
    XSetBackground(_viewerDisplay, _viewerGC, _viewer_background_color);    
    XSetWindowBackground(_viewerDisplay, _viewerMainWindow, _viewer_background_color);
    
    repaint();
#ifdef _DEBUG    
    EPoint *dist = projectionGetVelue(1);
    EPoint *height = projectionGetVelue(2);

    LOG("View reseted, cam: distance = %.2f, height = %.2f\n",dist?*dist:0,height?*height:0);
#endif
    
    LOG_FLUSH;
}


static Bool event_filter(Display *display, XEvent *event, XPointer arg)
{
    (void) display;
    // XWindowAttributes *window_attributes = (XWindowAttributes*)arg;
    // window_attributes->all_event_masks
    (void) arg;

    Bool mw = event->xany.window == _viewerMainWindow;
    Bool dr = event->xany.window == _displayRootWindow;
              
    return mw || dr;
}

char *eventType2Name(int type)
{
    switch (type)
    {
        case EnterNotify: return "EnterNotify";
        case LeaveNotify: return "LeaveNotify";
        
        case KeyRelease: return "KeyRelease";
        case KeyPress: return "KeyPress";
        
        case PropertyNotify: return "PropertyNotify";
        case ReparentNotify: return "ReparentNotify";

        case MapNotify: return "MapNotify";
        case VisibilityNotify: return "VisibilityNotify";
        case Expose: return "Expose";

        case UnmapNotify: return "UnmapNotify";
        case GraphicsExpose: return "GraphicsExpose";
        case CreateNotify: return "CreateNotify";
        
        case ConfigureNotify: return "ConfigureNotify";
        case ConfigureRequest: return "ConfigureRequest";
        case MappingNotify: return "MappingNotify";
        
        case DestroyNotify: return "DestroyNotify";
        case ButtonPress: return "ButtonPress";
        case ButtonRelease: return "ButtonRelease";        
        case MotionNotify: return "MotionNotify";
    }
    
    return UNKNOWN_SYMBOL;
}

void repaint()
{
    if (NULL != _viewerPaintMethod)
    {
        viewerClear();
        
        _viewerPaintMethod(_viewerPaintMethodData);
    }
    
    widgetDrawWidgets();

    XMapRaised(_viewerDisplay,_viewerMainWindow);
    
    _viewerState.state &= ~(VIEWERSTATE_NEEDS_REPLOT|VIEWERSTATE_WINDOW_CLEARED);  
}


char *eventMaskToString(long mask, char* buffer,size_t bsize)
{
    buffer[0] = 0;
    
    for (size_t i = 0; i < sizeof(mask);i++)
    {
        long my_mask = (1<<i);
        
        int len = bsize - strlen(buffer);
        switch (mask & my_mask)
        {
            case ExposureMask: strncat(buffer,"ExposureMask,\n",len); break;
            case VisibilityChangeMask: strncat(buffer,"VisibilityChangeMask,\n",len); break;
            case KeyReleaseMask: strncat(buffer,"KeyReleaseMask,\n",len); break;
            case Button1MotionMask: strncat(buffer,"Button1MotionMask,\n",len); break;
            case Button3MotionMask: strncat(buffer,"Button3MotionMask,\n",len); break;
            case Button4MotionMask: strncat(buffer,"Button4MotionMask,\n",len); break;
            case Button5MotionMask: strncat(buffer,"Button5MotionMask,\n",len); break;
            case KeyPressMask: strncat(buffer,"KeyPressMask,\n",len); break;
            case StructureNotifyMask: strncat(buffer,"StructureNotifyMask,\n",len); break;
            case SubstructureNotifyMask: strncat(buffer,"SubstructureNotifyMask,\n",len); break;
            case OwnerGrabButtonMask: strncat(buffer,"OwnerGrabButtonMask,\n",len); break;
            case ButtonPressMask: strncat(buffer,"ButtonPressMask,\n",len); break;
            case SubstructureRedirectMask: strncat(buffer,"SubstructureRedirectMask,\n",len); break;
        }
    }
    
    return buffer;
}

void _viewerPrintEventMask()
{
   char alls[1000];
   char yours[1000];
 
   XWindowAttributes window_attributes;
   XGetWindowAttributes(_viewerDisplay, _viewerMainWindow, &window_attributes);    
   
   LOG("wattrib.yev_mask = %s\nwattrib.aev_mask = %s\n",
           eventMaskToString(window_attributes.your_event_mask,alls,sizeof(alls)),
           eventMaskToString( window_attributes.all_event_masks,yours,sizeof(yours)));
   
   const int defaultScreen = DefaultScreen(_viewerDisplay);

   Screen *screen = ScreenOfDisplay(_viewerDisplay,defaultScreen);

   long screenMask = EventMaskOfScreen(screen);

   LOG("Screen Mask = %s\n",
           eventMaskToString(screenMask,alls,sizeof(alls)));
}

void viewerDrawCoordinates(int x, int y)
{
    char buffer[80];
    EPoint dir[3],pos[3];
    char pstr[40];
    
    projectionDisplayToModel(x,y,pos,dir);
//    const Vertex *v = modelstoreFindVertex(pos, dir);

    snprintf(buffer,sizeof(buffer),"X: %i, Y: %i, pos=%s", x, y, EPoint3ToString(pos,pstr,sizeof(pstr)));
   
    viewerSetDrawingColor(aperalGetColor(GAPERAL_WIDGET));

    viewerDrawText(0,0,buffer);
}

int loopViewer(int *runlevel)
{    
   XEvent event;

   _viewerRunlevel = runlevel;
   
   unsigned int serial = 0;
   while ((0 < *runlevel) && (0 == XIfEvent(_viewerDisplay, &event, event_filter, (XPointer) NULL)))
   {
       serial++;
       /*
       printf("Ev %u: type=%i (%s), window=%lu, runlevel=%i\n",
              serial,
              event.type, 
              eventType2Name(event.type), 
              event.xany.window,              
              *runlevel);
       */
       if (Expose == event.type)
       {
            if ((_viewerState.width != event.xexpose.width) ||
                (_viewerState.height != event.xexpose.height))
            {
                _viewerState.width = event.xexpose.width;
                _viewerState.height = event.xexpose.height;   
                
                projectionReset();
            }

            _widgetHandleWidgets(&event);

            {
                _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;  
            }
       }
       else
       if (MotionNotify == event.type)
       {
           // fprintf(stderr,"Motion: (%i,%i), button=%u\n",event.xmotion.x,event.xmotion.y,event.xmotion.state);
           
           if ((event.xmotion.state & Button1Mask) && (_viewerState.state & VIEWERSTATE_STARTPOINT_SET))
           {
               if ((event.xmotion.x < _viewerState.width) &&
                   (event.xmotion.y < _viewerState.height) &&
                   (event.xmotion.x > 0) &&
                   (event.xmotion.y > 0))
               {
                    double du = 0;
                    double dv = 0;
                    
                    if (0 <= event.xmotion.x)
                    {
                            du = (((double)event.xmotion.x) - ((double) _viewerState.button1_press_x));
                    }
                    
                    if (0 <= event.xmotion.y)
                    {               
                            dv = (((double)event.xmotion.y) - ((double)_viewerState.button1_press_y));
                    }
                    
                     _viewerState.button1_press_x = event.xbutton.x;
                     _viewerState.button1_press_y = event.xmotion.y;
                    
                    projectionTiltCam(du, dv);
                    
                    _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;                    
               }
           }
           else
           if ((event.xmotion.state & Button3Mask) && (_viewerState.state & VIEWERSTATE_STARTPOINT_SET))
           {
               if ((event.xmotion.x < _viewerState.width) &&
                   (event.xmotion.y < _viewerState.height) &&
                   (event.xmotion.x > 0) &&
                   (event.xmotion.y > 0))
               {
                    double dx = 0;
                    double dy = 0;
                    
                    if (0 <= event.xmotion.x)
                    {
                            dx = (((double)event.xmotion.x) - ((double) _viewerState.button1_press_x));
                    }
                    
                    if (0 <= event.xmotion.y)
                    {               
                            dy = (((double)event.xmotion.y) - ((double)_viewerState.button1_press_y));
                    }

                     _viewerState.button1_press_x = event.xbutton.x;
                     _viewerState.button1_press_y = event.xmotion.y;
                     
                    // fprintf(stderr,"du = %.2f, dv = %.2f, window(%i,%i)\n",du,dv, _viewerState.width, _viewerState.height);
                    projectionPanCam(dx, -dy);
                    _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
               }
           } 

           {
               viewerDrawCoordinates(event.xmotion.x, event.xmotion.y);
           }
       }
       else
       if (ButtonPress == event.type)
       {
//             if (event.xbutton.state & ControlMask)
//             {
//                 EPoint p[3],d[3];
//                 if (projectionDisplayToModel(event.xbutton.x, event.xbutton.y, p,d))
//                 {        
//                     Vertex *vertex = modelstoreFindVertex(p,d);
//                     if (vertex)
//                     {
//                         vertex->flags |= GM_VERTEX_FLAG_MARK;
//                         _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
//                     }
//                 }
//             }                    
//            else
           if (! _widgetHandleWidgets(&event))
           {
                // fprintf(stderr,"Press s=%i, b=%i\n",event.xbutton.state,event.xbutton.button);
                if ((1 == event.xbutton.button) || (3 == event.xbutton.button))
                {
                        _viewerState.button1_press_x = event.xbutton.x;
                        _viewerState.button1_press_y = event.xbutton.y;   
                        
                        _viewerState.state |= VIEWERSTATE_STARTPOINT_SET;
                        
                        // fprintf(stderr,"ButtonPress at (%i,%i)\n",event.xbutton.x,event.xbutton.y);
                }
                else
                if (5 == event.xbutton.button)
                {
                    projectionZoomCam(50);
                }
                else
                if (4 == event.xbutton.button)
                {
                    projectionZoomCam(-50);
                }
           }
           viewerDrawCoordinates(event.xbutton.x, event.xbutton.y);
       }
       else
       if (ButtonRelease == event.type)
       {
           _widgetHandleWidgets(&event);
           
           
           // fprintf(stderr,"Release s=%i, b=%i\n",event.xbutton.state,event.xbutton.button);

           viewerDrawCoordinates(event.xbutton.x, event.xbutton.y);
                          
           _viewerState.state &= ~VIEWERSTATE_STARTPOINT_SET;
           
       }
       else
       if (ConfigureNotify == event.type)
       {
            if ((_viewerState.width != event.xconfigure.width) ||
                (_viewerState.height != event.xconfigure.height))
            {
                _viewerState.width = event.xconfigure.width;
                _viewerState.height = event.xconfigure.height;

                _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
            }
       }
       else
       if (ReparentNotify == event.type)
       {
           _displayRootWindow = event.xreparent.parent;
#ifdef _DEBUG
           LOG("Reparent of %lu, parent is %lu from %lu\n",
                   event.xreparent.window,
                   event.xreparent.parent,
                   event.xreparent.event);
#endif           
       }
       else
       if (KeyPress == event.type)
       {
           _widgetHandleWidgets(&event);
       }
       
       if (_viewerState.state & VIEWERSTATE_NEEDS_REPLOT)
       {
            repaint();
       }

   }

  if (viewer_normal_font.fontstruct)
  {
     XFreeFont(_viewerDisplay,viewer_normal_font.fontstruct);
  }
  if (viewer_small_font.fontstruct)
  {
     XFreeFont(_viewerDisplay,viewer_small_font.fontstruct);
  }
  viewer_font = NULL;
  
  XDestroyWindow(_viewerDisplay,_viewerMainWindow); 
  XFreeGC(_viewerDisplay, _viewerGC);
  XCloseDisplay(_viewerDisplay);  
  
  return _viewerExitCode;
}

void viewerAddPaintMethod(PaintMethod paint, void* data)
{
    _viewerPaintMethodData = data;
    _viewerPaintMethod = paint;
    
    viewerResetView();
    
    _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
}



int viewerSetDrawingColor(color_type color)
{
    if (NULL == _viewerGC)
    {
        createGC();
    }
    
    if (_GVCOLOR_INDEXED_ID == (_GVCOLOR_ID_MASK & color))
    {
        color = aperalGetColor(_GVCOLOR_INDEXED_MASK & color);
    }
    
    if (GVCOLOR_FOREGROUND == color)
    {
        color = aperalGetColor(GAPERAL_FOREGROUND);
    }

    if (GVCOLOR_BACKGROUND == color)
    {
        color = aperalGetColor(GAPERAL_BACKGROUND);
    }
    
    return XSetForeground(_viewerDisplay, _viewerGC, color);
}

unsigned long viewerGetDrawingColor()
{
    XGCValues values;
    XGetGCValues(_viewerDisplay, _viewerGC, GCForeground | GCBackground, &values);
    
    return values.foreground;
}

int viewerDrawLine3D(const EPoint *p1, const EPoint *p2)
{    
    int x1[2], x2[2];
    
    projectionProject(p1,x1);
    projectionProject(p2,x2);
 
    // fprintf(stderr,"Plotting (%i, %i) -> (%i, %i)\n",x1[0],x1[1],x2[0],x2[1]);

    int err = XDrawLine(_viewerDisplay, _viewerMainWindow, _viewerGC, 
                        x1[0], x1[1], 
                        x2[0], x2[1]);
    if (0 > err)
    {
        error(0, err, "Draw failed\n");
        LOG1("Draw failed\n");
        return -1;
    }
    
    return 0;
}

int viewerDrawLine(int x1, int y1, int x2, int y2)
{
    return XDrawLine(_viewerDisplay, _viewerMainWindow, _viewerGC, 
                x1, y1, 
                x2, y2);        
}


int viewerDrawRectangle(int x, int y, int width, int height)
{
    if (NULL == _viewerGC)
    {
        createGC();
    }
    
    XPoint xp[5];
    
    xp[0].x = x;
    xp[0].y = y;

    xp[1].x = x + width;
    xp[1].y = y;

    xp[2].x = x + width;
    xp[2].y = y + height;

    xp[3].x = x;
    xp[3].y = y + height;

    xp[4].x = x;
    xp[4].y = y;

    return XDrawLines(_viewerDisplay,_viewerMainWindow,_viewerGC,
               xp, 
               sizeof(xp)/sizeof(XPoint), 
               CoordModeOrigin);
}

int viewerFillRectanlge(int x, int y, int width, int height, unsigned long color)
{
    viewerSetDrawingColor(color);

    return XFillRectangle(_viewerDisplay, _viewerMainWindow, _viewerGC, x, y, width,height);
}

void viewerClear()
{
    XClearWindow(_viewerDisplay,_viewerMainWindow);
    
    _viewerState.state |= (VIEWERSTATE_WINDOW_CLEARED | VIEWERSTATE_NEEDS_REPLOT);
}

int viewerStringSize(const char *string, int length, int* width, int* height, int* character_width, int* character_height, int *baseline)
{
    // XFontStruct *fontstruct: Is needed to get better results here!
    
    if (viewer_font)        
    {
        if (string)
        {
            int direction_return;
            int font_ascent_return;
            int font_descent_return;
            XCharStruct overall_return;
            
            XTextExtents(viewer_font->fontstruct, string, length, 
                        &direction_return, 
                        &font_ascent_return, 
                        &font_descent_return, 
                        &overall_return);

            if (width)
            {
                *width = overall_return.width;
            }
            
            if (height)
            {
                *height =  overall_return.ascent + overall_return.descent;
            }
        }
        
        if (baseline)
        {
            *baseline = viewer_font->fontstruct->max_bounds.ascent;
        }
        
        if (character_width)
        {
            *character_width = viewer_font->fontstruct->max_bounds.rbearing - viewer_font->fontstruct->min_bounds.lbearing;
        }
        
        if (character_height)
        {
            *character_height = viewer_font->fontstruct->max_bounds.ascent + viewer_font->fontstruct->max_bounds.descent;
        }
        
        return 0;
    }    
    return -1;
}
    
int viewerDrawText(int x, int y, const char* text)
{
    if (text && viewer_font)
    {
        int label_len = strlen(text);

        if (*text)
        {
            if (x < 0)
            {
                x = (_viewerState.width - (label_len * viewer_font->max_char_width)) - 3;
            }
            
            if (y < 2)
            {
                if (VIEWER_TEXT_STICK_TOP != y)
                {
                    y = 5;
                }
            }
            
            if (y < 0)
            {
                y = _viewerState.height - viewer_font->max_char_height;
            }
        }
        else
        {
            label_len = 5; // ???: To clear field. See clip text
        }

        // Clear background
        if (0 == (_viewerState.state & VIEWERSTATE_WINDOW_CLEARED))
        {
            const int width = ((label_len + 3)* viewer_font->max_char_width);
            const int height = viewer_font->max_char_height;
            
            const int yc = _viewerState.height -  y - viewer_font->font_baseline;
            
            XClearArea(_viewerDisplay, _viewerMainWindow, x , yc, width, height, False);
        }
        
        if (*text)
        {
            int err = XDrawString(_viewerDisplay, _viewerMainWindow, _viewerGC,
                        x, _viewerState.height - y, text, label_len);

            if (0 > err)
            {
                viewerPrintError(0, "Draw text failed\n");
                error(0, err, "Draw text failed\n");            
                return -1;
            }
        }
    }
    
    return 0;    
}

int viewerDrawText3D(EPoint *p, const char* text, size_t length)
{
    int x1[2];
    
    projectionProject(p,x1);
    
    int err = XDrawString(_viewerDisplay, _viewerMainWindow, _viewerGC,
                x1[0], x1[1], text, length);

    if (0 > err)
    {
        viewerPrintError(0,  "Draw text failed\n");
        error(0, err, "Draw text failed\n");
        
        return -1;
    }
    
    return 0;
}

void viewerInvalidate()
{
    _viewerState.state |= VIEWERSTATE_NEEDS_REPLOT;
}

void viewerDrawDot(const EPoint *x)
{
    int x1[2];
    
    projectionProject(x,x1);
 
    // fprintf(stderr,"Plotting (%i, %i) -> (%i, %i)\n",x1[0],x1[1],x2[0],x2[1]);

    XDrawPoint(_viewerDisplay, _viewerMainWindow, _viewerGC, 
                        x1[0], x1[1]);
}

int viewerSetFontSize(int size)
{
    switch(size)
    {
        case GV_FONT_SIZE_NORMAL: {
            if (viewer_normal_font.fontstruct)
            {
                if (0  == XSetFont(_viewerDisplay, _viewerGC, viewer_normal_font.fontstruct->fid))
                {
                    viewer_font  = &viewer_normal_font;
                    return 0;
                }
            }
        } break;
        case GV_FONT_SIZE_SMALL: {
            if (viewer_small_font.fontstruct)
            {
                if (0  == XSetFont(_viewerDisplay, _viewerGC, viewer_small_font.fontstruct->fid))
                {
                    viewer_font  = &viewer_small_font;
                    return 0;
                }
            }
        } break;
        default: {
            if (0  == XSetFont(_viewerDisplay, _viewerGC, viewer_normal_font.fontstruct->fid))
            {
                viewer_font  = &viewer_normal_font;
                return 0;
            }
        }
    }
    
    return -1;
}

int viewerGetFontSize()
{
    if (viewer_font)
    {
        return viewer_font->font_size;
    }
    
    return -1;
}

int viewerSetLineWidth(int line_width)
{
    if (0 <= line_width)
    {
        return XSetLineAttributes(_viewerDisplay, _viewerGC,line_width, LINE_STYLE);
    }
    
    return XSetLineAttributes(_viewerDisplay, _viewerGC,LINE_ATTRIBUTES);    
}

