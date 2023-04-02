#include <string.h>
#include <math.h>

/* Ocean Dialog for GV 20.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "commen.h"
#include "oceandialog.h"
#include "popup.h"
#include "widget.h"
#include "sequencer.h"
#include "aperal.h"
#include "viewer.h"
#include "controller.h"
#include "table.h"

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 20
#define BUTTON_INCREMENT 25
#define BUTTON_X 10
#define BUTTON_START_Y 10


static ViewerWidget *dialog = NULL;
#define DIALOG_LABEL "Ocean"
#define DIALOG_BUTTON_X BUTTON_X
#define DIALOG_BUTTON_Y BUTTON_START_Y
#define DIALOG_BUTTON_WIDTH BUTTON_WIDTH
#define DIALOG_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *value_edit = NULL;
#define VALUEED_LABEL "Value"
#define VALUEED_BUTTON_X 150
#define VALUEED_BUTTON_Y 30
#define VALUEED_BUTTON_WIDTH 500
#define VALUEED_BUTTON_HEIGHT 600

static ViewerWidget *okButton = NULL;
#define OK_LABEL "Close"
#define OK_BUTTON_X BUTTON_X
#define OK_BUTTON_Y BUTTON_START_Y
#define OK_BUTTON_WIDTH BUTTON_WIDTH
#define OK_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *saveButton = NULL;
#define SAVE_LABEL "Reset"
#define SAVE_BUTTON_X BUTTON_X
#define SAVE_BUTTON_Y BUTTON_START_Y
#define SAVE_BUTTON_WIDTH BUTTON_WIDTH
#define SAVE_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *dropButton = NULL;
#define DROP_LABEL "Close"
#define DROP_BUTTON_X BUTTON_X
#define DROP_BUTTON_Y BUTTON_START_Y
#define DROP_BUTTON_WIDTH BUTTON_WIDTH
#define DROP_BUTTON_HEIGHT BUTTON_HEIGHT

static ViewerWidget *markButton = NULL;
#define MARK_LABEL "Mark"
#define MARK_BUTTON_X 10
#define MARK_BUTTON_Y 5
#define MARK_BUTTON_WIDTH BUTTON_WIDTH
#define MARK_BUTTON_HEIGHT BUTTON_HEIGHT

TableWidget *resultTable = NULL;

#define VALUE_ROW_WIDTH 450

static char modelName[80];

static ViewerWidget **_oceandialog_value_widgets = NULL;
static int _oceandialog_number_of_value_widgets = 0;
static int _oceandialog_max_number_of_value_widgets = 0;

static Model *_oceandialog_model = NULL;
static int oceanDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static int oceanDialogValueCallback(ViewerWidget *w, int ev, int x, int y, void *data);

static ViewerWidget *createValueWidget(const char* text, Value *value)
{
    if (resultTable)
    {
        ViewerWidget *widget = NULL;
        
        if (_oceandialog_number_of_value_widgets >= _oceandialog_max_number_of_value_widgets)
        {
            _oceandialog_max_number_of_value_widgets += 30;
            _oceandialog_value_widgets = memory_realloc(_oceandialog_value_widgets,_oceandialog_max_number_of_value_widgets*sizeof(ViewerWidget*));
            memset(&_oceandialog_value_widgets[_oceandialog_number_of_value_widgets],0,(_oceandialog_max_number_of_value_widgets-_oceandialog_number_of_value_widgets)*sizeof(ViewerWidget*));
        }

        if (NULL == _oceandialog_value_widgets[_oceandialog_number_of_value_widgets])
        {
            _oceandialog_value_widgets[_oceandialog_number_of_value_widgets] = 
                widgetCreateWidget(resultTable->widget,NULL,VIEWERWIDGET_TYPE_BUTTON,SAVE_BUTTON_X,SAVE_BUTTON_Y,SAVE_BUTTON_WIDTH,SAVE_BUTTON_HEIGHT,
                                        oceanDialogValueCallback,NULL);
        }        

        widget = _oceandialog_value_widgets[_oceandialog_number_of_value_widgets];
        
        if (widget)
        {
            _oceandialog_number_of_value_widgets++;
            
            widgetSetName(widget,text);
            widget->data = value;
            widget->flags |= VIEWERWIDGET_FLAG_LEFT_ALIGN;
            
            widgetSetColor(widget, GVCOLOR_TRANSPARENT, GVCOLOR_TRANSPARENT);

            return widget;
        }
    }
    
    return NULL;
}

void oceandiaInit()
{
   sequencerInit(aperalGetIndexedColor);
   
    if (NULL == dialog)
    {
        dialog = widgetCreateWidget(NULL,NULL,VIEWERWIDGET_TYPE_WINDOW,DIALOG_BUTTON_X,DIALOG_BUTTON_Y,DIALOG_BUTTON_WIDTH,DIALOG_BUTTON_HEIGHT,
                                    oceanDialogCallback,NULL);

        saveButton = widgetCreateWidget(dialog,SAVE_LABEL,VIEWERWIDGET_TYPE_BUTTON,SAVE_BUTTON_X,SAVE_BUTTON_Y,SAVE_BUTTON_WIDTH,SAVE_BUTTON_HEIGHT,
                                    oceanDialogCallback,NULL);
        
        dropButton = widgetCreateWidget(dialog, DROP_LABEL, VIEWERWIDGET_TYPE_BUTTON,DROP_BUTTON_X,DROP_BUTTON_Y,DROP_BUTTON_WIDTH,DROP_BUTTON_HEIGHT,
                                    oceanDialogCallback,NULL);

        value_edit = widgetCreateWidget(NULL,NULL,VIEWERWIDGET_TYPE_WINDOW,VALUEED_BUTTON_X,VALUEED_BUTTON_Y,VALUEED_BUTTON_WIDTH,VALUEED_BUTTON_HEIGHT,
                                    oceanDialogCallback,NULL);

        okButton = widgetCreateWidget(value_edit, OK_LABEL, VIEWERWIDGET_TYPE_BUTTON,(value_edit->width-OK_BUTTON_WIDTH)/2,value_edit->height-OK_BUTTON_HEIGHT-10,OK_BUTTON_WIDTH,OK_BUTTON_HEIGHT,
                                    oceanDialogValueCallback,NULL);

        markButton = widgetCreateWidget(value_edit, MARK_LABEL, VIEWERWIDGET_TYPE_CHECK,MARK_BUTTON_X,MARK_BUTTON_Y,MARK_BUTTON_WIDTH,MARK_BUTTON_HEIGHT,
                                    oceanDialogValueCallback,NULL);
        
        resultTable = tableCreateTable(dialog, NULL, 3, 30);        
        
        tableSetColumnWidth(resultTable, 0, 150);
        tableSetColumnWidth(resultTable, 1, VALUE_ROW_WIDTH);

        widgetInvisible(dialog);
        widgetInvisible(value_edit);
    }
   
}

int oceanUpdateDataFromMea(TableWidget *table, Mea *mea)
{   
    if (mea)
    {
#ifdef _DEBUG_OCEANDIA                            
        char s1[GM_VERTEX_BUFFER_SIZE];        
        char s2[GM_VERTEX_BUFFER_SIZE];
        char s3[GM_VERTEX_BUFFER_SIZE];
#endif
        
        const int last_row = tableGetNumberOfRows(table);
        CObject *cobj = (CObject *)isCObject(ANY_COBJECT,mea->parent);
        tableAddTextToCell(table, 0, last_row, cobj->name);
        
        char calcText[100];
        snprintf(calcText,sizeof(calcText),"%s (%s)",mea->name,cobj->material?cobj->material->name:UNKNOWN_SYMBOL);
        tableAddTextToCell(table, 1, last_row, calcText);
        tableSetRowSeperator(table, last_row);
        
        for (int i = 0; i < mea->number_of_values; i++)
        {
            Value *value = &mea->values[i];

            const int last_row = tableGetNumberOfRows(table);

            char cell_text[128];

            snprintf(cell_text,sizeof(cell_text)," %s",value->name);
            tableAddTextToCell(table, 0, last_row, cell_text);
            // tableSetCellFontSize(table, 0,last_row, GV_FONT_SIZE_SMALL);

            double min = 0;
            double max = 0;
            MeaUnit new_unit = MeaUnit_None;
            
            cell_text[0] = 0;
            color_type color = meaGetValueColor(value);
            
            if (value->flags & GM_GOBJECT_VERTEX_FLAG_MARK)
            {
                color = aperalGetColor(GAPERAL_TEXT_HIGHLIGHT);
            }

            if (0 == meaGetConvinientUnit(value, -1,&min, &max, &new_unit))
            {
                const char *v= "";
                if (ValueType_Vector == value->vtype)
                {
                    v = "|v|=";
                }
                
                const char* unit = meaUnitToString(new_unit);
                snprintf(cell_text,sizeof(cell_text),"%s[%g, %g] %s",
                         v,
                        min,max,unit);                    
            }
            else
            {
                strncpy(cell_text,UNKNOWN_SYMBOL,sizeof(cell_text));
            }
               
            ViewerWidget *widget = createValueWidget(cell_text,value);
            tableAddWidgetToCell(table, 1, last_row, widget);
            
#ifdef _DEBUG_OCEANDIA            
            LOG("Adding [%s] to table [%s] using [%s] at %i with \"%s\"\n",
                vertexPath((GObject*)value,s1,sizeof(s1)), 
                vertexPath((GObject*)table,s2,sizeof(s2)),
                vertexPath((GObject*)widget,s3,sizeof(s3)),
                last_row,cell_text);
#endif
            
            tableSetCellColor(table, 1, last_row, color);

            // tableSetCellFontSize(table, 1,last_row, GV_FONT_SIZE_SMALL);            
        }
        

        return 0;
    }
    return -1;
}

int oceanUpdateDataTable(TableWidget *table, const Model *model)
{
    tableClear(table);
    _oceandialog_number_of_value_widgets = 0;

    for (const CObject *object = isCObject(ANY_COBJECT,model->first); object; object= isCObject(ANY_COBJECT,object->next))
    {
        // Go for the measurements
        for (const CObject *mobject = isCObject(ANY_COBJECT,object->first_mea); mobject; mobject= isCObject(ANY_COBJECT,mobject->next))
        {
            Mea *mea = (Mea*)isCObject(OBJ_MEASUREMENT,mobject);
            oceanUpdateDataFromMea(table, mea);
        }        
    }    
    
    return 0;
}

void oceandiaClear()
{
    _oceandialog_model = NULL;
    _oceandialog_number_of_value_widgets = 0;
    tableClear(resultTable);
}

static int position(ViewerWidget *widget)
{
    int width,height;
    
    viewerGetWindowSize(&width, &height);

    const int pwidth = width / 2.5;
    const int pheight = height/4*3.6;
    const int x = (width - pwidth) - 10;
    const int y = (pheight/16);

    widgetSizeWidget(widget, pwidth, pheight);
    widgetMoveWidget(widget,x, y);

    int bt_x = 10;
    int bt_y = pheight - saveButton->height - 10;

    widgetMoveWidget(saveButton, bt_x, bt_y);

    bt_x = pwidth - dropButton->width - 10;
    bt_y = pheight - dropButton->height - 10;

    widgetMoveWidget(dropButton, bt_x, bt_y);
    dropButton->flags |= VIEWERWIDGET_FLAG_ESCAPE;
        
    tablePosition(resultTable, 5, 5);                
    
    return 0;
}


void oceandiaOpen(Model *model)
{
#ifdef _DEBUG    
    char s1[GM_VERTEX_BUFFER_SIZE];    
    LOG("Open ocean dialog on [%s]\n", vertexPath((GObject*)model,s1,sizeof(s1)));
#endif
    
    widgetEnableDesktop(0, dialog);

    
    oceanUpdateDataTable(resultTable, model);         
    
    modelName[0] = 0;
    strncpy(modelName,model->name,sizeof(modelName));
    dialog->data = model;
    
    _oceandialog_model = model;

    dialog->frame_color = aperalGetColor(GAPERAL_OCEAN);
    if (model->material)
    {
        dialog->frame_color = model->material->color;
    }

    position(dialog);

    plotterLockSelection(1);
    widgetInvisible(value_edit);
    widgetVisible(dialog);                
}

int oceanDialogCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;

#ifdef _DEBUG_OCEANDIA
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("OC:EV(%s) = %s, (%i,%i)\n",vertexPath((GObject*)w,s1,sizeof(s1)),widgetEventToText(ev),x,y);
#endif
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == saveButton))
    {        
        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);
        widgetInvisible(value_edit);
        widgetInvisible(dialog);  
        oceandiaClear();
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == dropButton))
    {
        plotterLockSelection(0);        
        widgetEnableDesktop(1, NULL);
        widgetInvisible(value_edit);
        widgetInvisible(dialog);        
        oceandiaClear();
        return 1; // consume event
    }

    if ((VIEWEREVENT_EXPOSURE == ev) && (w == value_edit))
    {
        const Value *value = (Value *)isGObject(OBJ_VALUE,data);

        if (value)
        {
            viewerFillRectanlge(value_edit->x + value_edit->width - 30 , value_edit->y +10, 20, 20, meaGetValueColor(value));
        }
    }

    return 0;
}

int oceandialogOpenValue(Value *value,ViewerWidget *value_widget)
{
    static char text[600];
    text[0] = 0;
    char s1[300];
    char s2[GM_VERTEX_BUFFER_SIZE];
    char s3[GM_VERTEX_BUFFER_SIZE];
    char s4[GM_VERTEX_BUFFER_SIZE];
    
    CObject *object = NULL;
    Mea *mea = NULL;
    const char *calculated = NULL;
    if (value && value->parent && value->parent->parent)
    {
        object = (CObject *)isCObject(ANY_COBJECT,value->parent->parent);
        mea = (Mea*) isCObject(OBJ_MEASUREMENT,value->parent);
        if (mea)
        {
            calculated = mea->created_by;
        }
    }

    widgetSetCheck(markButton,(value->flags & GM_GOBJECT_VERTEX_FLAG_MARK));
    
    snprintf(s1,sizeof(s1),"Object: [%s] \"%s\"\nCalculated: ([%s] \"%s\") %s\nValue: [%s] \"%s\"\n\n",
             vertexPath((GObject*)object,s2,sizeof(s2)),
             object?object->name:UNKNOWN_SYMBOL,
             vertexPath((GObject*)mea,s4,sizeof(s4)),
             mea?mea->name:UNKNOWN_SYMBOL,             
             calculated?calculated:UNKNOWN_SYMBOL,
             vertexPath((GObject*)value,s3,sizeof(s3)),
             value?value->name:UNKNOWN_SYMBOL             
            );
    commenStringCat(text,s1,sizeof(text));

    if (value)
    {
        switch(value->vtype)
        {
            case ValueType_Scalar: {
                double smax = 0;
                double smin = 0;
                MeaUnit new_unit;
                
                for (int i = 0; i <= MIN(value->value_index,(int)(sizeof(value->data)/sizeof(value->data[0]))); i++)
                {
                    double new_value_min;
                    double new_value_max;
                    meaGetConvinientUnit(value, i, &new_value_min, &new_value_max, &new_unit);

                    snprintf(s1,sizeof(s1),"[ %g, %g] %s\n",
                            new_value_min,
                            new_value_max,
                            meaUnitToString(new_unit));
                    
                    smax += new_value_max;
                    smin += new_value_min;
                    
                    commenStringCat(text,s1,sizeof(text));
                }
                
                const double mean_max = (smax + smin)/2;
                const double mean_min = (smax + smin)/2;
                const double sdev_max = (smax - smin)/2;
                const double sdev_min = (smax - smin)/2;
                const double med_max = (smax - smin)/2;
                const double med_min = (smax - smin)/2;
                snprintf(s1,sizeof(s1),"\nMean= [ %g, %g]\nMed= [ %g, %g]\nSDev= [ %g, %g]\n\n",
                        mean_min,mean_max,
                        sdev_min,sdev_max,
                        med_min,med_max);
                
                commenStringCat(text,s1,sizeof(text));                     
            } break;
            
            case ValueType_Vector: {
            } break;
            
            case ValueType_Connection: {
            } break;
            
            default:
                ;
        }

        commenStringCat(text,value->description,sizeof(text));
        commenStringCat(text,"\n\n",sizeof(text));  
        
        if (object)
        {
            double width = 0;
            double height = 0;
            double length = 0;
            
            if (vertexGetCObjectSize(object, &width, &length, &height))
            {
                ERROR1("Can not get object size\n");
            }
            
            EPointL widthM, lengthM, heightM;
            meaConvert(width, MeaUnit_Pixel, MeaUnit_Meter, &widthM);
            meaConvert(length, MeaUnit_Pixel, MeaUnit_Meter, &lengthM);
            meaConvert(height, MeaUnit_Pixel, MeaUnit_Meter, &heightM);

            if (object->material)
            {
                const Material *material = object->material;
                
                const double max_density = vertexGetMaxDensitey(material);
                const double min_density = vertexGetMinDensitey(material);

                const double max_thick = vertexGetMaxThick(material);
                const double min_thick = vertexGetMinThick(material);
                
                snprintf(s1,sizeof(s1),"Material: %s\n  density [%.3f,%.3f] %s\n  thick [%.3f,%.3f] %s,\nSpec: %s\n\n"
                                "Dimensions: width=%.3Lfm\n       length=%.3Lfm\n       height=%.3Lfm\n",
                        material->name,
                        min_density,max_density, meaUnitToString(material->densityUnit),
                        min_thick,max_thick, meaUnitToString(material->thickUnit),
                        material->spec,                    
                        widthM,lengthM,heightM);  
            }
            else
            {
                snprintf(s1,sizeof(s1),"Material: No material assigned\n\n"
                            "Dimensions: width=%.3Lfm\n       length=%.3Lfm\n       height=%.3Lfm",
                                widthM,lengthM,heightM);          
            }
            
            
            commenStringCat(text,s1,sizeof(text));                     
        }
    }
    
    value_edit->data = value;
    markButton->data = value_widget;
    okButton->data = value;
    
    widgetSetText(value_edit,text);
    widgetSetTextPosition(value_edit, 2, 80);
    widgetEnable(value_edit, 1);

    return widgetVisible(value_edit);  
}

int oceanDialogValueCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)w;

#ifdef _DEBUG_OCEANDIA
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("OCV:EV(%s) = %s, (%i,%i)\n",vertexPath((GObject*)w,s1,sizeof(s1)),widgetEventToText(ev),x,y);
#endif

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == okButton))
    {
        Value *value = (Value *)isGObject(OBJ_VALUE,data);

        if (value)
        {
            if (WIDGETCHECKMARK(markButton))
            {
                value->flags |= GM_GOBJECT_VERTEX_FLAG_MARK;
            }
            else
            {
                value->flags &= ~GM_GOBJECT_VERTEX_FLAG_MARK;
            }
        }
        
        widgetInvisible(value_edit);              
        // oceanUpdateDataTable(resultTable, _oceandialog_model);
        return 1;
    }

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == markButton))
    {
        ViewerWidget *value_widget = (ViewerWidget *)isGObject(OBJ_WIDGET,data);
        const Value *value = (Value *)isGObject(OBJ_VALUE,okButton->data);
        
        if (value_widget && value)
        {
            if (WIDGETCHECKMARK(markButton))
            {
                color_type color = aperalGetColor(GAPERAL_TEXT_HIGHLIGHT);
                widgetSetColor(value_widget, color, GVCOLOR_TRANSPARENT);
            }
            else
            {
                color_type color = meaGetValueColor(value);            
                widgetSetColor(value_widget, color, GVCOLOR_TRANSPARENT);
            }
        }
        
        
        return 1;
    }
        
    
    if (VIEWEREVENT_BUTTONRELEASE == ev)
    {      
        Value *value = (Value *)isGObject(OBJ_VALUE,data);
        if (NULL == value)
        {
            value = (Value *)isGObject(OBJ_VALUE,okButton->data);
        }
        
        if (value)
        {
            oceandialogOpenValue(value,w);
        }
              
        return 1;
    }        
    
    return 0;
}
