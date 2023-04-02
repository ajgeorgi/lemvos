#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Tabsheet for GV 21.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "tabsheet.h"
#include "viewer.h"


#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 20
#define BUTTON_INCREMENT 25
#define BUTTON_X 10
#define BUTTON_START_Y 10

#define TAB_LABEL "Tap"
#define TAB_BUTTON_X BUTTON_X
#define TAB_BUTTON_Y BUTTON_START_Y
#define TAB_BUTTON_WIDTH (BUTTON_WIDTH*2/3)
#define TAB_BUTTON_HEIGHT BUTTON_HEIGHT

#define TAB_WINDOW_X BUTTON_X
#define TAB_WINDOW_Y BUTTON_START_Y
#define TAB_WINDOW_WIDTH BUTTON_WIDTH
#define TAB_WINDOW_HEIGHT BUTTON_HEIGHT

static int tabsheetCallback(ViewerWidget *w, int ev, int x, int y, void *data);

TabSheet *tabsheetCreate(ViewerWidget *parent, const char* name, int x, int y, int width, int height)
{
    (void)name;
    TabSheet* tabsheet = (TabSheet*)memory_aloc(sizeof(TabSheet));
    memset(tabsheet,0,sizeof(TabSheet));
    
    objectInit((GObject *)tabsheet, (GObject *)parent, 0, OBJ_TABSHEET);

    tabsheet->window = widgetCreateWidget(parent, 
                                          NULL,
                                          VIEWERWIDGET_TYPE_WINDOW, 
                                          x, y, width, height, 
                                          tabsheetCallback, 
                                          tabsheet);
    
    tabsheet->window->flags |= VIEWERWIDGET_FLAG_NOFRAME;
    
    return tabsheet;
}

Tab *tabsheetAddTab(TabSheet *tabsheet, const char* name)
{
    if (isObject(tabsheet))
    {
        if (tabsheet->number_of_tabs < (int)(sizeof(tabsheet->tabs)/sizeof(tabsheet->tabs[0])))
        {
            Tab* tab = (Tab*)memory_aloc(sizeof(Tab));
            memset(tab,0,sizeof(Tab));

            tab->button = widgetCreateWidget(tabsheet->window, 
                                        name,
                                        VIEWERWIDGET_TYPE_TAP_BUTTON, 
                                        TAB_BUTTON_X, 
                                        TAB_BUTTON_Y, 
                                        TAB_BUTTON_WIDTH, 
                                        TAB_BUTTON_HEIGHT, 
                                        tabsheetCallback, 
                                        tabsheet);        

            tab->window = widgetCreateWidget(tabsheet->window, 
                                            name,
                                            VIEWERWIDGET_TYPE_TAP_WINDOW, 
                                            TAB_WINDOW_X, 
                                            TAB_WINDOW_Y, 
                                            TAB_WINDOW_WIDTH, 
                                            TAB_WINDOW_HEIGHT, 
                                            tabsheetCallback, 
                                            tabsheet);    
            
                            
            widgetAddSibling(tab->window,tab->button);
            
            tabsheet->tabs[tabsheet->number_of_tabs++] = tab;
                    
            if (tab != tabsheet->tabs[0])
            {
                tab->window->flags |= (VIEWERWIDGET_FLAG_INVISIBLE|VIEWERWIDGET_FLAG_MANUAL_VISIBILITY);
                
                tabsheetSetTab(tabsheet, tabsheet->tabs[0]);
            }
            
            tabsheetResize(tabsheet,  tabsheet->window->x,  tabsheet->window->y,  tabsheet->window->width,  tabsheet->window->height);

            return tab;
        }
        
        ERROR("Maximum number of tab sheets hast been reached on \"%s\"!\n",tabsheet->window->name);
    }
    
    return NULL;
}

Tab *tabsheetGetTab(TabSheet *tabsheet, int index)
{
    if (tabsheet)
    {
        if ((0 <= index) && (index < tabsheet->number_of_tabs))
        {
            return tabsheet->tabs[index];
        }
    }
    
    return NULL;    
}

void tabsheetResize(TabSheet *tabsheet, int x, int y, int width, int height)
{
    if (isObject(tabsheet))
    {
        widgetSizeWidget(tabsheet->window, width, height);
        widgetMoveWidget(tabsheet->window, x, y);
        
        for (int i = 0; i < tabsheet->number_of_tabs; i++)
        {                
            const int wwidth = width - TAB_BUTTON_WIDTH;
            const int wheigth = height;

            const int wx = TAB_BUTTON_WIDTH;
            const int wy = 0;
            widgetMoveWidget(tabsheet->tabs[i]->window, wx, wy);
            
            const int bwidth = TAB_BUTTON_WIDTH;
            const int bheigth = BUTTON_HEIGHT;

            const int bx =  0;
            const int by =  wy + (i * (BUTTON_HEIGHT + 5));

            widgetMoveWidget(tabsheet->tabs[i]->button, bx, by);
            
            widgetSizeWidget(tabsheet->tabs[i]->button, bwidth, bheigth);
            widgetSizeWidget(tabsheet->tabs[i]->window, wwidth, wheigth);        
        }
    }
}

int tabsheetSetTab(TabSheet *tabsheet, Tab *tab)
{
    if (isObject(tabsheet) && tab)
    {
        for (int i = 0; i < tabsheet->number_of_tabs; i++)
        {
            Tab *tabx = tabsheet->tabs[i];
            if (tabx)
            {
                if (tab == tabx)
                {
                    // We have to do it to keep stacking order of tabs
                    tabx->window->flags &= ~VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;
                    widgetVisible(tabx->window);
                }
                else
                {
                    widgetInvisible(tabx->window);
                    tabx->window->flags |= VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;                    
                }
            }
        }
        
        viewerInvalidate();    
    }
    
    return 0;
}


int tabsheetCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    
#ifdef _DEBUG_EVENTS    
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("TS:%s:%s:EV(%s):(%i,%i)\n",vertexPath((GObject*)w,s1,sizeof(s1)),w->name?w->name:"?",widgetEventToText(ev),x,y);
#endif    
    
    if (data && (ev == VIEWEREVENT_BUTTONRELEASE))
    {
        TabSheet *tabsheet = (TabSheet *)isGObject(OBJ_TABSHEET,data);
        
        for (int i = 0; i < tabsheet->number_of_tabs; i++)
        {
            if ((tabsheet->tabs[i]) && (w == tabsheet->tabs[i]->button))
            {
                tabsheetSetTab(tabsheet,tabsheet->tabs[i]);
                
                return 1;
            }
        }
    }
    
    return 0;
}
