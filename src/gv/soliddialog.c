#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* SolidDialog for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define __GOBJECT_PRIVATE
#include "gobject.h"
#undef __GOBJECT_PRIVATE

#include "widget.h"
#include "viewer.h"
#include "soliddialog.h"
#include "controller.h"
#include "tabsheet.h"
#include "input.h"
#include "configuration.h"
#include "modelcreator.h"
#include "combobox.h"
#include "modelstore.h"
#include "gmath.h"
#include "table.h"

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

static ViewerWidget *normButton = NULL;
#define NORM_LABEL "Norm"
#define NORM_BUTTON_X BUTTON_X
#define NORM_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define NORM_BUTTON_WIDTH BUTTON_WIDTH
#define NORM_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *bvvisButton = NULL;
#define VIS_BV_LABEL "Surface"
#define VIS_BV_BUTTON_X BUTTON_X
#define VIS_BV_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define VIS_BV_BUTTON_WIDTH BUTTON_WIDTH
#define VIS_BV_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *LFButton = NULL;
#define LF_LABEL "Last&First"
#define LF_BUTTON_X BUTTON_X
#define LF_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define LF_BUTTON_WIDTH BUTTON_WIDTH
#define LF_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *BBButton = NULL;
#define BB_LABEL "BB visible"
#define BB_BUTTON_X BUTTON_X
#define BB_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define BB_BUTTON_WIDTH BUTTON_WIDTH
#define BB_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *calcButton = NULL;
#define CALC_LABEL "No calc"
#define CALC_BUTTON_X BUTTON_X
#define CALC_BUTTON_Y (BUTTON_START_Y + BUTTON_WIDTH + 20)
#define CALC_BUTTON_WIDTH BUTTON_WIDTH
#define CALC_BUTTON_HEIGHT BUTTON_HEIGHT



static ViewerWidget *dialog = NULL;
#define DIALOG_LABEL "SolidEditor"
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


static DialogUserChoice  solidedit_choice_visible = NULL;
static CObject *_soliddialog_solid = NULL;
static Tab *modelTab = NULL;
static Tab *materialTab = NULL;
static Tab *dataTab = NULL;
static Tab *selectionTab = NULL;
static Tab *debugTab = NULL;
static Input* inputYaw = NULL;
static Input* inputRoll = NULL;
static Input* inputPitch = NULL;
static Input* inputHeave = NULL;

static Input* inputMaterialThicknesMax = NULL;
static Input* inputMaterialThicknesMin = NULL;

static ComboBox *materialSelect = NULL;

static TableWidget *dataTable = NULL;

static ViewerWidget *calcButtons[10];
static int number_of_calcbuttons = 0;

static char materialText[800];
static char commentText[1500];
static char solidName[50];
static char selectionText[1000];
static char buttonName[50];
static char modelText[100];

static int solidDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static int solidCalcCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static void comboMaterialChoice(struct ComboBoxT* combo, void *data, int selected_line);
static int solideditSetMaterial(Material *material);


static int solidEditMaterialUpdate(CObject *object);

static int addCalcButtons(CObject *object);

static void inputValidated(Input *input);

void initSolidDialog()
{
    if (NULL == dialog)
    {
        dialog = widgetCreateWidget(NULL,DIALOG_LABEL,VIEWERWIDGET_TYPE_WINDOW,DIALOG_BUTTON_X,DIALOG_BUTTON_Y,DIALOG_BUTTON_WIDTH,DIALOG_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);

        saveButton = widgetCreateWidget(dialog,SAVE_LABEL,VIEWERWIDGET_TYPE_BUTTON,SAVE_BUTTON_X,SAVE_BUTTON_Y,SAVE_BUTTON_WIDTH,SAVE_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);

        dropButton = widgetCreateWidget(dialog, DROP_LABEL, VIEWERWIDGET_TYPE_BUTTON,DROP_BUTTON_X,DROP_BUTTON_Y,DROP_BUTTON_WIDTH,DROP_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);

        visibleButton  = widgetCreateWidget(dialog,VISIBLE_LABEL, VIEWERWIDGET_TYPE_CHECK,VISIBLE_BUTTON_X,VISIBLE_BUTTON_Y,VISIBLE_BUTTON_WIDTH,VISIBLE_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);

        normButton  = widgetCreateWidget(dialog,NORM_LABEL, VIEWERWIDGET_TYPE_CHECK,NORM_BUTTON_X,NORM_BUTTON_Y,NORM_BUTTON_WIDTH,NORM_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);
        
        LFButton   = widgetCreateWidget(dialog,LF_LABEL, VIEWERWIDGET_TYPE_CHECK,LF_BUTTON_X,LF_BUTTON_Y,LF_BUTTON_WIDTH,LF_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);
        
        bvvisButton  = widgetCreateWidget(dialog,VIS_BV_LABEL, VIEWERWIDGET_TYPE_CHECK,VIS_BV_BUTTON_X,VIS_BV_BUTTON_Y,VIS_BV_BUTTON_WIDTH,VIS_BV_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);

        BBButton  = widgetCreateWidget(dialog,BB_LABEL, VIEWERWIDGET_TYPE_CHECK,BB_BUTTON_X,BB_BUTTON_Y,BB_BUTTON_WIDTH,BB_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);
        
        calcButton  = widgetCreateWidget(dialog,CALC_LABEL, VIEWERWIDGET_TYPE_CHECK,CALC_BUTTON_X,CALC_BUTTON_Y,CALC_BUTTON_WIDTH,CALC_BUTTON_HEIGHT,
                                    solidDialogCallback,NULL);
        
        tabsheet = tabsheetCreate(dialog, "Tabsheet", TABSHEET_WINDOW_X, TABSHEET_WINDOW_Y, TABSHEET_WINDOW_WIDTH, TABSHEET_WINDOW_HEIGHT);
        
        
        modelTab = tabsheetAddTab(tabsheet, NULL);
        materialTab = tabsheetAddTab(tabsheet, "Material");
        dataTab = tabsheetAddTab(tabsheet, "Data");
        selectionTab  = tabsheetAddTab(tabsheet, "Selection");
        debugTab  = tabsheetAddTab(tabsheet, "Debug");

        dataTable = tableCreateTable(debugTab->window, "Data", 2, 30);
        
        inputYaw = inputCreate(modelTab->window,100, 10, 100, inputValidated);
        inputRoll = inputCreate(modelTab->window,100, 60, 100, inputValidated);
        inputPitch = inputCreate(modelTab->window,100, 120, 100, inputValidated);
        inputHeave = inputCreate(modelTab->window,100, 180, 100, inputValidated);
                
        // Tab order
        widgetAddSibling(inputYaw->widget,inputRoll->widget);
        widgetAddSibling(inputRoll->widget,inputPitch->widget);
        widgetAddSibling(inputPitch->widget,inputHeave->widget);
        widgetAddSibling(inputHeave->widget,inputYaw->widget);

        inputMaterialThicknesMin = inputCreate(materialTab->window,100, 10, 100, inputValidated);
        inputMaterialThicknesMax = inputCreate(materialTab->window,100, 60, 100, inputValidated);        
        
        // Tab order
        widgetAddSibling(inputMaterialThicknesMin->widget,inputMaterialThicknesMax->widget);
        widgetAddSibling(inputMaterialThicknesMax->widget,inputMaterialThicknesMin->widget);
                
        materialSelect = comboCreateComboBox(materialTab->window, "Made of", BUTTON_WIDTH,BUTTON_HEIGHT, comboMaterialChoice);

        memset(calcButtons,0,sizeof(calcButtons));
        
        widgetInvisible(dialog);
    }
}

int solidSetupSolid(CObject *object)
{
    int changed = 0;
    
    double yaw = 0;
    if (inputIsValid(inputYaw))
    {
            yaw = inputGetNumber(inputYaw) / 180.0 * M_PI; 
            if (inputIsChanged(inputYaw))
                changed = 1;
    }

    double roll = 0;
    if (inputIsValid(inputRoll))
    {
            roll = inputGetNumber(inputRoll) / 180.0 * M_PI; 
            if (inputIsChanged(inputRoll))
                changed = 1;
    }

    double pitch = 0;
    if (inputIsValid(inputPitch))
    {
            pitch = inputGetNumber(inputPitch) / 180.0 * M_PI; 
            if (inputIsChanged(inputPitch))
                changed = 1;
    }

    double heave = 0;
    if (inputIsValid(inputHeave))
    {
            heave = inputGetNumber(inputHeave); 
            if (inputIsChanged(inputHeave))
                changed = 1;
    }

    int ret = 1;
    if (changed)
    {
        object->box.yaw = yaw;
        object->box.roll = roll;
        object->box.pitch = pitch;
        object->p[Y_PLANE] = heave;
        
        object->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
        ret = 0;
    }
        
    if (inputIsValid(inputMaterialThicknesMin))
    {    
        if (object->material)
        {
            if (inputIsChanged(inputMaterialThicknesMin))
            {
                object->material->edited_min_thick = inputGetNumber(inputMaterialThicknesMin);
                object->material->flags |= GM_GOBJECT_FLAG_MODIFIED;
                object->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
                ret = 0;
            }
        }
    }

    if (inputIsValid(inputMaterialThicknesMax))
    {    
        if (object->material)
        {
            if (inputIsChanged(inputMaterialThicknesMin))
            {            
                object->material->edited_max_thick = inputGetNumber(inputMaterialThicknesMax);
                object->material->flags |= GM_GOBJECT_FLAG_MODIFIED;
                object->flags &= ~GM_GOBJECT_FLAG_CALCULATED;            
                ret = 0;            
            }
        }
    }
    
    return ret;
}

int solidDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;

#ifdef _DEBUG_SOLDIA    
    LOG("S:EV(W%i) = %i, (%i,%i)\n",w->index,ev,x,y);
#endif
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == saveButton))
    {
        cobjectSetVisibility(_soliddialog_solid,WIDGETCHECKMARK(visibleButton));
        
        if (WIDGETCHECKMARK(normButton))
        {
            _soliddialog_solid->flags |= GM_GOBJECT_FLAG_SHOW_NORM;
        }
        else
        {
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_SHOW_NORM;
        }

        if (WIDGETCHECKMARK(bvvisButton))
        {
            _soliddialog_solid->flags |= GM_GOBJECT_FLAG_SHOW_TRI;
        }
        else
        {
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_SHOW_TRI;
        }

        if (WIDGETCHECKMARK(LFButton))
        {
            _soliddialog_solid->flags |= GM_FLAG_MARK_START_END;
        }
        else
        {
            _soliddialog_solid->flags &= ~GM_FLAG_MARK_START_END;
        }

        if (WIDGETCHECKMARK(BBButton))
        {
            _soliddialog_solid->flags |= GM_GOBJECT_FLAG_SHOW_BBOX;
        }
        else
        {
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_SHOW_BBOX;
        }

        if (WIDGETCHECKMARK(calcButton))
        {
            _soliddialog_solid->flags |= GM_GOBJECT_FLAG_DO_NOT_CALCULATE;
        }
        else
        {
            if (_soliddialog_solid->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE)
            {
                // We need recalculations here
                _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
            }
                        
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_DO_NOT_CALCULATE;
        }
                
        if (0 == solidSetupSolid(_soliddialog_solid))
        {
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
        }
        
        if (solidedit_choice_visible)
        {
            solidedit_choice_visible(DIALOG_CHOICE_SAVE,_soliddialog_solid);
        }
        solidedit_choice_visible = NULL;

        solidEditClear();
        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);

        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == dropButton))
    {
        if (solidedit_choice_visible)
        {
            solidedit_choice_visible(DIALOG_CHOICE_DROP,_soliddialog_solid);
        }
        solidedit_choice_visible = NULL;
        
        solidEditClear();

        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);
        widgetInvisible(dialog);        
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_EXPOSURE == ev) && (w == dialog))
    {
        CObject * object = (CObject*)isCObject(ANY_COBJECT,(GObject*)w->data);
        if (object)
        {
            unsigned long color = aperalGetColor(GAPERAL_MODEL);

            if (object->material)
            {
                color = object->material->color;
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

// ???: This function is not neccessary if widgets using relative coordiantes and drawn by the widget handling
// in case of resize. But we might need some kind of smart placement of widgets 
// they also need a uniform coordinate system

static int position(ViewerWidget *widget)
{
    int width,height;
    
    viewerGetWindowSize(&width, &height);
#ifdef _DEBUG_SOLDIA
    LOG("Placing widgets on W%i\n",widget->index);
#endif
    
    const int pwidth = width /4*3;
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

    bt_x = 10 + visibleButton->width + 10;
    bt_y = 30;

    widgetMoveWidget(normButton, bt_x, bt_y);

    bt_x = 10 + visibleButton->width + normButton->width + 20;
    bt_y = 30;

    widgetMoveWidget(bvvisButton, bt_x, bt_y);

    bt_x = 10 + bvvisButton->width + visibleButton->width + normButton->width + 30;
    bt_y = 30;

    widgetMoveWidget(LFButton, bt_x, bt_y);

    bt_x = 10 + LFButton->width + bvvisButton->width + visibleButton->width + normButton->width + 40;
    bt_y = 30;
    
    widgetMoveWidget(BBButton, bt_x, bt_y);

    bt_x = 10 + BBButton->width + LFButton->width + bvvisButton->width + visibleButton->width + normButton->width + 50;
    bt_y = 30;
    
    widgetMoveWidget(calcButton, bt_x, bt_y);
    
    tabsheetResize(tabsheet, 10, 70, pwidth - 20, pheight - 120);

    inputResize(inputYaw, 150, 40, 100);
    inputResize(inputRoll, 150, 80, 100);
    inputResize(inputPitch, 150, 120, 100);
    inputResize(inputHeave, 150, 160, 100);
    
    inputResize(inputMaterialThicknesMin, 150, 100, 100);
    inputResize(inputMaterialThicknesMax, 150, 140, 100);

    comboPosition(materialSelect, 540, 80);

    tablePosition(dataTable, 5, 5);  
    
    addCalcButtons(_soliddialog_solid);

    return 0;
}

int solidEditClear()
{
    inputClearText(inputYaw);
    inputClearText(inputRoll);
    inputClearText(inputPitch);
    inputClearText(inputHeave);

    inputClearText(inputMaterialThicknesMin);
    inputClearText(inputMaterialThicknesMax);
    
    comboClear(materialSelect);        

    _soliddialog_solid = NULL;
    
    materialText[0] = 0;
    commentText[0]= 0;
    solidName[0] = 0;
    selectionText[0] = 0;
    buttonName[0] = 0;
    
    // number_of_calcbuttons = 0;
    // memset(calcButtons,0,sizeof(calcButtons));
    
    dialog->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;
    modelTab->button->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;        
    dataTab->window->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;
    materialTab->window->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;
    selectionTab->window->flags &= ~VIEWERWIDGET_FLAG_USE_TEXT;
    
    return 0;
}

int addCalcButtons(CObject *object)
{
    if (object)
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,object->parent);
        if (model)
        {
            char modelName[100];
            int length = 0;
            strncpy(modelName,model->name,sizeof(modelName));
            char *s = strchr(modelName,'.');
            if (s)
            {
                *s = 0;
            }
            char**calculators = modelcreatorGetSolidCalculators(modelName, &length);
            
            if ((NULL== calculators) || (0>=length))
            {
                LOG("No model calculator found for \"%s\"\n",modelName);
            }
            
            int b_x = 10;
            for (int i = 0; i < (int)(sizeof(calcButtons)/sizeof(calcButtons[0])); i++)
            {
                if (isGObject(OBJ_WIDGET,calcButtons[i]))
                {
                    if (i < length)
                    {
                        widgetSetText(calcButtons[i],calculators[i]);
                        widgetMoveWidget(calcButtons[i], b_x, VIEWERWIDGET_POS_STICK_BOTTOM);
                        b_x += BUTTON_WIDTH+5;
                    }
                    else
                    {
                        widgetRemoveWidget(calcButtons[i]);
                        calcButtons[i] = NULL;
                        number_of_calcbuttons--;
                    }
                    
                }
                else
                {
                    if (i < length)
                    {
                        calcButtons[i] = widgetCreateWidget(modelTab->window,calculators[i],
                                        VIEWERWIDGET_TYPE_BUTTON,
                                        b_x,
                                        VIEWERWIDGET_POS_STICK_BOTTOM,
                                        BUTTON_WIDTH,
                                        BUTTON_HEIGHT,
                                        solidCalcCallback,_soliddialog_solid);  
                        b_x += BUTTON_WIDTH+5;   
                        number_of_calcbuttons++;
                    }
                    else
                    {
                        calcButtons[i] = NULL;
                    }
                }
                
                
                if (calcButtons[i])
                {
                    calcButtons[i]->flags |= VIEWERWIDGET_FLAG_TEMPORARY;
                }
            }
            
            inputValidated(inputYaw);
            return 0;
        }
    }
    
    return -1;
}

int objetcEditFillDebugTable(const CObject *object, TableWidget *table)
{
    tableClear(table);
    int row = 0;
    
    char s1[GM_VERTEX_BUFFER_SIZE];
    tableAddTextToCell(table, 0, row, object->name); tableAddTextToCell(table, 1, row, vertexPath((GObject*)object,s1,sizeof(s1)));
    row++;
    tableAddTextToCell(table, 0, row, "flags"); tableAddTextToCell(table, 1, row, vertexFlagsToString((GObject*)object,s1,sizeof(s1)));
    row++;
    
    tableAddTextToCell(table, 0, row, "BB->center"); tableAddTextToCell(table, 1, row, EPoint3ToString(object->box.center,s1,sizeof(s1)));
    row++;
    Model *model = (Model*)isCObject(OBJ_MODEL,object->parent);
    snprintf(s1,sizeof(s1),"Model:%lX, Object:%lX",model?model->rep_type:0,object->rep_type);
    tableAddTextToCell(table, 0, row, "Representation"); tableAddTextToCell(table, 1, row, s1);
    row++;
    
    s1[0] = 0;
    if (object->box.flags & GO_BB_CALCULATED)
    {
        commenStringCat(s1,"calc",sizeof(s1));
    }
    if (object->box.flags & GO_BB_NEED_ROTATION)
    {
        if (s1[0])
        {
            commenStringCat(s1,",",sizeof(s1));
        }
        commenStringCat(s1,"nrot",sizeof(s1));
    }
    if (object->box.flags & GO_BB_ORIGINAL_DATA)
    {
        if (s1[0])
        {
            commenStringCat(s1,",",sizeof(s1));
        }
        commenStringCat(s1,"Orig",sizeof(s1));
    }    
    tableAddTextToCell(table, 0, row, "BB->flags"); tableAddTextToCell(table, 1, row, s1);
    row++;
    
    const Solid *solid = (const Solid*)isCObject(OBJ_SOLID,object);
    if (solid)
    {
        tableAddTextToCell(table, 0, row, "triang_filter"); tableAddTextToCell(table, 1, row, vertexJustFlagsToString(solid->triang_filter,s1,sizeof(s1)));
        row++;

        for (const Polygon *poly = solid->first; poly; poly = (const Polygon *)poly->next)
        {            
            tableAddTextToCell(table, 0, row, vertexPath((GObject*)poly,s1,sizeof(s1))); 
            tableAddTextToCell(table, 1, row, vertexFlagsToString((GObject*)poly,s1,sizeof(s1)));  
            row++;
        }
    }
    
    return 0;
}

int solidEdit(CObject *object, DialogUserChoice callback)
{
    if (isCObject(ANY_COBJECT,object))
    {
        char vertexBuff[50];
                        
        if (_soliddialog_solid != object)
        {
            comboClear(materialSelect);        
        }
        
        solidedit_choice_visible = callback;
        _soliddialog_solid = object;
        
        widgetEnableDesktop(0, dialog);

        solidName[0] = 0;
        strncpy(solidName,object->name,sizeof(solidName));
        dialog->data = object;
        
        widgetSetText(dialog,solidName);    
        widgetSetText(modelTab->button,widgetTextClip(modelTab->button,solidName,buttonName,sizeof(buttonName)));
        
        char s1[GM_VERTEX_BUFFER_SIZE];
        Model *model = (Model*)isCObject(OBJ_MODEL,object->parent);
        int poly_count = vertexCountPolygones(object, 0);
        snprintf(modelText,sizeof(modelText),"[%s]: \"%s\" from \"%s\" with %i polygones at %s",
                vertexPath((GObject*)object,s1,sizeof(s1)),
                solidName,
                commenNameFromModel(model->name),
                poly_count,
                EPoint3ToString(object->p,vertexBuff,sizeof(vertexBuff)));
        
        widgetSetText(modelTab->window,modelText);
        
        inputSetNumber(inputYaw, object->box.yaw/M_PI * 180.0);
        inputSetNumber(inputRoll, object->box.roll/M_PI * 180.0);
        inputSetNumber(inputPitch, object->box.pitch/M_PI * 180.0);        
        
        double heave = object->box.p[1];
        inputSetNumber(inputHeave, heave);
        
        inputSetLabel(inputYaw,"yaw [°]");
        inputSetLabel(inputRoll,"roll [°]");
        inputSetLabel(inputPitch,"pitch [°]");
        inputSetLabel(inputHeave,"heave [px]");

        solideditSetMaterial(object->material);
        
        materialTab->window->data = object;
        
        objetcEditFillDebugTable(object, dataTable);
        
        dialog->frame_color = aperalGetColor(GAPERAL_MODEL);
        if (object->material)
        {
            dialog->frame_color = object->material->color;
        }

        widgetSetCheck(visibleButton, cobjectIsVisible(_soliddialog_solid));
        widgetSetCheck(normButton, (_soliddialog_solid->flags & GM_GOBJECT_FLAG_SHOW_NORM));
        widgetSetCheck(bvvisButton, (_soliddialog_solid->flags & GM_GOBJECT_FLAG_SHOW_TRI));
        widgetSetCheck(LFButton, (_soliddialog_solid->flags & GM_FLAG_MARK_START_END));
        widgetSetCheck(BBButton, (_soliddialog_solid->flags & GM_GOBJECT_FLAG_SHOW_BBOX));
        widgetSetCheck(calcButton, (_soliddialog_solid->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE));
         
        solidEditMaterialUpdate(object);

        commentText[0] = 0;
        if (object->comments)
        {
            char text[120];
            strncpy(commentText,commenStripString(object->comments),sizeof(commentText));

            double width = 0;
            double height = 0;
            double length = 0;
            
            if (0 == vertexGetCObjectSize(object, &width, &length, &height))
            {
                snprintf(text,sizeof(text),"Solid width:%.3f, length: %.3f, height:%.3f",width,length,height);
                commenStringCat(commentText,text,sizeof(commentText));
            }
        }
        
        widgetSetText(dataTab->window,commentText);    
        widgetSetText(materialTab->window,materialText);
        
        int len = 0;
        selectionText[0] = 0;
        const GObject **sel = plotterGetSelection(&len);
        for (int i = 0; i < len;i++)
        {
            char text[100];
            text[0] = 0;
            if (sel[i])
            {
                if (NULL == vertexPath(isObject(sel[i]),text,sizeof(text)))
                {
                    break;
                }
                
                commenStringCat(text,",",sizeof(text));
                commenStringCat(selectionText,text,sizeof(selectionText));
                
                if ((i > 0) && (i % 8 == 0))
                {
                    commenStringCat(selectionText,"\n",sizeof(selectionText));
                }
            }
        }
        
        widgetSetText(selectionTab->window,selectionText);
        
        position(dialog);
        
        if (0 == comboNumberOfChoices(materialSelect))
        {         
            if (object->material)
            {
                comboAddText(materialSelect, object->material->name, object->material );
            }
            
            for (Material *material = modelstoreGetMaterial(NULL); NULL != material; material = (Material*)material->next)
            {
                if (object->material != material)
                {
                    comboAddText(materialSelect, material->name, material );
                }
            }
        }
        
        if (object->material)
        {                
            comboSetText(materialSelect,object->material->name);
        }
        
        addCalcButtons(object);
        
        tabsheetSetTab(tabsheet,modelTab);
        
        plotterLockSelection(1);
        widgetVisible(dialog);        
        
        return 0;
    }
    
    return -1;
}


void inputValidated(Input *input)
{
    (void)input;
    
    if (modelTab->window == (ViewerWidget*)isGObject(OBJ_WIDGET,input->widget->parent))
    {
        const int valid = inputIsValid(inputYaw) && inputIsValid(inputPitch) && inputIsValid(inputRoll) && inputIsValid(inputHeave);
        const int available = inputGetTextLength(inputYaw) && inputGetTextLength(inputPitch) && inputGetTextLength(inputRoll) && inputGetTextLength(inputHeave);

        if (valid && available)
        {
            ENABLE_WIDGET(saveButton);
        }
        else
        {
            DISABLE_WIDGET(saveButton);
        }
        
        for (int i = 0; i < (int)(sizeof(calcButtons)/sizeof(calcButtons[0])); i++)
        {
            if (isGObject(OBJ_WIDGET,calcButtons[i]))
            {
    #ifdef _DEBUG_SOLDIA            
                LOG("Calc button W%i = %i\n",calcButtons[i]->index,valid && available);
    #endif            
                if (valid && available)
                {
                    ENABLE_WIDGET(calcButtons[i]);
                }
                else
                {
                    DISABLE_WIDGET(calcButtons[i]);
                }
                
                widgetInvalidate(calcButtons[i]);
            }
            else
            {
                break;
            }
        }    
    }
    
    if (materialTab->window == (ViewerWidget*)isGObject(OBJ_WIDGET,input->widget->parent))
    {
        CObject *object = isCObject(ANY_COBJECT,materialTab->window->data);
        if (object)
        {
            solidEditMaterialUpdate(object);
        }
    }
}

int solideditSetMaterial(Material *material)
{
    char s1[GM_VERTEX_BUFFER_SIZE];
    
    if (isObject(material))
    {
        double min_thick = vertexGetMinThick(material);
        double max_thick = vertexGetMaxThick(material);
        
        MeaUnit unit = material->thickUnit;

        int err = inputSetNumber(inputMaterialThicknesMin,min_thick);
        err |= inputSetNumber(inputMaterialThicknesMax,max_thick);
        
        snprintf(s1,sizeof(s1),"min thick [%s]",meaUnitToString(unit));
        err |= inputSetLabel(inputMaterialThicknesMin,s1);
        
        snprintf(s1,sizeof(s1),"max thick [%s]",meaUnitToString(unit));
        err |= inputSetLabel(inputMaterialThicknesMax,s1);
        
        return err;
    }
    else
    {
        MeaUnit unit = MeaUnit_None;
        
        snprintf(s1,sizeof(s1),"min thick [%s]",meaUnitToString(unit));
        int err = inputSetLabel(inputMaterialThicknesMin,s1);
        
        snprintf(s1,sizeof(s1),"max thick [%s]",meaUnitToString(unit));
        err |= inputSetLabel(inputMaterialThicknesMax,s1);        

        inputEditable(inputMaterialThicknesMax, 0);
        inputEditable(inputMaterialThicknesMin, 0);            
        
        return err;
    }
    
    return 1;
}

int solidEditMaterialUpdate(CObject *object)
{
    if (object)
    {
        // char text[200];
        
        double width, length, height;
        vertexGetCObjectSize((CObject*)object, &width, &length, &height);
                
        EPointL area = 0;
        double area_min = 0;
        double area_max = 0;
        
        MeaUnit unit = MeaUnit_None;
        
        if (0 == meaGetScalar((CObject*)object, GMATH_MEA_ATTRIBUTES_NAME, "Area", &area_min, &area_max,&unit))
        {
            area = (area_min + area_max)/2.0;            
        }
       
        meaConvert(area, MeaUnit_SquarePixel, MeaUnit_SquareMeter, &area);

        EPointL widthM, lengthM, heightM;
        meaConvert(width, MeaUnit_Pixel, MeaUnit_Meter, &widthM);
        meaConvert(length, MeaUnit_Pixel, MeaUnit_Meter, &lengthM);
        meaConvert(height, MeaUnit_Pixel, MeaUnit_Meter, &heightM);

        
        materialText[0] = 0;
        if (object->material)
        {
            solideditSetMaterial(object->material);

            const Material *material = object->material;
            
            const double max_density = vertexGetMaxDensitey(material);
            const double min_density = vertexGetMinDensitey(material);

            const double max_thick = vertexGetMaxThick(material);
            const double min_thick = vertexGetMinThick(material);
            
            snprintf(materialText,sizeof(materialText),"%s: density [%.3f,%.3f] %s, thick [%.3f,%.3f] %s,\nSpec: %s\n"
                            "%s:Surface area = %.3Lf%s\n  width=%.3Lfm, length=%.3Lfm, height=%.3Lfm",
                    material->name,
                    min_density,max_density, meaUnitToString(material->densityUnit),
                    min_thick,max_thick, meaUnitToString(material->thickUnit),
                    material->spec,                    
                    object->name,
                    area,meaUnitToString(MeaUnit_SquareMeter),
                    widthM,lengthM,heightM);  
        }
        else
        {
            snprintf(materialText,sizeof(materialText),"%s: No material assigned\n"
                        "Surface area = %.3Lf%s\n  width=%.3Lfm, length=%.3Lfm, height=%.3Lfm",
                            object->name,
                            area,meaUnitToString(MeaUnit_SquareMeter),
                            widthM,lengthM,heightM);          
        }
        
        /*
        double min_thick = inputGetNumber(inputMaterialThicknesMin);
        double max_thick = inputGetNumber(inputMaterialThicknesMax);
        double max_density = 0;
        double min_density = 0;
        
        if (object->material)
        {
            min_density = object->material->min_density;
            max_density = object->material->max_density;
        }
        
        double min_wight = area * min_density * min_thick;
        double max_wight = area * max_density * max_thick;
        
        snprintf(text,sizeof(text),"%s: max weight = %.3f, min weight = %.3f",
                    object->name,max_wight,min_wight);
        
        viewerSetDrawingColor(aperalGetColor(GAPERAL_TEXT));
        widgetDrawTextWidget(materialTab->window, 10, 240, text, 0);        
        */
    }
    return 0;
}

int solidCalcCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)data;
    (void)x;
    (void)y;

#ifdef _DEBUG_SOLDIA    
    LOG("SO:EV(W%i) = %i, (%i,%i)\n",w->index,ev,x,y);
#endif
    
    if (VIEWEREVENT_BUTTONRELEASE == ev)
    {
        for (int i = 0; i < (int)(sizeof(calcButtons)/sizeof(calcButtons[0])); i++)
        {
            if ((GObject*)w == isGObject(OBJ_WIDGET,calcButtons[i]))
            {     
//                const int valid = inputIsValid(input1) && inputIsValid(input2);
//                const int available = inputGetTextLength(input1) && inputGetTextLength(input2);
                
//                if (valid && available)
                {
                    // inputCommit(input1);
                    // inputCommit(input2);
                    LOG("%s:\n",calcButtons[i]->name);
                    
                    if (_soliddialog_solid)
                    {
                        Model *model = (Model*)isCObject(OBJ_MODEL,_soliddialog_solid->parent);

                        if (model)
                        {
                            return modelcreatorRunSolidCalculator(model->name,calcButtons[i]->name,(CObject*)_soliddialog_solid);
                        }
                    }
                }
                break;
            }
        }
    }  
    
    if (VIEWEREVENT_DESTROY == ev)
    {
        for (int i = 0; i < (int)(sizeof(calcButtons)/sizeof(calcButtons[0])); i++)
        {
            if (i < number_of_calcbuttons)
            {
                if (w == calcButtons[i])
                {
                    calcButtons[i] = NULL;
                    return 1;
                }
            }
            else
            {
                break;
            }
        }
    }
    
    if ((w == materialTab->window) && (VIEWEREVENT_EXPOSURE == ev))
    {
        CObject *object = isCObject(ANY_COBJECT,data);
        if (object)
        {
            if (inputIsValid(inputMaterialThicknesMin) && inputIsValid(inputMaterialThicknesMax))
            {
                solidEditMaterialUpdate(object);
            }
        }
    }
    
    return 0;
}

void comboMaterialChoice(struct ComboBoxT* combo, void *data, int selected_line)
{
    (void)combo;
    (void)selected_line;
    
    if (_soliddialog_solid)
    {
        Material *material = (Material*)isGObject(OBJ_MATERIAL,data);
        if (material && (material != _soliddialog_solid->material))
        {
            vertexReplaceMaterial((CObject*)_soliddialog_solid,material);
            
            _soliddialog_solid->flags &= ~GM_GOBJECT_FLAG_CALCULATED;

            solidEditMaterialUpdate(_soliddialog_solid);
            
            widgetInvalidate(dialog);
        }
    }
}
