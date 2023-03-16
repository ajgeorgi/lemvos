#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#include "modeldia.h"

#include "widget.h"
#include "viewer.h"
#include "controller.h"
#include "tabsheet.h"
#include "input.h"
#include "configuration.h"
#include "modelcreator.h"
#include "combobox.h"


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



static ViewerWidget *dialog = NULL;
#define DIALOG_LABEL "ModelEditor"
#define DIALOG_BUTTON_X BUTTON_X
#define DIALOG_BUTTON_Y BUTTON_START_Y
#define DIALOG_BUTTON_WIDTH BUTTON_WIDTH
#define DIALOG_BUTTON_HEIGHT BUTTON_HEIGHT

static TabSheet *tabsheet = NULL;
#define TABSHEET_LABEL "tabsheet"
#define TABSHEET_WINDOW_X BUTTON_X
#define TABSHEET_WINDOW_Y BUTTON_START_Y
#define TABSHEET_WINDOW_WIDTH BUTTON_WIDTH
#define TABSHEET_WINDOW_HEIGHT BUTTON_HEIGHT




static char modelName[80];

static DialogUserChoice  _modeldialog_choice_visible = NULL;
static Model *_modeldialog_model = NULL;
static int modelDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static int position(ViewerWidget *widget);


void initModelDialog()
{
    if (NULL == dialog)
    {
        dialog = widgetCreateWidget(NULL,DIALOG_LABEL,VIEWERWIDGET_TYPE_WINDOW,DIALOG_BUTTON_X,DIALOG_BUTTON_Y,DIALOG_BUTTON_WIDTH,DIALOG_BUTTON_HEIGHT,
                                    modelDialogCallback,NULL);

        saveButton = widgetCreateWidget(dialog,SAVE_LABEL,VIEWERWIDGET_TYPE_BUTTON,SAVE_BUTTON_X,SAVE_BUTTON_Y,SAVE_BUTTON_WIDTH,SAVE_BUTTON_HEIGHT,
                                    modelDialogCallback,NULL);
        
        dropButton = widgetCreateWidget(dialog, DROP_LABEL, VIEWERWIDGET_TYPE_BUTTON,DROP_BUTTON_X,DROP_BUTTON_Y,DROP_BUTTON_WIDTH,DROP_BUTTON_HEIGHT,
                                    modelDialogCallback,NULL);
        
        tabsheet = tabsheetCreate(dialog, "Tabsheet", TABSHEET_WINDOW_X, TABSHEET_WINDOW_Y, TABSHEET_WINDOW_WIDTH, TABSHEET_WINDOW_HEIGHT);

        
        widgetInvisible(dialog);
    }    
}


int modelEdit(CObject *object, DialogUserChoice callback)
{
    Model *model = (Model*)isCObject(OBJ_MODEL,object);
    
    if (model)
    {
        _modeldialog_choice_visible = callback;
        _modeldialog_model = model;
        
        widgetEnableDesktop(0, dialog);

         modelName[0] = 0;
         strncpy(modelName,model->name,sizeof(modelName));
        dialog->data = model;
        
        widgetSetText(dialog,modelName);    
        
        dialog->frame_color = aperalGetColor(GAPERAL_MODEL);
        if (model->material)
        {
            dialog->frame_color = model->material->color;
        }
        
//         char s1[GM_VERTEX_BUFFER_SIZE];                        
//         for (const CObject *object = vertexConstCObjectIterator(model,NULL,0);object; object = vertexConstCObjectIterator(model,object,0))
//         {    
//             LOG("Adding [%s] to table\n",vertexPath((GObject*)object,s1,sizeof(s1)));
//                         
//             for (const Mea* mea = (const Mea*)object->first_mea; mea; mea =  (const Mea*)mea->next)
//             {
//                 LOG("Adding [%s] to tabsheet\n",vertexPath((GObject*)mea,s1,sizeof(s1)));
//                 
//                 if (isCObject(OBJ_MEASUREMENT,mea))
//                 {
//                     Tab *modelTab = tabsheetAddTab(tabsheet, mea->name);
//                     modelEditCreateDataTable(mea->name, mea, modelTab);
//                 }
//             }
//         }
        
        position(dialog);
        
        plotterLockSelection(1);
        widgetVisible(dialog);                
    }
    
    return 0;    
}

int modelEditClear()
{
    _modeldialog_choice_visible = NULL;
    _modeldialog_model = NULL;

    return 0;
}


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
    
    tabsheetResize(tabsheet, 10, 70, pwidth - 20, pheight - 120);
    
/*    
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
    */
    return 0;
}

int modelDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;

#ifdef _DEBUG_MODELDIA    
    LOG("MODEL:EV(W%i) = %i, (%i,%i)\n",w->index,ev,x,y);
#endif
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == saveButton))
    {
        if (_modeldialog_choice_visible)
        {
            _modeldialog_choice_visible(DIALOG_CHOICE_SAVE,(CObject*)_modeldialog_model);
        }
        _modeldialog_choice_visible = NULL;

        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);

        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == dropButton))
    {
        if (_modeldialog_choice_visible)
        {
            _modeldialog_choice_visible(DIALOG_CHOICE_DROP,(CObject*)_modeldialog_model);
        }
        _modeldialog_choice_visible = NULL;
        
        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);
        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_EXPOSURE == ev) && (w == dialog))
    {
        Model * model = (Model*)isCObject(OBJ_MODEL,(GObject*)w->data);
        if (model)
        {
            unsigned long color = aperalGetColor(GAPERAL_MODEL);

            if (model->material)
            {
                color = model->material->color;
            }
            
            viewerFillRectanlge(dialog->x + dialog->width - 30 , dialog->y +10, 20, 20, color);

            viewerSetDrawingColor(aperalGetColor(GAPERAL_FOREGROUND));
        }
        
        return 1;
    }

    return 0;
}
