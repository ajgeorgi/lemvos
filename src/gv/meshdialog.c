#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* MeshDialog for GV 09.11.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"
#include "viewer.h"
#include "soliddialog.h"
#include "controller.h"
#include "tabsheet.h"
#include "input.h"
#include "configuration.h"
#include "modelcreator.h"
#include "meshdialog.h"


#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 20
#define BUTTON_INCREMENT 25
#define BUTTON_X 10
#define BUTTON_START_Y 10

static ViewerWidget *saveButton = NULL;
#define SAVE_LABEL "Save"
#define SAVE_BUTTON_X BUTTON_X
#define SAVE_BUTTON_Y BUTTON_START_Y
#define SAVE_BUTTON_WIDTH BUTTON_WIDTH
#define SAVE_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *dropButton = NULL;
#define DROP_LABEL "Drop"
#define DROP_BUTTON_X BUTTON_X
#define DROP_BUTTON_Y BUTTON_START_Y
#define DROP_BUTTON_WIDTH BUTTON_WIDTH
#define DROP_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *visibleButton = NULL;
#define VISIBLE_LABEL "Visible"
#define VISIBLE_BUTTON_X BUTTON_X
#define VISIBLE_BUTTON_Y BUTTON_START_Y
#define VISIBLE_BUTTON_WIDTH BUTTON_WIDTH
#define VISIBLE_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *volumeButton = NULL;
#define VOLUME_LABEL "Volume"
#define VOLUME_BUTTON_X BUTTON_X
#define VOLUME_BUTTON_Y BUTTON_START_Y
#define VOLUME_BUTTON_WIDTH BUTTON_WIDTH
#define VOLUME_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *normaleButton = NULL;
#define NORMALE_LABEL "Normale"
#define NORMALE_BUTTON_X BUTTON_X
#define NORMALE_BUTTON_Y BUTTON_START_Y
#define NORMALE_BUTTON_WIDTH BUTTON_WIDTH
#define NORMALE_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *BBButton = NULL;
#define BB_LABEL "BB visible"
#define BB_BUTTON_X BUTTON_X
#define BB_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define BB_BUTTON_WIDTH BUTTON_WIDTH
#define BB_BUTTON_HEIGHT BUTTON_HEIGHT


static ViewerWidget *dialog = NULL;
#define DIALOG_LABEL "VolumeEditor"
#define DIALOG_BUTTON_X BUTTON_X
#define DIALOG_BUTTON_Y BUTTON_START_Y
#define DIALOG_BUTTON_WIDTH BUTTON_WIDTH
#define DIALOG_BUTTON_HEIGHT BUTTON_HEIGHT

static char meshName[80];

static DialogUserChoice  meshedit_choice_visible = NULL;
static Mesh *_meshdialog_mesh = NULL;
static int meshDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static int position(ViewerWidget *widget);

void initMeshDialog()
{
    if (NULL == dialog)
    {
        dialog = widgetCreateWidget(NULL,DIALOG_LABEL,VIEWERWIDGET_TYPE_WINDOW,DIALOG_BUTTON_X,DIALOG_BUTTON_Y,DIALOG_BUTTON_WIDTH,DIALOG_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);

        saveButton = widgetCreateWidget(dialog,SAVE_LABEL,VIEWERWIDGET_TYPE_BUTTON,SAVE_BUTTON_X,SAVE_BUTTON_Y,SAVE_BUTTON_WIDTH,SAVE_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);
        
        dropButton = widgetCreateWidget(dialog, DROP_LABEL, VIEWERWIDGET_TYPE_BUTTON,DROP_BUTTON_X,DROP_BUTTON_Y,DROP_BUTTON_WIDTH,DROP_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);
        
        visibleButton  = widgetCreateWidget(dialog,VISIBLE_LABEL, VIEWERWIDGET_TYPE_CHECK,VISIBLE_BUTTON_X,VISIBLE_BUTTON_Y,VISIBLE_BUTTON_WIDTH,VISIBLE_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);

        volumeButton  = widgetCreateWidget(dialog,VOLUME_LABEL, VIEWERWIDGET_TYPE_CHECK,VOLUME_BUTTON_X,VOLUME_BUTTON_Y,VOLUME_BUTTON_WIDTH,VOLUME_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);

        normaleButton  = widgetCreateWidget(dialog,NORMALE_LABEL, VIEWERWIDGET_TYPE_CHECK,NORMALE_BUTTON_X,NORMALE_BUTTON_Y,NORMALE_BUTTON_WIDTH,NORMALE_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);
        
        BBButton = widgetCreateWidget(dialog,BB_LABEL, VIEWERWIDGET_TYPE_CHECK,BB_BUTTON_X,BB_BUTTON_Y,BB_BUTTON_WIDTH,BB_BUTTON_HEIGHT,
                                    meshDialogCallback,NULL);
        widgetInvisible(dialog);
    }
}

int meshEdit(CObject *object, DialogUserChoice callback)
{
    Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
    
    if (mesh)
    {
        meshedit_choice_visible = callback;
        _meshdialog_mesh = mesh;
        
        widgetEnableDesktop(0, dialog);

         meshName[0] = 0;
         strncpy(meshName,mesh->name,sizeof(meshName));
        dialog->data = mesh;
        
        widgetSetText(dialog,meshName);    
        
        dialog->frame_color = aperalGetColor(GAPERAL_MESH);
        if (mesh->material)
        {
            dialog->frame_color = mesh->material->color;
        }

        widgetSetCheck(visibleButton, cobjectIsVisible(_meshdialog_mesh));    
        
        
        widgetSetCheck(volumeButton, mesh->flags & GM_GOBJECT_FLAG_SHOW_TRI);
        widgetSetCheck(normaleButton, mesh->flags & GM_GOBJECT_FLAG_SHOW_NORM);
        widgetSetCheck(BBButton, mesh->flags & GM_GOBJECT_FLAG_SHOW_BBOX);
        
        position(dialog);
        
        plotterLockSelection(1);
        widgetVisible(dialog);                
    }
    
    return 0;
}

int meshEditClear()
{
   _meshdialog_mesh = NULL;
   meshedit_choice_visible = NULL;
   
   return 0;
}

// ???: This function is not neccessary if widgets using relative coordiantes and drawn by the widget handling
// in case of resize. But we might need some kind of smart placement of widgets 
// they also need a uniform coordinate system

static int position(ViewerWidget *widget)
{
    int width,height;
    
    viewerGetWindowSize(&width, &height);
#ifdef _DEBUG_MESHDIA
    LOG("Placing widgets on W%i\n",widget->index);
#endif
    
    const int pwidth = width /3*2;
    const int pheight = height/3*2;
    const int x = (width - pwidth) / 2;
    const int y = (pheight/3);

    widgetSizeWidget(widget, pwidth, pheight);
    widgetMoveWidget(widget,x, y);

    int bt_x = 10;
    int bt_y = pheight - saveButton->height - 10;

    widgetMoveWidget(saveButton, bt_x, bt_y);

    bt_x = pwidth - dropButton->width - 10;
    bt_y = pheight - dropButton->height - 10;

    widgetMoveWidget(dropButton, bt_x, bt_y);
    dropButton->flags |= VIEWERWIDGET_FLAG_ESCAPE;
    
    bt_x = 10;
    bt_y = 30;

    widgetMoveWidget(visibleButton, bt_x, bt_y);

    bt_x = 20 + visibleButton->width;
    bt_y = 30;

    widgetMoveWidget(volumeButton, bt_x, bt_y);
    
    bt_x = 30 + visibleButton->width + volumeButton->width;
    bt_y = 30;

    widgetMoveWidget(normaleButton, bt_x, bt_y);
    
    bt_x = 40 + visibleButton->width + volumeButton->width + normaleButton->width;
    bt_y = 30;

    widgetMoveWidget(BBButton, bt_x, bt_y);    
    
    return 0;
}

int meshDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;

#ifdef _DEBUG_MESHDIA    
    LOG("M:EV(W%i) = %i, (%i,%i)\n",w->index,ev,x,y);
#endif
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == saveButton))
    {
        cobjectSetVisibility(_meshdialog_mesh,WIDGETCHECKMARK(visibleButton));
        
        if (WIDGETCHECKMARK(volumeButton))
        {
            _meshdialog_mesh->flags |= GM_GOBJECT_FLAG_SHOW_TRI;
        }
        else
        {
            _meshdialog_mesh->flags &= ~GM_GOBJECT_FLAG_SHOW_TRI;
        }

        if (WIDGETCHECKMARK(normaleButton))
        {
            _meshdialog_mesh->flags |= GM_GOBJECT_FLAG_SHOW_NORM;
        }
        else
        {
            _meshdialog_mesh->flags &= ~GM_GOBJECT_FLAG_SHOW_NORM;
        }

        if (WIDGETCHECKMARK(BBButton))
        {
            _meshdialog_mesh->flags |= GM_GOBJECT_FLAG_SHOW_BBOX;
        }
        else
        {
            _meshdialog_mesh->flags &= ~GM_GOBJECT_FLAG_SHOW_BBOX;
        }
        
        if (meshedit_choice_visible)
        {
            meshedit_choice_visible(DIALOG_CHOICE_SAVE,(CObject*)_meshdialog_mesh);
        }
        meshedit_choice_visible = NULL;

        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);

        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == dropButton))
    {
        if (meshedit_choice_visible)
        {
            meshedit_choice_visible(DIALOG_CHOICE_DROP,(CObject*)_meshdialog_mesh);
        }
        meshedit_choice_visible = NULL;
        
        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);
        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_EXPOSURE == ev) && (w == dialog))
    {
        Mesh * mesh = (Mesh*)isCObject(OBJ_MESH,(GObject*)w->data);
        if (mesh)
        {
            unsigned long color = aperalGetColor(GAPERAL_MESH);

            if (mesh->material)
            {
                color = mesh->material->color;
            }
            
            viewerFillRectanlge(dialog->x + dialog->width - 30 , dialog->y +10, 20, 20, color);

            viewerSetDrawingColor(aperalGetColor(GAPERAL_FOREGROUND));
        }
        
        return 1;
    }

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == visibleButton))
    {
        return 1; // consume event
    }

    return 0;
}
