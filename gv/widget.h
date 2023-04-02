#ifndef __WIDGET__
#define __WIDGET__

/* Widget for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "gobject.h"

#define VIEWEREVENT_BUTTONPRESS   1
#define VIEWEREVENT_BUTTONRELEASE 2
#define VIEWEREVENT_EXPOSURE 3
#define VIEWEREVENT_RESIZE 4
#define VIEWEREVENT_MOVE 5
#define VIEWEREVENT_DESTROY 6
#define VIEWEREVENT_VERTEX_SELECT 7
#define VIEWEREVENT_KEY  8
#define VIEWEREVENT_FOCUS 9
#define VIEWEREVENT_FOCUS_LOST 10


typedef struct ViewerWidgetT ViewerWidget;

typedef int (*ViewerCallback)(ViewerWidget *w, int ev, int x, int y, void *data);


#define VIEWERWIDGET_CLASS_BUTTON 1
#define VIEWERWIDGET_CLASS_WINDOW 2
#define VIEWERWIDGET_CLASS_INPUT  3

#define VIEWERWIDGET_CLASS_MASK 0xff

#define VIEWERWIDGET_TYPE_BUTTON (VIEWERWIDGET_CLASS_BUTTON|(1<<8))
#define VIEWERWIDGET_TYPE_CHECK (VIEWERWIDGET_CLASS_BUTTON|(2<<8))
#define VIEWERWIDGET_TYPE_TAP_BUTTON (VIEWERWIDGET_CLASS_BUTTON|(3<<8))

#define VIEWERWIDGET_TYPE_WINDOW (VIEWERWIDGET_CLASS_WINDOW|(1<<8))
#define VIEWERWIDGET_TYPE_TEXT (VIEWERWIDGET_CLASS_WINDOW|(2<<8))
#define VIEWERWIDGET_TYPE_TAP_WINDOW (VIEWERWIDGET_CLASS_WINDOW|(3<<8))

#define VIEWERWIDGET_TYPE_INPUT (VIEWERWIDGET_CLASS_INPUT|(1<<8))


#define VIEWERWIDGET_FLAG_TEMPORARY (1<<0)
#define VIEWERWIDGET_FLAG_INVISIBLE   (1<<1)
#define VIEWERWIDGET_FLAG_NOFRAME   (1<<2)
#define VIEWERWIDGET_FLAG_CLEARED   (1<<3)
#define VIEWERWIDGET_FLAG_SET       (1<<4)
#define VIEWERWIDGET_FLAG_IGNORE    (1<<5)
#define VIEWERWIDGET_FLAG_ESCAPE    (1<<6)
#define VIEWERWIDGET_FLAG_MANUAL_VISIBILITY (1<<7)
#define VIEWERWIDGET_FLAG_DONOT_FREE_NAME (1<<8)
#define VIEWERWIDGET_FLAG_USE_TEXT  (1<<9)
#define VIEWERWIDGET_FLAG_HAS_FOCUS (1<<10)
#define VIEWERWIDGET_FLAG_IS_PRESSED (1<<11)
#define VIEWERWIDGET_FLAG_LEFT_ALIGN (1<<12)
#define VIEWERWIDGET_FLAG_FIX_LABEL_POS (1<<13)
#define VIEWERWIDGET_FLAG_COLOR_SET  (1<<14)
#define VIEWERWIDGET_FLAG_FRAME_COLOR_SET  (1<<15)
#define VIEWERWIDGET_FLAG_HIGHLIGHTED (1<<16)

#define WIDGETCLASS(_w) ((_w)->wtype & VIEWERWIDGET_CLASS_MASK)

#define widgetSetCheck(_w,_c) { if (_c)  (_w)->flags |= VIEWERWIDGET_FLAG_SET;  \
                                    else (_w)->flags &= ~VIEWERWIDGET_FLAG_SET; }

#define WIDGETCHECKMARK(_w) ((_w)->flags & VIEWERWIDGET_FLAG_SET)

#define ISWIDGET_INVISIBLE(_w) ((_w)->flags & VIEWERWIDGET_FLAG_INVISIBLE)
#define ISWIDGET_VISIBLE(_w) (0 == ((_w)->flags & VIEWERWIDGET_FLAG_INVISIBLE))
#define ISWIDGET_DISABLED(_w) ((_w)->flags & VIEWERWIDGET_FLAG_IGNORE)
#define ISWIDGET_ESCAPE(_w) ((_w)->flags & VIEWERWIDGET_FLAG_ESCAPE)

#define DISABLE_WIDGET(_w)  (_w)->flags |= VIEWERWIDGET_FLAG_IGNORE
#define ENABLE_WIDGET(_w)   (_w)->flags &= ~VIEWERWIDGET_FLAG_IGNORE

typedef struct ViewerWidgetT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// ---------------------------------
    
    int wtype;
    
    int x,y; // Apsolut window coordinates
    int width,height;
    
    int label_x; // Apsolut window coordinates
    int label_y;
    
    // Should be replaced here!!!???
    // char* name;    
    char name[MODEL_NAME_SIZE];    
    const char* text;
    int textClip;
    
    ViewerCallback callback;
    
    color_type color;
    color_type frame_color;
    
    void *data;
    int line_count;
    
    struct ViewerWidgetT *sibling;
    
    struct ViewerWidgetT **childs;    
    int number_of_childs;
    int max_number_of_childs;
    
    int last_event;
} ViewerWidget;

#define VIEWERWIDGET_POS_STICK_BOTTOM (-1)
#define VIEWERWIDGET_KEEP_SIZE (-1)
#define WIDGET_TEXT_SIZE (-1)

int initWidget();

int widgetSendEvent(int ev, ViewerWidget *widget, int x, int y);
int widgetMoveWidget(ViewerWidget *widget, int x, int y);
int widgetSizeWidget(ViewerWidget *widget, int width, int height);

ViewerWidget* widgetCreateWidget(ViewerWidget *parent, const char* name, int type, int x, int y, int width, int height, ViewerCallback callback, void *data);
int widgetRemoveWidget(ViewerWidget *widget);
int widgetRemoveTempWidgets();
int widgetClearWidget(ViewerWidget *widget);
int widgetDrawWidget(ViewerWidget *widget);
int widgetVisible(ViewerWidget *widget);
int widgetInvisible(ViewerWidget *widget);
int widgetAddChild(ViewerWidget *widget, ViewerWidget *child);
int widgetAddSibling(ViewerWidget *widget, ViewerWidget *sibling);
void widgetInvalidate(ViewerWidget *widget);

int widgetDrawTextWidget(ViewerWidget *widget, int x, int y, const char* text, int length);

int widgetDrawWidgets();
const char *widgetTextClip(ViewerWidget *widget, const char *text, char* buffer, size_t buffer_size);

void widgetEnableDesktop(int enable, ViewerWidget *exclude);

int widgetFocus(ViewerWidget* widget);

int widgetSetText(ViewerWidget* widget, const char* text);
int widgetSetName(ViewerWidget* widget, const char* name);
int widgetSetTextPosition(ViewerWidget* widget, int x, int y);

ViewerWidget* widgetGetChild(ViewerWidget* widget, int index);

const char* widgetEventToText(int ev);

int widgetEnable(ViewerWidget *widget, int enable);

int widgetSetColor(ViewerWidget* widget, color_type font_color, color_type frame_color);



#endif