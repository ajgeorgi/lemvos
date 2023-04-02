#include <X11/X.h>
#include <X11/Xlib.h>

#include <string.h>
#include <stdio.h>

/* Widget for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"
#include "viewer.h"
#include "aperal.h"

static ViewerWidget *_viewer_widget_first = NULL;
static ViewerWidget *_viewer_widget_last = NULL;

static int _viewerInsideEventLoop = 0;

extern int _viewer_max_char_width;
extern int _viewer_max_char_height;                
extern int _viewer_font_baseline;

extern Display* _viewerDisplay;
extern Window _viewerMainWindow;
extern Window _displayRootWindow;
extern GC _viewerGC;
// extern GC _viewer_UI_GC;
static int _top_level_widget_index = 1;

static struct EventQueue { 
    int ev;
    ViewerWidget *widget;
    int x;
    int y;
} _widget_event_queue[10];

static int widgetPlaceLabel(ViewerWidget *widget);


int initWidget()
{
    memset(_widget_event_queue,0,sizeof(_widget_event_queue));
    
    return 0;
}

const char *widgetTextClip(ViewerWidget *widget, const char *text, char* buffer, size_t buffer_size)
{
    int width,height;
    int charWidth = 0;
    
    if (text)
    {
        const int textLen = strlen(text);
        viewerStringSize(text, textLen,&width, &height, &charWidth, NULL,NULL);

        if ((width > widget->width) && (0 != charWidth))
        {
            int max_character =  widget->width / charWidth;
            
            if (0 < max_character)
            {
                if (buffer)
                {
                    strncpy(buffer,text,buffer_size);
                    buffer[max_character-1] = 0;
                }
                
                widget->textClip = max_character;
                
                widget->label_x = 2;
                
                if (buffer)
                {
                    return buffer;
                }
            }
        }
    }
    
    return text;
}

static int _widgetSendEvent(int ev, ViewerWidget *widget, int x, int y)
{
    int ret = 0;
        
    if (isGObject(OBJ_WIDGET,widget) && widget->callback)
    {
        if (ISWIDGET_VISIBLE(widget))
        {        
            if (VIEWEREVENT_EXPOSURE == ev)
            {
                if ((VIEWERWIDGET_CLASS_WINDOW == WIDGETCLASS(widget)) ||
                    (VIEWERWIDGET_CLASS_INPUT == WIDGETCLASS(widget)))
                {
                    widget->last_event = ev;
                    ret = widget->callback(widget,ev,x,y, widget->data);
                }
            }
            else
            {
                if (!ISWIDGET_DISABLED(widget))
                {
                    if (VIEWEREVENT_KEY == ev)
                    {                     
                        if (VIEWERWIDGET_CLASS_INPUT == WIDGETCLASS(widget))
                        {                     
                            if (widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS)
                            {
                                widget->last_event = ev;
                                ret = widget->callback(widget,ev,x,y, widget->data);
                            }  
                        }
                    }
                    else
                    {
                        widget->last_event = ev;
                        ret = widget->callback(widget,ev,x,y, widget->data);                        
                    }
                }
                else
                {
                    if ((VIEWEREVENT_RESIZE == ev) || (VIEWEREVENT_MOVE == ev) || (VIEWEREVENT_DESTROY == ev))
                    {
                        widget->last_event = ev;
                        ret = widget->callback(widget,ev,x,y, widget->data);
                    }                     
                }
            }
        }
        else
        {
            if ((VIEWEREVENT_RESIZE == ev) || (VIEWEREVENT_MOVE == ev) || (VIEWEREVENT_DESTROY == ev))
            {
                widget->last_event = ev;
                ret = widget->callback(widget,ev,x,y, widget->data);
            }
        }            
    }
        
    return ret;   
}

int widgetSendEvent(int ev, ViewerWidget *widget, int x, int y)
{
    static struct EventQueue *qstart = &_widget_event_queue[0];
    static struct EventQueue *qend = &_widget_event_queue[0];
    static struct EventQueue *qlast = &_widget_event_queue[sizeof(_widget_event_queue)/sizeof(_widget_event_queue[0])-1];
    
    int ret = 0;
#ifdef _DEBUG                    
    char s1[GM_VERTEX_BUFFER_SIZE];
#endif    
  
    if (isGObject(OBJ_WIDGET,widget) && widget->callback)
    {
        if (0 == _viewerInsideEventLoop)
        {
            _viewerInsideEventLoop++;
            
            ret = _widgetSendEvent(ev,widget,x,y);

            _viewerInsideEventLoop--;
            
            while ((qend != qstart) && qend->widget)
            {
#ifdef _DEBUG                
                LOG("Removing %i->[%s] from queue\n",qend->ev,vertexPath((GObject*)qend->widget,s1,sizeof(s1)));
#endif                
                
                int err = 0;
                _viewerInsideEventLoop++;
                
                err = _widgetSendEvent(qend->ev,qend->widget,qend->x,qend->y);
                
                _viewerInsideEventLoop--;

                qend->widget = NULL;
                qend->ev = 0;
                
                qend++;
                if (qend > qlast)
                {
                    qend = &_widget_event_queue[0];
                }     
                
                if (0 == ret)
                {
                    ret = err;
                }                
            }
        }
        else
        {
            if (ev == VIEWEREVENT_DESTROY)
            {
                _viewerInsideEventLoop++;
        
                _widgetSendEvent(ev,widget,x,y);
        
                _viewerInsideEventLoop--;                        
            }
            else
            if (3 > _viewerInsideEventLoop)
            {
                if ((NULL == qstart->widget) && 
                    (ev != VIEWEREVENT_EXPOSURE) && 
                    (ev != VIEWEREVENT_RESIZE) && 
                    (ev != VIEWEREVENT_MOVE))
                {
                    qstart->widget = widget;
                    qstart->ev = ev;
                    qstart->x = x;
                    qstart->y = y;
                    
#ifdef _DEBUG                                    
                    LOG("Adding %i->[%s] to queue\n",ev,vertexPath((GObject*)widget,s1,sizeof(s1)));
#endif                    
                    
                    qstart++;
                    if (qstart > qlast)
                    {
                        qstart = &_widget_event_queue[0];
                    }                
                }
            }
        }
    }
    
    return ret;   
}


int widgetMoveWidget(ViewerWidget *widget, int x, int y)
{
    if (isObject(widget))
    {
        // Absolute coordinates from parents
        // Get position from parents
        int px = 0;
        int py = 0;

        ViewerWidget *parent = (ViewerWidget *)isGObject(OBJ_WIDGET,widget->parent);
        
        if (VIEWERWIDGET_POS_STICK_BOTTOM == y)
        {
            if (parent)
            {
                y = parent->height - widget->height - 10;
            }
            else
            {
                int width,height;
                viewerGetWindowSize(&width,&height);

                y = height  - widget->height - 10;
            }
        }
        
        if (parent)
        {            
            px = parent->x;
            py = parent->y;
        }
        
        // New absolue coordinates
        widget->x = px + x;
        widget->y = py + y;

         widgetPlaceLabel(widget);

         widgetSendEvent(VIEWEREVENT_MOVE,widget,widget->x,widget->y);
        
        widgetInvalidate(widget);        
    }
    
    return 0;
}

int widgetPlaceLabel(ViewerWidget *widget)
{
    int label_len = 0;
    const char *text = widget->name;
    
    widget->textClip = 0;
    
    if (VIEWERWIDGET_CLASS_BUTTON == WIDGETCLASS(widget))
    {
        if (VIEWERWIDGET_FLAG_USE_TEXT & widget->flags)
        {
            text = widget->text;
            widget->text = widgetTextClip(widget, widget->text, NULL, 0);
        }
        else
        {
            const char *name = widgetTextClip(widget, widget->name, NULL, 0);
            
            if (name != widget->name)
            {
                widget->text = name;
                widget->flags |= VIEWERWIDGET_FLAG_USE_TEXT;
                text = widget->text;
            }        
        }
    }
    else
    {
        if (VIEWERWIDGET_FLAG_USE_TEXT & widget->flags)
        {
            text = widget->text;
        }
    }
    
    if (0 == (VIEWERWIDGET_FLAG_FIX_LABEL_POS & widget->flags))
    {
        if (text)
        {
            label_len = strlen(text);
            if (0 < widget->textClip)
            {
                label_len = widget->textClip; 
            }
        }
        
        if (VIEWERWIDGET_CLASS_BUTTON == WIDGETCLASS(widget))
        {
            int max_char_width = 0;
            int max_char_height = 0;
            int font_baseline = 0;
            viewerStringSize(NULL, 0, NULL, NULL, &max_char_width, &max_char_height, &font_baseline);

            if (widget->flags & VIEWERWIDGET_FLAG_LEFT_ALIGN)
            {
                widget->label_x = 1;        
                widget->label_y = font_baseline;                    
            }
            else
            {
                widget->label_x = (widget->width - (label_len * max_char_width))/2;        
                widget->label_y = (widget->height - max_char_height)/2 + font_baseline;        
            }
        }
        else
        {
            if (VIEWERWIDGET_TYPE_INPUT == widget->wtype)
            {
                int width = 0;
                int height = 0;
                viewerStringSize(text, label_len, &width,&height,NULL,NULL,NULL);
                
                widget->label_x = - width - 5;
                widget->label_y = height;            
            }
            else
            {
                int max_char_height = 0;
                viewerStringSize(NULL, 0, NULL, NULL, NULL, &max_char_height, NULL);
                
                widget->label_x = 10;
                widget->label_y = max_char_height;
            }
        }    
    }
    
    return 0;
}

int widgetSizeWidget(ViewerWidget *widget, int width, int height)
{
    if (isObject(widget))
    {    
        if (0 <= width)
        {
            widget->width = width;
        }
        else
        if (WIDGET_TEXT_SIZE == width)
        {
            const char *text = widget->name;
            if (VIEWERWIDGET_FLAG_USE_TEXT & widget->flags)
            {
                text = widget->text;
            }
            viewerStringSize(text, strlen(text),&width, NULL, NULL, NULL,NULL);
            widget->width = width;
        }
        
        if (0 <= height)
        {
            widget->height = height;
        }
        
        widgetPlaceLabel(widget);
        
        int err = widgetSendEvent(VIEWEREVENT_RESIZE,widget, widget->width, widget->height);  
        
        widgetInvalidate(widget);        
        
        return err;
    }
    
    return 1;
}

int widgetSetName(ViewerWidget* widget, const char* name)
{
    if (widget && name)
    {
        widget->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;
        
        widget->textClip = 0;
        strncpy(widget->name,name,sizeof(widget->name));
        
        widgetPlaceLabel(widget);
        return 0;
    }
    
    return -1;
}


ViewerWidget* widgetCreateWidget(ViewerWidget *parent, const char* name, int type, int x, int y, int width, int height, ViewerCallback callback, void *data)
{
    ViewerWidget *widget = (ViewerWidget*)memory_aloc(sizeof(ViewerWidget));
    memset(widget,0,sizeof(ViewerWidget));
    
    int index = 0;
    if (NULL == parent)
    {
        index = _top_level_widget_index++;
    }
    objectInit((GObject*)widget, (GObject *)parent, index, OBJ_WIDGET);

    widget->wtype = type;
    widget->width = width;
    widget->height = height;
    widget->data = data;
    
    widgetSetName(widget,name);

    widget->color = aperalGetColor(GAPERAL_WIDGET);
    widget->frame_color = widget->color;
    
    // Create the widget index
    objectGetIndex((GObject*)widget);
    
    // Initialy create invisible
    widget->flags |= VIEWERWIDGET_FLAG_INVISIBLE;
    
    if (isObject(parent))
    {
         widgetAddChild(((ViewerWidget*)parent), widget);
         
    }
    widget->callback = callback;
        
    // fprintf(stderr,"Label %s at (%i,%i) base %i\n",widget->name,widget->label_x ,widget->label_y , font_baseline);    
    
    if (NULL == _viewer_widget_last)
    {
        _viewer_widget_last = widget;
        _viewer_widget_first = widget;
    }
    else
    {
        ViewerWidget *prev = _viewer_widget_last;
        _viewer_widget_last = widget;
        prev->next = (GObject*)widget;        
    }
    
    widgetMoveWidget(widget, x, y);

    return widget;
}

int widgetRemoveWidget(ViewerWidget *widget)
{
    if (isGObject(OBJ_WIDGET,widget))
    {
        ViewerWidget *prev = NULL;
        for (ViewerWidget *_widget = _viewer_widget_first; NULL != _widget;)
        {
            ViewerWidget *next =  (ViewerWidget*)isGObject(OBJ_WIDGET,_widget->next);
            if (widget == _widget)
            {
                if (_widget == _viewer_widget_first)
                {
                    _viewer_widget_first = (ViewerWidget*)isGObject(OBJ_WIDGET,_widget->next);
                }
                if (_widget == _viewer_widget_last)
                {
                    _viewer_widget_last = prev;
                }

                if (prev)
                {
                    prev->next = _widget->next;
                }

                widgetSendEvent(VIEWEREVENT_DESTROY,widget,0,0);
                
                if (widget->childs)
                {
                    // Remove my childs too
                    for (int i = 0; i < widget->number_of_childs;i++)
                    {
                        ViewerWidget *child = widget->childs[i];
                        if (child)
                        {
                            widget->number_of_childs--;
                            widgetRemoveWidget(child);
                        }
                    }

                    memory_free(widget->childs);
                }

                widget->magic = 0;
                memory_free(widget);
            }
            else
            if (_widget && (((ViewerWidget*)_widget->parent) == widget))
            {
                // I can not be a parent for any child
                _widget->parent = NULL;
            }
            else
            {
                // Remove myself from all parents
                for (int i = 0; i < _widget->number_of_childs;i++)
                {            
                    ViewerWidget *child = _widget->childs[i];
                    if (child == widget)
                    {
                        _widget->number_of_childs--;
                        _widget->childs[i] = NULL;
                    }
                }
            }
            
            prev = _widget;
            _widget = next;
        }    
        
        viewerInvalidate();
    }
    
    return 0;
}

int widgetRemoveTempWidgets()
{
    for (ViewerWidget *_widget = _viewer_widget_first; NULL != _widget; )
    {
        if (_widget->flags & VIEWERWIDGET_FLAG_TEMPORARY)
        {
            ViewerWidget *next = (ViewerWidget *)isGObject(OBJ_WIDGET,_widget->next);
            widgetRemoveWidget(_widget);
            _widget = next;
            continue;
        }
        _widget = (ViewerWidget*)isGObject(OBJ_WIDGET,_widget->next);
    }    
    
    viewerInvalidate();

    return 0;
}

int widgetClearWidget(ViewerWidget *widget)
{
    if (ISWIDGET_VISIBLE(widget))
    {
        if (0 == (widget->flags & VIEWERWIDGET_FLAG_CLEARED))
        {            
            if (VIEWERWIDGET_CLASS_WINDOW == WIDGETCLASS(widget))
            {
                    XClearArea(_viewerDisplay, _viewerMainWindow,  
                            widget->x - 1, widget->y - 1, 
                            widget->width + 2,
                            widget->height + 2,
                            False);
                    
            }
            else
            if (VIEWERWIDGET_CLASS_INPUT == WIDGETCLASS(widget))
            {
                unsigned long color = aperalGetColor(GAPERAL_INPUT_BACKGROUND);
                
                int x = widget->x;
                int y = widget->y;
                int width = widget->width;
                int height = widget->height;
                if (0 == (widget->flags & VIEWERWIDGET_FLAG_NOFRAME))
                {
                    x += 2;
                    y += 2;
                    width -= 4;
                    height -= 4;
                }
                
                viewerFillRectanlge(x, y, 
                            width,
                            height,
                            color);
            }
            else
            if (VIEWERWIDGET_CLASS_BUTTON == WIDGETCLASS(widget))
            {
                if (widget->flags & VIEWERWIDGET_FLAG_IS_PRESSED)
                {
                    viewerFillRectanlge(widget->x, widget->y, 
                            widget->width,
                            widget->height,
                            aperalGetColor(GAPERAL_WIDGET));
                }
                else
                {
                    /*
                    int width = widget->width;
                    if (VIEWERWIDGET_TYPE_TAP_BUTTON == widget->wtype)
                    {
                        width -= 2;
                    }
                    */

                    XClearArea(_viewerDisplay, _viewerMainWindow,  
                                widget->x, widget->y, 
                                widget->width,
                                widget->height,
                                False);                    
                }
            }
            
            widget->line_count = 0;
            widget->flags |= VIEWERWIDGET_FLAG_CLEARED;
        }
    }
    
    return 0;
}

int _widgetDrawFrame(ViewerWidget *widget)
{
    if ((0 == (widget->flags & VIEWERWIDGET_FLAG_NOFRAME)) &&
        ISWIDGET_VISIBLE(widget))
    {
        unsigned long color = widget->frame_color;

        if (ISWIDGET_DISABLED(widget))
        {
            color = aperalGetColor(GAPERAL_WIDGET_DISABLE);
        }

        if (ISWIDGET_ESCAPE(widget))
        {
            color = aperalGetColor(GAPERAL_WIDGET_HIGHLIGHT);
        }
        
        viewerSetDrawingColor(color);
        if (VIEWERWIDGET_TYPE_TAP_BUTTON == widget->wtype)
        {
            viewerDrawLine(widget->x, widget->y, widget->x + widget->width, widget->y);
            viewerDrawLine(widget->x, widget->y, widget->x, widget->y + widget->height);
            viewerDrawLine(widget->x, widget->y+ widget->height, widget->x+ widget->width, widget->y + widget->height);
        }
        else
        if (VIEWERWIDGET_TYPE_TAP_WINDOW == widget->wtype)
        {
            viewerDrawLine(widget->x, widget->y, widget->x + widget->width, widget->y);                        
            viewerDrawLine(widget->x, widget->y + widget->height, widget->x+ widget->width, widget->y + widget->height);            
            viewerDrawLine(widget->x+ widget->width, widget->y, widget->x+ widget->width, widget->y + widget->height);  

            ViewerWidget *button = widget->sibling;
            
            if (button)
            {
                viewerDrawLine(widget->x, button->y + button->height, widget->x, widget->y + widget->height);
                viewerDrawLine(widget->x, button->y, widget->x, widget->y);
            }
        }
        else
        {
            if (widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS)
            {
                viewerSetDrawingColor(aperalGetColor(GAPERAL_INPUT_FOCUS));
            }
            
            viewerDrawRectangle(widget->x,widget->y,widget->width,widget->height);        
            // fprintf(stderr,"Drawing %s in %lX at (%i,%i)\n",widget->name,widget->frame_color,widget->x,widget->y);
                
            if (WIDGETCHECKMARK(widget))
            {
                viewerDrawLine(widget->x + 15, widget->y, widget->x, widget->y + 15);
            }
        }
        
        widgetInvalidate(widget);
    }
    
    return 0;
}

int widgetDrawWidget(ViewerWidget *widget)
{
    if (widget && ISWIDGET_VISIBLE(widget))
    {            
        // widgetInvalidate(widget);                
        widgetClearWidget(widget);
        
        if (0 == (widget->flags & VIEWERWIDGET_FLAG_NOFRAME))
        {
            _widgetDrawFrame(widget);
        }
        
        color_type color = widget->color;
        if (ISWIDGET_ESCAPE(widget))
        {
            color = aperalGetColor(GAPERAL_WIDGET_HIGHLIGHT);
        }
        
        if (ISWIDGET_DISABLED(widget))
        {
            color = aperalGetColor(GAPERAL_WIDGET_DISABLE);
        }
                        

        const char * text = widget->name;
        if (widget->flags & VIEWERWIDGET_FLAG_USE_TEXT)
        {
            text = widget->text;
        }
        
        if (text)
        {
            color_type _color = color;
            if (VIEWERWIDGET_CLASS_BUTTON == WIDGETCLASS(widget))
            {
                if (widget->flags & VIEWERWIDGET_FLAG_IS_PRESSED)
                {
                    _color = GVCOLOR_BLACK;
                }
                else
                if (widget->flags & VIEWERWIDGET_FLAG_HIGHLIGHTED)
                {
                    _color = aperalGetColor(GAPERAL_TEXT_HIGHLIGHT);
                }
            }

            viewerSetDrawingColor(_color);  
            int textLen = strlen(text);
            if (0 < widget->textClip)
            {
                textLen = widget->textClip;
            }
            widgetDrawTextWidget(widget, widget->label_x, widget->label_y, text, textLen);
        }
        
        viewerSetDrawingColor(color);

        for (int i = 0; i < widget->number_of_childs;i++)
        {
            ViewerWidget *_widget = widget->childs[i];
            widgetDrawWidget(_widget);
        }
        
        viewerSetDrawingColor(color);
        if (0 == _viewerInsideEventLoop)
        {
            widgetSendEvent(VIEWEREVENT_EXPOSURE, widget,0,0);        
        }
    }
    
    return 0;
}

int widgetVisible(ViewerWidget *widget)
{
    if (widget && (0 == (widget->flags & VIEWERWIDGET_FLAG_MANUAL_VISIBILITY)))
    {    
        widget->flags &= ~VIEWERWIDGET_FLAG_INVISIBLE;
        widget->flags &= ~VIEWERWIDGET_FLAG_CLEARED;
        widgetPlaceLabel(widget);
        widgetInvalidate(widget);        

        for (int i = 0; i < widget->number_of_childs;i++)
        {
            if (widget->childs[i])
            {
                widgetVisible(widget->childs[i]); 
            }
        }
        
        widgetDrawWidget(widget);        
    }
    
    return 0;
}

int widgetInvisible(ViewerWidget *widget)
{
    if (widget && (0 == (widget->flags & VIEWERWIDGET_FLAG_MANUAL_VISIBILITY)))
    {
        widgetClearWidget(widget);
        
        widget->flags |= VIEWERWIDGET_FLAG_INVISIBLE;
        widget->flags &= ~VIEWERWIDGET_FLAG_ESCAPE;
                
        if (widget->childs)
        {
            for (int i = 0; i < widget->number_of_childs;i++)
            {
                if (widget->childs[i])
                {
                    widgetInvisible(widget->childs[i]);
                }
            }
        }
        
        viewerInvalidate();
    }
    
    return 0;
}


int widgetDrawWidgets()
{
    for (ViewerWidget *widget = _viewer_widget_first; NULL != widget; widget = (ViewerWidget*)widget->next)
    {
        if (ISWIDGET_VISIBLE(widget))
        {
            if (NULL == widget->parent)
            {                
                widgetDrawWidget(widget);
            }
        }
    }

    return 0;
}

int widgetAddChild(ViewerWidget *widget, ViewerWidget *child)
{
    if (isGObject(OBJ_WIDGET,widget) && isGObject(OBJ_WIDGET,child))
    {
        if (widget->max_number_of_childs <= widget->number_of_childs)
        {
            int old_size = widget->number_of_childs;
            widget->max_number_of_childs += 5;
            widget->childs = (ViewerWidget **)memory_realloc(widget->childs, sizeof(ViewerWidget *)*widget->max_number_of_childs);                        
            memset(&widget->childs[widget->number_of_childs],0,(widget->max_number_of_childs - old_size) * sizeof(ViewerWidget *));            
        }
        widget->childs[widget->number_of_childs] = child;
        widget->number_of_childs++;
        
         if (ISWIDGET_INVISIBLE(widget))
         {
             child->flags |= VIEWERWIDGET_FLAG_INVISIBLE;
         }
        
        if (NULL == child->parent)
        {
            if ((NULL != child->parent) && (widget != (ViewerWidget *)child->parent))
            {
                viewerPrintError(0, "Warning: replacing parent on \"%s\"\n", child->name);
            }
            
            child->parent = (GObject*)widget;
        }
        widgetInvalidate(widget);        
    }
    
    return 0;
}

int widgetAddSibling(ViewerWidget *widget, ViewerWidget *sibling)
{
    if (isGObject(OBJ_WIDGET,widget) && isGObject(OBJ_WIDGET,sibling))
    {
        if (widget->sibling)
        {
            LOG("Warning: Raplacing sibling on W%i\n",widget->index);
        }
        widget->sibling = sibling;
        return 0;
    }    
    
    return -1;
}


int widgetDrawTextWidget(ViewerWidget *widget, int x, int y, const char* text, int length)
{
    if (ISWIDGET_VISIBLE(widget) && text)
    {
        int err = 1;
        int x1[2] = { widget->x+x, widget->y+y };

        int max_char_height = 0;
        viewerStringSize(NULL, 0, NULL, NULL, NULL, &max_char_height, NULL);        
        
        char textBuffer[256];
        char *tbuff = textBuffer;
        *tbuff = 0;
        if (0 >= length)
        {
            length = strlen(text);
        }
        
        for(const char *s = text; (s - text < length) && *s; s++)
        {
            if ('\n' == *s)
            {
                *tbuff = 0;
                if (textBuffer[0])
                {
                    err = XDrawString(_viewerDisplay, _viewerMainWindow, _viewerGC,
                        x1[0], x1[1], textBuffer, strlen(textBuffer));            
                }                
                x1[1] += max_char_height;
                tbuff = textBuffer;
                *tbuff = 0;
            }
            else
            {
                *tbuff = *s;
                tbuff++;
                *tbuff = 0;
            }
        }
        
        if (textBuffer[0])
        {
            *tbuff = 0;
            err = XDrawString(_viewerDisplay, _viewerMainWindow, _viewerGC,
                    x1[0], x1[1], textBuffer, strnlen(textBuffer,sizeof(textBuffer)));
        }

        widget->flags &= ~VIEWERWIDGET_FLAG_CLEARED;
        
        if (0 > err)
        {
            viewerPrintError(0, "Draw text failed\n");
            return -1;
        }
        
        // fprintf(stderr,"Text: \"%s\" at (%i,%i)\n",text,x,y);
    }
    
    return 0;
}

int widgetFocus(ViewerWidget* widget)
{
    ViewerWidget* focused = NULL;
    
    int wcount = 0; // in case we are circling
    for (focused = widget; focused; focused = focused->sibling)
    {
        wcount++;
        if (!ISWIDGET_DISABLED(focused) || (100 < wcount))
        {
            break;
        }
    }
    
    if (focused && !ISWIDGET_DISABLED(focused))
    {
        for (ViewerWidget *_widget = _viewer_widget_first; NULL != _widget; _widget = (ViewerWidget*)_widget->next)
        {
            if (ISWIDGET_VISIBLE(widget) &&
                (VIEWERWIDGET_CLASS_INPUT == WIDGETCLASS(widget)) )
            {
                if (focused == _widget)
                {
                    if (0 == (focused->flags & VIEWERWIDGET_FLAG_HAS_FOCUS))
                    {
                        focused->flags |= VIEWERWIDGET_FLAG_HAS_FOCUS;
                        _widgetDrawFrame(focused);
                        widgetSendEvent(VIEWEREVENT_FOCUS, focused, 0, 0);                
                    }
                    widgetInvalidate(focused);
                }
                else
                {
                    if (_widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS)
                    {                
                        _widget->flags &= ~VIEWERWIDGET_FLAG_HAS_FOCUS;
                        _widgetDrawFrame(_widget);
                        widgetSendEvent(VIEWEREVENT_FOCUS_LOST, _widget, 0, 0);
                    }
                }
            }
        }
        
        return 0;
    }
    
    return -1;
}

Bool _widgetHandleWidgets(XEvent *event)
{
    int catched = 0;
    int needs_repaint = 0;
    
    for (ViewerWidget *widget = _viewer_widget_first; (0 == catched) && NULL != widget; widget = (ViewerWidget*)isGObject(OBJ_WIDGET,widget->next))
    {
        // if (widget->callback)
        {
            const int px = event->xbutton.x;                       
            const int py = event->xbutton.y;
            const int x1 = widget->x;
            const int y1 = widget->y;
            
            if (Expose == event->type)
            {
                if (ISWIDGET_VISIBLE(widget))
                {
                    widgetInvalidate(widget);
                    catched = 1;
                    needs_repaint = 1;
                }
            }            
            else
            if (VIEWERWIDGET_CLASS_INPUT == WIDGETCLASS(widget))
            {
                if (ISWIDGET_VISIBLE(widget) && !ISWIDGET_DISABLED(widget))
                {
                    if (ButtonPress == event->type)
                    {
                        const Bool xi = ((px > x1) && (px < x1+widget->width));
                        const Bool yi = ((py > y1) && (py < y1+widget->height));
                        
                        if (xi && yi)
                        {
                            if (widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS)
                            {
                                catched = 1;
                            }
                            else
                            {
                                // fprintf(stderr,"Sending focus W%i\n",widget->index);
                                widget->flags |= VIEWERWIDGET_FLAG_HAS_FOCUS;
                                _widgetDrawFrame(widget);
                                widgetSendEvent(VIEWEREVENT_FOCUS, widget, 0, 0);
                            }
                        }
                        else
                        {
                            if (widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS)
                            {
                                // fprintf(stderr,"Retracting focus W%i\n",widget->index);
                                widget->flags &= ~VIEWERWIDGET_FLAG_HAS_FOCUS;
                                _widgetDrawFrame(widget);
                                // Don't catch the event here. Its not in my window
                                widgetSendEvent(VIEWEREVENT_FOCUS_LOST, widget, 0, 0);
                            }
                        }
                    }
                    else
                    if (KeyPress == event->type)
                    {
                        int index = 0;
                        if (event->xkey.state & (ShiftMask))
                        {
                            index = 1;
                        }
                        if (event->xkey.state & (LockMask))
                        {
                            index = 2;
                        }                        
                        KeySym sym = XLookupKeysym(&event->xkey, index);
                        
                        // XLookupString(&event->xkey, keyBuffer, sizeof(keyBuffer), NULL, NULL);
                        //long sym = (int)keyBuffer[0];
#ifdef _DEBUG_KEY                        
                        LOG("Sending key %lX (%i,%X) to W%i\n",sym,event->xkey.state,event->xkey.state,widget->index);
#endif                        
                        catched = widgetSendEvent(VIEWEREVENT_KEY, widget,sym, event->xkey.state);
                    }                               
                }
            }
            else
            if (VIEWERWIDGET_CLASS_BUTTON == WIDGETCLASS(widget))
            {
                if (ISWIDGET_VISIBLE(widget) && !ISWIDGET_DISABLED(widget))
                {
                    if (ButtonPress == event->type)
                    {
                        const Bool xi = ((px > x1) && (px < x1+widget->width));
                        const Bool yi = ((py > y1) && (py < y1+widget->height));
                        
                        if (xi && yi)
                        {
                            widget->flags |= VIEWERWIDGET_FLAG_IS_PRESSED;
                            widgetDrawWidget(widget);                            
                            catched = widgetSendEvent(VIEWEREVENT_BUTTONPRESS, widget,px-x1,py-y1);
                        }
                    }
                    else
                    if (ButtonRelease == event->type)
                    {
                        const Bool xi = ((px > x1) && (px < x1+widget->width));
                        const Bool yi = ((py > y1) && (py < y1+widget->height));
                        
                        // Release events when window left ???
                        if (xi && yi)
                        {
                            widget->flags &= ~VIEWERWIDGET_FLAG_IS_PRESSED;
                            
                            if (VIEWERWIDGET_TYPE_CHECK == widget->wtype)
                            {
                                widget->flags ^= (VIEWERWIDGET_FLAG_SET);
                            }

                            widgetDrawWidget(widget);                            
                            catched = widgetSendEvent(VIEWEREVENT_BUTTONRELEASE, widget,px-x1,py-y1);
                        }
                    }
                }
            }
        }
        
        // Once the event has been catched the widget list could have been changed. 
        // So don't touch the list anymore!
        if (catched)
        {
            break;
        }
    }  

    if (needs_repaint)
    {
        // ???: needed? just invalidate?
        widgetDrawWidgets();
    }

    if (catched > 1)
    {
        viewerInvalidate();
    }
    
    return catched;
}

int widgetEnable(ViewerWidget *widget, int enable)
{
    if (isObject(widget))
    {
        if (enable)
        {
            ENABLE_WIDGET(widget);
            widgetInvalidate(widget);        
        }
        else
        {
            widget->flags &= ~VIEWERWIDGET_FLAG_ESCAPE;
            DISABLE_WIDGET(widget);
        }
        return 0;
    }
    return -1;
}

void widgetEnableDesktop(int enable, ViewerWidget *exclude)
{    
    for (ViewerWidget *_widget = _viewer_widget_first; NULL != _widget; _widget = (ViewerWidget*)isGObject(OBJ_WIDGET,_widget->next))
    {
        // Disable all top level widgets
        if ((NULL == _widget->parent) && (_widget != exclude))
        {
            widgetEnable(_widget,enable);
        }
    }    
}

void widgetInvalidate(ViewerWidget *widget)
{
    widget->flags &= ~VIEWERWIDGET_FLAG_CLEARED;
    
    // widgetSendEvent(VIEWEREVENT_EXPOSURE, widget,0,0);            
    
    viewerInvalidate();
}

int widgetSetText(ViewerWidget* widget, const char* text)
{
    if (isGObject(OBJ_WIDGET,widget))
    {
        if (text)
        {
            widget->text = text;
            widget->flags |= VIEWERWIDGET_FLAG_USE_TEXT;
            
            widgetPlaceLabel(widget);
        }
        else
        {
            widget->text = NULL;
            widget->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;            
        }
        
        return 0;
    }
    
    return -1;
}

ViewerWidget* widgetGetChild(ViewerWidget* widget, int index)
{
    if (widget)
    {
        if ((0 >= index) && (index < widget->number_of_childs))
        {
            return widget->childs[index];
        }
    }
    
    return NULL;
}

const char* widgetEventToText(int ev)
{
    static const char *events[] = { "BP", "BR", "EX", "RSZ", "MO", "DTY", "SEL", "KY", "FO", "FOL" };
    
    if ((0 < ev ) && (ev < (int)(sizeof(events)/sizeof(events[0]))))
    {
            return events[ev-1];
    }
    
    return "?";
}

int widgetSetTextPosition(ViewerWidget* widget, int x, int y)
{
    if (isGObject(OBJ_WIDGET,widget))
    {
        if ((0 <= x) && (0 <= y))
        {
            widget->label_x = x;
            widget->label_y = y;
        
            widget->flags |= VIEWERWIDGET_FLAG_FIX_LABEL_POS;
        }
        else
        {
            widget->label_x = 0;
            widget->label_y = 0;
        
            widget->flags &= ~VIEWERWIDGET_FLAG_FIX_LABEL_POS;            
            widget->textClip = 0;
        }
        
        return widgetPlaceLabel(widget);
    }
    
    return -1;
}


int widgetSetColor(ViewerWidget* widget, color_type font_color, color_type frame_color)
{
    if (widget)
    {
        if (GVCOLOR_TRANSPARENT != font_color)
        {
            widget->color = font_color;
            widget->flags |= VIEWERWIDGET_FLAG_COLOR_SET;
        }else{
            widget->color = aperalGetColor(GAPERAL_WIDGET);
            widget->flags &= ~VIEWERWIDGET_FLAG_COLOR_SET;
        }

        if (GVCOLOR_TRANSPARENT != frame_color)
        {
            widget->frame_color = frame_color;
            widget->flags |=VIEWERWIDGET_FLAG_FRAME_COLOR_SET;
        }else{
            widget->frame_color = aperalGetColor(GAPERAL_WIDGET);
            widget->flags &= ~VIEWERWIDGET_FLAG_FRAME_COLOR_SET;
        }
        
        return 0;
    }
    
    return 1;
 
}
