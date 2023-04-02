#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Label.h>
#include <stdio.h>
#include <X11/StringDefs.h>

#include <time.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <math.h>

/* Controller for GV 26.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define __GOBJECT_PRIVATE

#include "controller.h"
#include "viewer.h"
#include "widget.h"
#include "modelstore.h"
#include "loader.h"
#include "configuration.h"
#include "popup.h"
#include "projection.h"

#include "exporter.h"
#include "soliddialog.h"
#include "meshdialog.h"
#include "oceandialog.h"
#include "modeldia.h"

#include "gmath.h"
#include "creator_lemvos.h"
#include "shapes.h"
#include "modelcreator.h"

#include "sequencer.h"

#include "progress.h"

#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 20
#define BUTTON_INCREMENT 25
#define BUTTON_X 10
#define BUTTON_START_Y 10
#define SYS_BUTTON_START 600

ViewerWidget *reloadButton = NULL;
#define RELOAD_LABEL "Reload"
#define RELOAD_BUTTON_X BUTTON_X
#define RELOAD_BUTTON_Y BUTTON_START_Y
#define RELOAD_BUTTON_WIDTH BUTTON_WIDTH
#define RELOAD_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *resetViewButton = NULL;
#define RESETVIEW_LABEL "ResetView"
#define RESETVIEW_BUTTON_X BUTTON_X
#define RESETVIEW_BUTTON_Y (RELOAD_BUTTON_Y + BUTTON_INCREMENT)
#define RESETVIEW_BUTTON_WIDTH BUTTON_WIDTH
#define RESETVIEW_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *yxViewButton = NULL;
#define YXVIEW_LABEL "Y-X Plane"
#define YXVIEW_BUTTON_X BUTTON_X
#define YXVIEW_BUTTON_Y (RESETVIEW_BUTTON_Y + BUTTON_INCREMENT)
#define YXVIEW_BUTTON_WIDTH BUTTON_WIDTH
#define YXVIEW_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *yzViewButton = NULL;
#define YZVIEW_LABEL "Y-Z Plane"
#define YZVIEW_BUTTON_X BUTTON_X
#define YZVIEW_BUTTON_Y (YXVIEW_BUTTON_Y + BUTTON_INCREMENT)
#define YZVIEW_BUTTON_WIDTH BUTTON_WIDTH
#define YZVIEW_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *xzViewButton = NULL;
#define XZVIEW_LABEL "X-Z Plane"
#define XZVIEW_BUTTON_X BUTTON_X
#define XZVIEW_BUTTON_Y (YZVIEW_BUTTON_Y + BUTTON_INCREMENT)
#define XZVIEW_BUTTON_WIDTH BUTTON_WIDTH
#define XZVIEW_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *printViewButton = NULL;
#define PRINT_LABEL "Export"
#define PRINT_BUTTON_X BUTTON_X
#define PRINT_BUTTON_Y (XZVIEW_BUTTON_Y + BUTTON_INCREMENT)
#define PRINT_BUTTON_WIDTH BUTTON_WIDTH
#define PRINT_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *fileViewButton = NULL;
#define FILE_LABEL "Load"
#define FILE_BUTTON_X BUTTON_X
#define FILE_BUTTON_Y (PRINT_BUTTON_Y + BUTTON_INCREMENT)
#define FILE_BUTTON_WIDTH BUTTON_WIDTH
#define FILE_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *solidButtons[20];

unsigned int _solid_button_index = 0;
#define SOLID_BUTTON_X  BUTTON_X
#define SOLID_BUTTON_START_Y  (FILE_BUTTON_Y + BUTTON_INCREMENT + 10)
#define SOLID_BUTTON_WIDTH    BUTTON_WIDTH
#define SOLID_BUTTON_HEIGHT   BUTTON_HEIGHT


ViewerWidget *oceanButton = NULL;
#define SEA_LABEL "Ocean"
#define SEA_BUTTON_X BUTTON_X
#define SEA_BUTTON_Y SYS_BUTTON_START
#define SEA_BUTTON_WIDTH BUTTON_WIDTH
#define SEA_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *modelButton = NULL;
#define CREA_LABEL "Model"
#define CREA_BUTTON_X BUTTON_X
#define CREA_BUTTON_Y (SEA_BUTTON_Y + BUTTON_INCREMENT)
#define CREA_BUTTON_WIDTH BUTTON_WIDTH
#define CREA_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *aboutButton = NULL;
#define ABOUT_LABEL "Info"
#define ABOUT_BUTTON_X BUTTON_X
#define ABOUT_BUTTON_Y (CREA_BUTTON_Y + BUTTON_INCREMENT)
#define ABOUT_BUTTON_WIDTH BUTTON_WIDTH
#define ABOUT_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *quitButton = NULL;
#define QUITAPP_LABEL "Quit"
#define QUITAPP_BUTTON_X BUTTON_X
#define QUITAPP_BUTTON_Y (ABOUT_BUTTON_Y + BUTTON_INCREMENT)
#define QUITAPP_BUTTON_WIDTH BUTTON_WIDTH
#define QUITAPP_BUTTON_HEIGHT BUTTON_HEIGHT


#define GV_VOLUME_MESH_NAME "Wettet"
#define GV_WATER_SOLID_NAME "Water"

unsigned int plott_serial = 0;

typedef struct PlotterTextT {
    char* text;
    int x;
    int y;
    int width;
    int height;
    unsigned long color;
    
    struct PlotterTextT * next;
} PlotterText;


static const GObject *selection[100];
static int selection_length = 0;

#define PLOTTER_TEXT_START 5
static PlotterText* _plotter_text_first = NULL;
static PlotterText* _plotter_text_last = NULL;
static int _plotter_error_text_y = PLOTTER_TEXT_START;

static Model *_plotterModel = NULL;
static char _plotter_model_filename[512];

static int _plotterPaint(void *data);
static int _plotterDrawTexts();
static int _plotterCreateSolidButtons(Model *model);

static Bool _lock_plotter_selection  = 0;

static int _plotterDrawBoundingBox(BoundingBox *box);
int plotterDrawPoint(const EPoint *p, unsigned long color, int size);
static ViewerDriver _plotterViewerDriver;

// extern int XTextExtents(
//     XFontStruct*        /* font_struct */,
//     _Xconst char*       /* string */,
//     int                 /* nchars */,
//     int*                /* direction_return */,
//     int*                /* font_ascent_return */,
//     int*                /* font_descent_return */,
//     XCharStruct*        /* overall_return */
// );
// 
// extern int XTextExtents16(
//     XFontStruct*        /* font_struct */,
//     _Xconst XChar2b*    /* string */,
//     int                 /* nchars */,
//     int*                /* direction_return */,
//     int*                /* font_ascent_return */,
//     int*                /* font_descent_return */,
//     XCharStruct*        /* overall_return */
// );
// 
// extern int XTextWidth(
//     XFontStruct*        /* font_struct */,
//     _Xconst char*       /* string */,
//     int                 /* count */
// );

CObject* plotterUpdateWettetArea(const char *name, CObject *object, unsigned long color);
CObject* plotterUpdateSelection(const char *name, CObject *object, unsigned long color);



int plotterButtonCallback(ViewerWidget *w, int ev, int x, int y, void* data);

void plotterLockSelection(int lock)
{
    _lock_plotter_selection = lock;
}

const GObject **plotterGetSelection(int *length)
{
    if (length)
    {
        *length = selection_length;
        return selection;
    }
    
    return NULL;
}

int clearSelection()
{
    for (int i = 0; i < selection_length;i++)
    {
        const GObject *v = selection[i];
        if ((!isObject(v)) || (v && (0 == (v->flags & GM_FLAG_SELECTED))))
        {
            selection[i] = NULL;
            memmove(&selection[i],&selection[i+1],sizeof(selection[0])*(selection_length-i));
            selection_length--;
            if (selection_length <= 0)
            {
                break;
            }
        }
    }
        
    return 0;
}

int addToSelection(const GObject *sel)
{
    for (int i = 0; i < selection_length;i++)
    {
        const GObject *v = selection[i];
        if (v == sel)
        {
            // Already selected
            return -1;
        }
        
        if (NULL == v)
        {
            selection[i] = v;
            return 0;
        }
    }
    
    if (selection_length < (int)(sizeof(selection)/sizeof(selection[0])))
    {
        selection[selection_length++] = sel;
    
        return 0;
    }
    
    return -1;
}

void plotterHandleError(int code, const char *text)
{
    (void)code;
    
    if (quitButton) // Just to see if we are already initialized
    {
        int width = 0;
        int height = 0;
        int character_height = 0;
        int font_baseline = 0;

        viewerGetWindowSize(&width, &height);

        viewerStringSize("j", 
                1, 
                NULL, 
                NULL, 
                NULL,
                &character_height, 
                &font_baseline);

        plotterDrawText(VIEWER_TEXT_STICK_RIGHT, VIEWER_TEXT_STICK_TOP,  text,  aperalGetColor(GAPERAL_ERROR));
                        // height - font_baseline - _plotter_error_text_y,
        
        _plotter_error_text_y += character_height;
        
        LOG_FLUSH;
    }
    
    fputs(text,stderr);
}

void plotterInit()
{
    if (NULL == reloadButton)
    {
        _plotterModel = NULL;
        _plotter_model_filename[0] = 0;
        memset(solidButtons,0,sizeof(solidButtons));
        memset(selection,0,sizeof(selection));
        selection_length = 0;
        ViewerWidget *buttons[20];
        memset(buttons,0,sizeof(buttons));
        memset(&_plotterViewerDriver,0,sizeof(_plotterViewerDriver));
        
        _plotterViewerDriver.drawBoundingBox = _plotterDrawBoundingBox;
        _plotterViewerDriver.drawPoint = plotterDrawPoint;        
        _plotterViewerDriver.setDrawingColor = viewerSetDrawingColor;
        _plotterViewerDriver.drawText = viewerDrawText3D;
        _plotterViewerDriver.drawLine = viewerDrawLine3D;
        _plotterViewerDriver.getColorFor = aperalGetColor;
        _plotterViewerDriver.printError = plotterHandleError;
        _plotterViewerDriver.drawDot = viewerDrawDot;

        
        buttons[0] = reloadButton = widgetCreateWidget(NULL,RELOAD_LABEL, VIEWERWIDGET_TYPE_BUTTON,RELOAD_BUTTON_X,RELOAD_BUTTON_Y,RELOAD_BUTTON_WIDTH,RELOAD_BUTTON_HEIGHT,
                                        plotterButtonCallback,NULL);
        
        buttons[1] = resetViewButton = widgetCreateWidget(NULL,RESETVIEW_LABEL,VIEWERWIDGET_TYPE_BUTTON,RESETVIEW_BUTTON_X,RESETVIEW_BUTTON_Y,RESETVIEW_BUTTON_WIDTH,RESETVIEW_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        buttons[2] = yxViewButton = widgetCreateWidget(NULL,YXVIEW_LABEL,VIEWERWIDGET_TYPE_CHECK,YXVIEW_BUTTON_X,YXVIEW_BUTTON_Y,YXVIEW_BUTTON_WIDTH,YXVIEW_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        buttons[3] = yzViewButton = widgetCreateWidget(NULL,YZVIEW_LABEL,VIEWERWIDGET_TYPE_CHECK,YZVIEW_BUTTON_X,YZVIEW_BUTTON_Y,YZVIEW_BUTTON_WIDTH,YZVIEW_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);

        buttons[4] = xzViewButton = widgetCreateWidget(NULL,XZVIEW_LABEL,VIEWERWIDGET_TYPE_CHECK,XZVIEW_BUTTON_X,XZVIEW_BUTTON_Y,XZVIEW_BUTTON_WIDTH,XZVIEW_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);

        buttons[5] = printViewButton = widgetCreateWidget(NULL,PRINT_LABEL,VIEWERWIDGET_TYPE_BUTTON,PRINT_BUTTON_X,PRINT_BUTTON_Y,PRINT_BUTTON_WIDTH,PRINT_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        buttons[6] = quitButton = widgetCreateWidget(NULL,QUITAPP_LABEL,VIEWERWIDGET_TYPE_BUTTON,QUITAPP_BUTTON_X,QUITAPP_BUTTON_Y,QUITAPP_BUTTON_WIDTH,QUITAPP_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);

        buttons[7] = fileViewButton = widgetCreateWidget(NULL,FILE_LABEL,VIEWERWIDGET_TYPE_BUTTON,FILE_BUTTON_X,FILE_BUTTON_Y,FILE_BUTTON_WIDTH,FILE_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);

        buttons[8] = oceanButton = widgetCreateWidget(NULL,SEA_LABEL,VIEWERWIDGET_TYPE_BUTTON,SEA_BUTTON_X,SEA_BUTTON_Y,SEA_BUTTON_WIDTH,SEA_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        buttons[9] = modelButton = widgetCreateWidget(NULL,CREA_LABEL,VIEWERWIDGET_TYPE_BUTTON,CREA_BUTTON_X,CREA_BUTTON_Y,CREA_BUTTON_WIDTH,CREA_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        buttons[10] = aboutButton = widgetCreateWidget(NULL,ABOUT_LABEL,VIEWERWIDGET_TYPE_BUTTON,ABOUT_BUTTON_X,ABOUT_BUTTON_Y,ABOUT_BUTTON_WIDTH,ABOUT_BUTTON_HEIGHT,
                                            plotterButtonCallback,NULL);
        
        for (int i = 0;i < (int)(sizeof(buttons)/sizeof(buttons[0]));i++)
        {
            if (buttons[i])
            {
                widgetVisible(buttons[i]);
            }
        }
                
        sequencerAddInitializer("UpdateWettetArea", plotterUpdateWettetArea, 0);
        sequencerAddInitializer("UpdateSelection", plotterUpdateSelection, 0);
        
        initWidget();
        projectionInit();
        gmathInit();
        initShapes(aperalGetColor(GAPERAL_GRID));

        initSolidDialog();
        initMeshDialog();
        initModelDialog();
        oceandiaInit();
        
        widgetSetCheck(yxViewButton,1);
        projectionSetCSystem(CSystem_XY);        
    }
}

void plotterClearTexts()
{
    for (PlotterText *found =  _plotter_text_first; NULL != found;)
    {
        PlotterText *next = found->next;
        memory_free(found->text);
        memory_free(found);
        found = next;
    }
    
    _plotter_error_text_y = PLOTTER_TEXT_START;
    
    _plotter_text_first = NULL;
    _plotter_text_last = NULL;
}

// If text = NULL text will be removed at position
// If position already exists text will be updated
int plotterDrawText(int x, int y, const char* text, unsigned long color)
{
    PlotterText *prev = NULL;
    for (PlotterText *found =  _plotter_text_first; NULL != found; found = found->next)
    {
        if ((found->x == x) && (found->y == y))
        {
            if ((found->text != text) && ((NULL == text) || (0 != strcmp(text,found->text))))
            {
                memory_free(found->text); 
                if ((NULL == text) || (0 == *text))
                {
                    // Remove text
                    if (prev)
                    {
                        prev->next = found->next;                        
                    }
                    if (found == _plotter_text_first)
                    {
                        _plotter_text_first = found->next;
                    }
                    if (found == _plotter_text_last)
                    {
                        _plotter_text_last = prev;
                    }
                    
                    memory_free(found);
                }
                else{
                    // Replace text
                    found->text = memory_strdup(text);
                }
                
            }
            
            _plotterDrawTexts();
            return 0;
        }
        prev = found;
    }

    if (text)
    {
        // Create new text
        PlotterText *ptext = (PlotterText*)memory_aloc(sizeof(PlotterText));
        memset(ptext,0,sizeof(PlotterText));
        
        ptext->x = x;
        ptext->y = y;
        
        viewerStringSize(text, 
                        strlen(text), 
                        &ptext->width, 
                        &ptext->height, 
                        NULL, 
                        NULL, 
                        NULL);

        
        ptext->text = memory_strdup(text);
        ptext->color = color;
        
        if (NULL == _plotter_text_last)
        {
            _plotter_text_first = ptext;
            _plotter_text_last = ptext;
        }
        else
        {
            PlotterText *prev = _plotter_text_last;
            _plotter_text_last = ptext;
            prev->next = ptext;
        }
        
        _plotterDrawTexts();
    }
    
    
    return 0;
}

int plotterLoadModelFromFile(const char* filename)
{
    if (NULL == _plotterModel)
    {
        int err = -1;
        Model *model = NULL;
        
        if (filename)
        {            
            // err = loaderLoadModelProgress(&model, filename, progressSetProgress, 200);     
            err = loaderLoadModel(&model, filename);     
            // progressClose();
        }
        
        if ((NULL == model) || (err < 0))
        {
            commenPrintError(0,"Failed to load model from \"%s\".\n",filename?filename:UNKNOWN_SYMBOL);
        }
        else
        {
            err = plotterAddModel(model, filename);
            if (0 != err)
            {
                commenPrintError(0,"Failed to add model:\n\"%s\"!",filename?filename:UNKNOWN_SYMBOL);
            } 
            else
            {
                char text[100];
                LOG("Loaded: %s\n",modelstoreReport(model, text, sizeof(text)));
            }
        }
                
        return err;
    }
    
    commenPrintError(0,"Model \"%s\" already loaded.\n",_plotterModel->name);
    
    return -1;
}

int plotterIsFileChanged()
{
    if (_plotterModel && _plotter_model_filename[0])
    {
        if (commenIsTime(&_plotterModel->last_modified))
        {
            LTime modification;

            if (0 == commenGetFileModTime(_plotter_model_filename, &modification))
            {   
                long modifiedat = commenTimeDiff(&modification, &_plotterModel->last_modified);
            
                if (ABS(modifiedat) > 1.0)
                {
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

ViewerWidget *plotterGetSolidButton(const CObject *object)
{
    for (int i = 0; i < (int)_solid_button_index; i++)
    {
        ViewerWidget * widget = solidButtons[i];
        
        if (widget->data == (void*)object)
        {
            return widget;
        }
    }
    
    return NULL;
}

int _plotterCreateSolidButtons(Model *model)
{
    int solid_button_y = SOLID_BUTTON_START_Y;

    for (CObject *object = isCObject(ANY_COBJECT,model->first); object; object = isCObject(ANY_COBJECT,object->next))
    {    
        if (_solid_button_index < sizeof(solidButtons)/sizeof(solidButtons[0]))
        {    
            ViewerWidget *widget = widgetCreateWidget(NULL,NULL,VIEWERWIDGET_TYPE_BUTTON,SOLID_BUTTON_X,solid_button_y,BUTTON_WIDTH,BUTTON_HEIGHT,
                                            plotterButtonCallback,(void*)object);
            
            widgetSetText(widget,object->name);
            
            widget->flags |= VIEWERWIDGET_FLAG_TEMPORARY;
            
            object->methods.objectEdit = solidEdit;
            if (object->material)
            {
                widget->frame_color = object->material->color;
            }
            else
            {
                widget->frame_color = aperalGetColor(GAPERAL_MESH);
            }
                
            widgetVisible(widget);
            
            solidButtons[_solid_button_index] = widget;
            solid_button_y += BUTTON_INCREMENT;
            _solid_button_index++;
        }
    }
    
    return 0;
}

void plotterExportChoice(int choice, void* userData)
{
    (void)userData;
    
    char filenamebuffer[256];
    const char *filename = _plotter_model_filename;
    
    const char**exporterList =  exporterListExporter();    
    const char *exporterName = NULL;
    
    if ((0 <= choice) && (NULL != exporterList) && (NULL != exporterList[choice]))
    {
        exporterName = exporterList[choice];
    }
    
    if (_plotterModel && exporterName)
    {
        char text[280];

        if (0 == _plotter_model_filename[0])
        {
            filename = _plotterModel->name;
        }
        
        int err = 1;
        Exporter *exporter = exporterGetExporter(exporterName);
        
        if (exporter)
        {
            const char *exportfilename = exporterFilename(exporter, filename, filenamebuffer, sizeof(filenamebuffer));
            
            if (exportfilename)
            {
                err = exporterExport(exporter, _plotterModel,exportfilename);
            }
            
            if (0 == err)
            {
                char directory[128];
                getcwd(directory, sizeof(directory));
                
                snprintf(text,sizeof(text),"%s File created:\n%s\nIn directory:\n%s\nVertices created: %i of %i",
                        exporterName,
                        exportfilename,
                        directory,
                        exporterGetVertexCount(exporter),
                        modelstoreGetVertexCount()
                        );
            }
        }
        
        if (err)
        {
            snprintf(text,sizeof(text),"Creating %s failed!", exporterName);
        }
        popup(text,NULL);
    }    
}

int plotterPrint()
{
    const char**exporterList =  exporterListExporter();
    
    if ((NULL != exporterList) && (NULL != exporterList[0]))
    {
        char text[100];
        snprintf(text,sizeof(text),"Pick an exporter:");
        userChoice(text, exporterList, plotterExportChoice, NULL);
    }

    return 0;
}

void plotterLoad(const char* filename)
{
        // No more model ...
    _plotterModel = NULL;     
    // ... nothing to paint.
    viewerAddPaintMethod(NULL,NULL);
    
    plotterClearTexts();
    solidEditClear();
    meshEditClear();
    oceandiaClear();
    modelEditClear();

    widgetRemoveTempWidgets();
    _solid_button_index = 0;
    memset(solidButtons,0,sizeof(solidButtons));
    
    if (0 < selection_length)
    {
        memset(selection,0,selection_length  * sizeof(selection[0]));
    }
    selection_length = 0;
        
    // Store should be emtpy now
    modelstoreClear();
    // tell user about empty store
    viewerSetTitle("Empty",NULL);         
    // Clear screen ...
    viewerClear();
    
    // ... and get a new model
    plotterLoadModelFromFile(filename);        
}

void plotterLoadChoice(int choice, void* userData)
{
    if ((0 <= choice) && userData)
    {
        const char* filename = (const char*)userData;
        plotterLoad(filename);
    }
}

void plotterLoadFile()
{   
    char path[128];
    getcwd(path, sizeof(path));
    
    fileChoice("Pick model:", path, "model;stl", plotterLoadChoice);
}

CObject* plotterUpdateWettetArea(const char *name, CObject *object, unsigned long color)
{
    (void)name;
    (void)color;
    
    Model *model = (Model*)isCObject(OBJ_MODEL,object);
    if (model)
    {
        Solid *water = modelstoreGetSolid(model, GV_WATER_SOLID_NAME);                
        if (water)
        {
            for (CObject *mobject = isCObject(ANY_COBJECT,model->first); mobject; mobject = isCObject(ANY_COBJECT,mobject->next))
            {                
                Solid *solid = (Solid*)isCObject(OBJ_SOLID,mobject);
                if (solid)
                {
                    if (solid->wettet && (water != solid))
                    {       
                        char s1[GM_VERTEX_BUFFER_SIZE];                        
                        LOG("Updating [%s]\n",vertexPath((GObject*)solid,s1,sizeof(s1)));
                        
                        if (NULL == gmathIntersect(solid, 0,water->normal.normal, water->normal.p, solid->wettet))
                        {
                            ERROR("Water dip failed for [%s].\n",vertexPath((GObject*)solid,s1,sizeof(s1)));
                        } 
                    }
                }
            }
        }
    }
    
    return (CObject*)model;
}

CObject* plotterUpdateSelection(const char *name, CObject *object, unsigned long color)
{
    (void)name;
    (void)color;
    
    Model *model = (Model*)isCObject(OBJ_MODEL,object);
    if (model)
    {    
        clearSelection();
        
        for (CObject *mobject = isCObject(ANY_COBJECT,model->first); mobject; mobject = isCObject(ANY_COBJECT,mobject->next))
        {                
            Solid *solid = (Solid*)isCObject(OBJ_SOLID,mobject);
            if (solid && solid->wettet)
            {
                Iter iter = gobjectCreateIterator((CObject*)solid->wettet,0);
                for (GObject *obj = gobjectIterate(iter); obj; obj = gobjectIterate(iter))
                {
                    Vertex *vertex2 = (Vertex*)isGObject(OBJ_VERTEX,obj);

                    if (vertex2 && (vertex2->flags & GM_FLAG_SELECTED))
                    {
                        if (addToSelection((GObject*)vertex2))
                        {
                            break;
                        }
                    }
                }                    
            }
        }
    }

    return (CObject*)model;
}


void objectDialogClose(int choice, CObject* object)
{        
    if (DIALOG_CHOICE_SAVE == choice)
    {
        if (0 == (object->flags & GM_GOBJECT_FLAG_CALCULATED))
        {
            _plotterModel->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
            
            if (object->material)
            {
                ViewerWidget *widget = plotterGetSolidButton(object);
                if (widget)
                {
                    widget->frame_color = object->material->color;
                }
            }
            
            if (0 != sequencerRun(_plotterModel))
            {
                char text[100];
                char s1[GM_VERTEX_BUFFER_SIZE];
                snprintf(text,sizeof(text),"Sequencer failed on \"%s\"\n",vertexPath((GObject*)_plotterModel,s1,sizeof(s1)));                
                plotterHandleError(0, text);
            }
        }                
    }
}

int plotterButtonCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == yxViewButton))
    {
        projectionSetCSystem(CSystem_XY);
        widgetSetCheck(xzViewButton,0);
        widgetSetCheck(yzViewButton,0);
        widgetSetCheck(yxViewButton,1);
        viewerResetView();
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == yzViewButton))
    {
        projectionSetCSystem(CSystem_YZ);
        widgetSetCheck(yxViewButton,0);
        widgetSetCheck(xzViewButton,0);
        widgetSetCheck(yzViewButton,1);
        viewerResetView();
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == xzViewButton))
    {
        projectionSetCSystem(CSystem_XZ);
        widgetSetCheck(yxViewButton,0);
        widgetSetCheck(yzViewButton,0);
        widgetSetCheck(xzViewButton,1);
        viewerResetView();
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == reloadButton))
    {
        plotterLoad(_plotter_model_filename);       
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == resetViewButton))
    {
        projectionInit();
        viewerResetView();
        
        return 1;
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == quitButton))
    {
        char text[300];
        text[0] = 0;

        if (_plotterModel)
        {
            for (CObject *object = isCObject(ANY_COBJECT,_plotterModel->first); object; object = isCObject(ANY_COBJECT,object->next))
            {
                if ((GM_GOBJECT_FLAG_MODIFIED|GM_GOBJECT_FLAG_LOADED) == (object->flags & (GM_GOBJECT_FLAG_MODIFIED|GM_GOBJECT_FLAG_LOADED)))
                {
                    char changed[100];
                    snprintf(changed,sizeof(changed),"\"%s\" has been changed!\n",object->name);
                    commenStringCat(text,changed,sizeof(text));
                }
            }
        }
        
        if (0 == text[0])
        {
            strncpy(text,"No changes at model\n",sizeof(text));
        }
        
        viewerQuit(0,text);
        
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == printViewButton))
    {
        plotterPrint();
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == oceanButton))
    {
        oceandiaOpen(_plotterModel);

        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == modelButton))
    {
        modelEdit((CObject*)_plotterModel,NULL);

        return 1; // consume event
    }    
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == fileViewButton))
    {
        plotterLoadFile();
        return 1; // consume event
    }
    else
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == aboutButton))
    {
        char text[1000];
        char directory[128];
        directory[0] = 0;
        char solids[600];
        
        solids[0] = 0;
        const char * dir = getcwd(directory,sizeof(directory));

        if (_plotterModel)
        {
            char s1[GM_VERTEX_BUFFER_SIZE];     
            for (CObject *object = isCObject(ANY_COBJECT,_plotterModel->first); object; object = isCObject(ANY_COBJECT,object->next))
            {
                const int measurements = vertexCountMeasurements(object, 0);
                int marked_count = vertexCountVertices(object, GM_GOBJECT_VERTEX_FLAG_MARK);
                int marked2_count = vertexCountVertices(object, GM_GOBJECT_VERTEX_FLAG_MARK2);
                int error_count = vertexCountVertices(object, GM_GOBJECT_VERTEX_FLAG_MARK_ERROR);

                long object_size = 0;
                if (object->methods.objectSize)
                {
                    object_size = object->methods.objectSize(object);
                }
                
                char solidText[250];
                solidText[0] = 0;
                Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
                Mesh  *mesh = (Mesh*)isCObject(OBJ_MESH,object);
                if (solid)
                {
                    const int poly_count = vertexCountPolygones(object, 0);
                    const int orig_poly_count = vertexCountPolygones(object, GM_GOBJECT_FLAG_POLY_ORIGINAL);

                    snprintf(solidText,sizeof(solidText),"%s:\"%s\", Polygones=%i/%i, Vert.=%li, |%i|, m=%i, m2=%i, e=%i\n",
                             vertexPath((GObject*)solid,s1,sizeof(s1)),solid->name,
                             poly_count,
                             orig_poly_count,
                             object_size,
                             measurements,
                             marked_count,
                             marked2_count,
                             error_count                             
                            );
                }
                else
                if (mesh)
                {
                    snprintf(solidText,sizeof(solidText),"%s:\"%s\", Vert.=%li |%i|, m=%i, m2=%i, e=%i\n",
                             vertexPath((GObject*)mesh,s1,sizeof(s1)),mesh->name,
                             object_size,
                             measurements,
                             marked_count,
                             marked2_count,
                             error_count
                            );
                }
                else
                {
                    snprintf(solidText,sizeof(solidText),"%s:\"%s\", Vert.=%li, |%i|, m=%i, m2=%i, e=%i\n",
                             vertexPath((GObject*)object,s1,sizeof(s1)),object->name,
                             object_size,
                             measurements,
                             marked_count,
                             marked2_count,
                             error_count                             
                            );                    
                }
                commenStringCat(solids,solidText,sizeof(solids));
            }
        }

        
        snprintf(text,sizeof(text),"Andrej Georgi: Lemvos %s version %s\n"
                                   "Build %s, %s\n"
                                   "Owner and copyright holder, ask for licence:\naxgeorgi@gmail.com\n"
                                   "A tool to watch propriatery 3D models\n"
                                   "intendet for inspection and production.\n"
                                   "NO MILITARY USE!\n"
                                   "Display info:\n%s\n"
                                   "Model directory:\n%s\n"
                                   "Number of Vertices selected: %i\n"
                                   "%s", 
                                   
                 GV_APPLICATION,
                 GV_VERSION,
                 __DATE__,
#ifdef _DEBUG
                "_DEBUG",
#else
                "Release",
#endif                 
                 projectionGetInfo(),
                 dir,
                 selection_length,
                 solids);
        
        popup(text,NULL);
        
        int width,height;
    
        viewerGetWindowSize(&width, &height);
        
        popupResize(width*0.6, height*2/3);

        return 1; // consume event
    }
    else
    {
        for (unsigned int i = 0; i < _solid_button_index; i++)
        {
            if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == solidButtons[i]))
            {
                CObject *object = (CObject*)isCObject(ANY_COBJECT,w->data);
                
                if (object)
                {
                    if (object->methods.objectEdit)
                    {
#ifdef _DEBUG                        
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        LOG("Editing [%s] \"%s\"\n",vertexPath((GObject*)object,s1,sizeof(s1)),object->name);
#endif
                        object->methods.objectEdit(object,objectDialogClose);
                    }                    
                }
                
                return 2; // ask for repaint and consume event
            }
        }        
    }
    
    return 0; // ignore event
}

int plotterAddModel(Model *model, const char* filename)
{
    if (NULL == _plotterModel)
    {
        if (NULL == isCObject(OBJ_MODEL,model))
        {
            viewerPrintError(0,"No model. Nothing to plot!\n");
            viewerClear();
            return -1;
        }
        else
        {
            char timeModified[100];
            char title[256];        
            char configFileNameBuffer[256];

            _plotterModel = model;
            
#ifdef _DEBUG_DISABLE_SEQUENCE            
            model->flags |= GM_GOBJECT_FLAG_DO_NOT_CALCULATE;
#endif      
            
            // Convert the model modification time to a string
            if (!commenIsTime(&_plotterModel->last_modified))
            {
                strncpy(timeModified,"Unknown",sizeof(timeModified));
            }
            else
            {
                strftime(timeModified, sizeof(timeModified), GM_TIME_FORMAT,&_plotterModel->last_modified);
            }
            
            if (filename)
            {
                if (filename != _plotter_model_filename)
                {
                    _plotter_model_filename[0] = 0;
                    if ('/' != *filename)
                    {
                        // expand relative file name to absolute path
                        getcwd(_plotter_model_filename,sizeof(_plotter_model_filename));
                        commenStringCat(_plotter_model_filename,"/",sizeof(_plotter_model_filename));
                    }
                    commenStringCat(_plotter_model_filename,filename,sizeof(_plotter_model_filename));

                    filename = _plotter_model_filename;
                }
                
                snprintf(title,sizeof(title),"\"%s\" from \"%s\" (%u) last modified: %s",
                         commenNameFromModel(_plotterModel->name),
                         _plotter_model_filename,
                         modelstoreGetVertexCount(),
                         timeModified);
            }
            else
            {
                snprintf(title,sizeof(title),"\"%s\" (%u) last modified: %s",
                         commenNameFromModel(_plotterModel->name),
                         modelstoreGetVertexCount(),
                         timeModified);                
                filename = configFileNameBuffer;

                const char *model_name = commenNameFromModel(_plotterModel->name);
                if (model_name)
                {
                    strncpy(configFileNameBuffer,model_name,sizeof(configFileNameBuffer));
                }
            }
            
            // Get model configuration too
            const char*configFileName =  loaderGetConfigNameFromModel(filename, configFileNameBuffer, sizeof(configFileNameBuffer));
            if (!isFileExists(configFileName) && ('/' != configFileName[0]))
            {
                commenPrefixString("../models/",configFileNameBuffer,sizeof(configFileNameBuffer));
            }
            loaderLoadConfig(configFileName);

            viewerSetTitle(title,_plotterModel->name);

            // Try to initialize model by its type
            if (0 != modelcreatorInitModel(_plotterModel))
            {
                // Is there a type name included in the model name?
                if (commenNameFromModel(_plotterModel->name) != _plotterModel->name)
                {
                    ERROR("Error: Initialization of model \"%s\" failed.\n",_plotterModel->name);
                }
            }
            
            // Initial face
            EPoint n[3] = { 0,1,0 };
            EPoint p[3] = { 0,0,0 };
            double width = 500;
            double length = 800;
            // ???
            vertexGetCObjectSize((CObject*)_plotterModel, &width, &length, NULL);
//            int wwidth = width*2.6;
//            int wlength = length*1.8;
            
            Solid *water = shapeCreateFace(_plotterModel,GV_WATER_SOLID_NAME, n, p, width*2.6, length*1.8);
            if (water)
            {
                water->flags |= GM_GOBJECT_FLAG_DO_NOT_CALCULATE;
                
                Material *material = modelstoreGetMaterial(GM_MATERIAL_WATER_NAME);
                if (material)
                {
                    vertexReplaceMaterial((CObject*)water,material);
                }
                else
                {
                    commenPrintError(0, "No water material definition found!\n");

                    ERROR1("No water material definition found!\n");
                }
                modelstoreMoveSolidToBottom(water);
            }
            else
            {
                ERROR1("Error: Failed to create water\n");
            }

            if (water)
            {
                char wettetName[100];
                char s1[GM_VERTEX_BUFFER_SIZE];
                for (GObject *object = isObject(model->first); object; object= isObject(object->next))
                {   
                    // Closing the box here. No more expansion of the BB only rotation from now on.
                    // For new expansion of the BB you need to clear the box
                    CObject *cobject = (CObject *)isCObject(ANY_COBJECT,object);
                    if (cobject)
                    {
                        if (vertexMakeBBNormal(&cobject->box))
                        {
                            ERROR("Failed to close BB on \"%s\"\n",cobject->name);
                        }
                        else
                        {
                            char s1[GM_VERTEX_BUFFER_SIZE];
                            LOG("Bounding box at \"%s\" [%s] created.\n",cobject->name,vertexPath((GObject*)cobject,s1,sizeof(s1)));
                        }
                    }
                    
                    if (GM_GOBJECT_FLAG_LOADED & object->flags)
                    {
                        DObject *dobject = (DObject *)isDObject(ANY_DOBJECT,object);
                        if (dobject)
                        {                        
                            char *wname = strchr(vertexPath((GObject*)dobject,s1,sizeof(s1)),'.');
                            
                            if (wname)
                            {
                                wname++;
                            }
                            else
                            {
                                wname = s1;
                            }
                            
                            snprintf(wettetName,sizeof(wettetName),"%s.%s",GV_VOLUME_MESH_NAME,
                                    wname);
                        
                            if (NULL == dobject->wettet)
                            {
                                dobject->wettet = modelstoreCreateMesh(_plotterModel, wettetName, 500);

                                if (dobject->wettet && (NULL == dobject->wettet->material))
                                {
                                    vertexReplaceMaterial((CObject*)dobject->wettet,modelstoreGetMaterial(GM_MATERIAL_WATER_NAME));
                                }
                            }
                            
                            dobject->original_box = dobject->box;
                            dobject->original_box.flags |= GO_BB_ORIGINAL_DATA;
                        }
                    }
                        
                }
            }

            // Get initial calculations
            sequencerRun(_plotterModel);
            
            _plotterCreateSolidButtons(_plotterModel);
            
            // Update some configuration on the viewer
            viewerResetView();
            
            snprintf(title,sizeof(title),"Model owner: %s",_plotterModel->owner);
            plotterDrawText(VIEWER_TEXT_STICK_RIGHT, 0, title, aperalGetColor(GAPERAL_MODEL_OWNER));

        }
            
        viewerAddPaintMethod(_plotterPaint, _plotterModel);
        
        // Initial paint
        _plotterPaint(model);
        
        return 0;
    }
    
    viewerPrintError(0,"Model \"%s\" exists already.\n",commenNameFromModel(_plotterModel->name));
    return -1;
}


void plotterOrigin()
{
    const double length = 200;
    EPoint x1[GM_VERTEX_DIMENSION]= { 0,0,0 };
    EPoint x2[GM_VERTEX_DIMENSION]= { length,0,0 };

    EPoint y1[GM_VERTEX_DIMENSION]= { 0,0,0 };
    EPoint y2[GM_VERTEX_DIMENSION]= { 0,length,0 };

    EPoint z1[GM_VERTEX_DIMENSION]= { 0,0,0 };
    EPoint z2[GM_VERTEX_DIMENSION]= { 0,0,length };

    // X-Plane
    viewerSetDrawingColor(aperalGetColor(GAPERAL_AXIS_X));    
    viewerDrawLine3D(x1, x2);            

    // Y-Plane
    viewerSetDrawingColor(aperalGetColor(GAPERAL_AXIS_Y));
    viewerDrawLine3D(y1, y2);            
    
    // Z-Plane
    viewerSetDrawingColor(aperalGetColor(GAPERAL_AXIS_Z));
    viewerDrawLine3D(z1, z2);

/*
    viewerSetDrawingColor(GVCOLOR_WHITE);    
    EPoint o[3] = {1,1,1};    
    viewerDrawLine3D(o,projectionGetVelue(0));
    */
}


int plotterPaintFace(EPoint *normal, EPoint *p)
{
    EPoint x1[3];
    EPoint y1[3];
    
    if (0 == vertexSpawnSurface(normal,p,x1,y1))
    {
        EPointExtend(x1,500,x1);    
        EPointExtend(y1,500,y1);
        
        viewerSetDrawingColor(aperalGetColor(GAPERAL_GRID));

        EPoint xl[3];
        //EPoint yl[3];
        EPoint xi[3],xip1[3];        
    
        //EPointExtend(yl,1000,y1);
        EPointExtend(xl,1000,x1);

        AddEPoints(xi,p,x1);
        
        /*
        char nstr[40],pstr[40];
        char xstr[40],ystr[40];    
        printf("Face: n=%s, p=%s, x1= %s, y1=%s\n",
            EPoint3ToString(normal,nstr,sizeof(nstr)),
            EPoint3ToString(p,pstr,sizeof(pstr)),
            EPoint3ToString(x1,xstr,sizeof(xstr)),
            EPoint3ToString(y1,ystr,sizeof(ystr)));
*/
        
        for (int xo = 0; xo <= 10; xo++)
        {
            AddEPoints(xip1,xi,xl);
            
            viewerDrawLine3D(xi,xip1);
            
            AddEPoints(xi,xi,y1);
        }
        
        return 0;
    }
    
    return -1;
}

int _plotterDrawBoundingBox(BoundingBox *box)
{
        unsigned long color = aperalGetColor(GAPERAL_BOUNDING_BOX);
        viewerSetDrawingColor(color);

        /*
        const EPoint *p = box->p;
        EPoint x1[3] = { box->min[0],box->min[1],box->min[2] };
        EPoint x2[3] = { box->max[0], 
                            box->min[1],
                            box->min[2]};
        EPoint x3[3] = { box->max[0], 
                            box->min[1],
                            box->max[2]};
        EPoint x4[3] = { box->min[0], 
                            box->min[1],
                            box->max[2]};
        EPoint x5[3] = { box->min[0], 
                            box->max[1],
                            box->max[2]};
        EPoint x6[3] = { box->max[0],box->max[1],box->max[2] };
        EPoint x7[3] = { box->max[0], 
                            box->max[1],
                            box->min[2]};
        EPoint x8[3] = { box->min[0], 
                            box->max[1],
                            box->min[2]};
            */

        EPoint corner[sizeof(box->corner)/sizeof(box->corner[0])];                            
        for (int i = 0; i < (int)(sizeof(box->corner)/sizeof(box->corner[0])/GM_VERTEX_DIMENSION); i++)
        {
            vertexVectorAdd(&corner[i*3],box->p,&box->corner[i*3]);
        }

        
        viewerDrawLine3D(&corner[0*3],&corner[1*3]);
        viewerDrawLine3D(&corner[1*3],&corner[2*3]);
        viewerDrawLine3D(&corner[2*3],&corner[3*3]);
        viewerDrawLine3D(&corner[3*3],&corner[4*3]);
        viewerDrawLine3D(&corner[4*3],&corner[5*3]);
        viewerDrawLine3D(&corner[5*3],&corner[2*3]);
        viewerDrawLine3D(&corner[7*3],&corner[4*3]);
        viewerDrawLine3D(&corner[6*3],&corner[5*3]);
        viewerDrawLine3D(&corner[0*3],&corner[3*3]);
        viewerDrawLine3D(&corner[0*3],&corner[7*3]);
        viewerDrawLine3D(&corner[7*3],&corner[6*3]);
        viewerDrawLine3D(&corner[1*3],&corner[6*3]);

/*      The center lines
 *   
        EPoint l2_p1[3];
        EPoint l2_p2[3];

        viewerDrawLine3D(box->max,box->min);
        
        l2_p1[0] = box->min[0];
        l2_p1[1] = box->max[1];
        l2_p1[2] = box->max[2];

        l2_p2[0] = box->max[0];
        l2_p2[1] = box->min[1];
        l2_p2[2] = box->min[2];

        viewerDrawLine3D(l2_p2,l2_p1);
      */  

        EPoint center[3], top_center[3];
        AddEPoints(center,box->p,box->center);
        plotterDrawPoint(center,color,40);

        AddEPoints(top_center,box->p,box->top_center);        
        plotterDrawPoint(top_center,color,35);
        
        return 0;
}

int _plotterPlot(Model *model)
{
    if (NULL == model)
    {
       return -1;
    }
   
    // plotterPaintFace(model->sea_level_normale, model->sea_level);
   
    for (CObject *object = isCObject(ANY_COBJECT,model->first); object; object= isCObject(ANY_COBJECT,object->next))
    {
        if (0 == (object->flags & GM_GOBJECT_FLAG_INVISIBLE))
        {
            if (object->methods.paint)
            {
                object->methods.paint(&_plotterViewerDriver,(GObject*)object);
            }
            
            if (object->flags & GM_GOBJECT_FLAG_SHOW_BBOX)
            {
#ifdef _DEBUG_BBOX                    
                char s1[GM_VERTEX_BUFFER_SIZE];
#endif                
                if (_plotterViewerDriver.drawBoundingBox)
                {
#ifdef _DEBUG_BBOX                    
                    double width = 0;
                    double length = 0;
                    double height = 0;
                    vertexGetCObjectSize(object, &width, &length, &height);    
                    
                    LOG("Drawing BB for [%s] = (%.3f,%.3f,%.3f)\n",vertexPath((GObject*)object,s1,sizeof(s1)),width,length,height);
#endif                    
                    _plotterViewerDriver.drawBoundingBox(&object->box);
                }
#ifdef _DEBUG_BBOX                                    
                else
                {
                    ERROR("No BB drawing method for [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
                }
#endif                
            }            
            
            // Go for the measurements
            for (CObject *mobject = isCObject(ANY_COBJECT,object->first_mea); mobject; mobject= isCObject(ANY_COBJECT,mobject->next))
            {
                if (mobject->methods.paint)
                {
                    mobject->methods.paint(&_plotterViewerDriver,(GObject*)mobject);
                }                
            }                                    
        }
    }

    if (model && (model->flags & GM_GOBJECT_FLAG_SHOW_BBOX))
    {
        _plotterDrawBoundingBox(&model->box);
        
    }

    // fprintf(stderr,"> %i polygones plottet\n", polyCount);
    
    return 0;
}


int _plotterPaint(void *data)
{
    int ret = 0;
    
    if (data)
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,data);
        ret = _plotterPlot(model);
    }
    
    if (plotterIsFileChanged())
    {
        reloadButton->color = aperalGetColor(GAPERAL_WIDGET_HIGHLIGHT);
    }
    else
    {
        reloadButton->color = aperalGetColor(GAPERAL_WIDGET);
    }
    
    plotterOrigin();
    
    _plotterDrawTexts();

    return ret;
}

int plotterDrawPoint(const EPoint *p, unsigned long color, int size)
{
    viewerSetDrawingColor(color);
    
    EPoint x1[GM_VERTEX_DIMENSION]= { -size,0,0 };
    EPoint x2[GM_VERTEX_DIMENSION]= { +size,0,0 };

    EPoint y1[GM_VERTEX_DIMENSION]= { 0,-size,0 };
    EPoint y2[GM_VERTEX_DIMENSION]= { 0,+size,0 };

    EPoint z1[GM_VERTEX_DIMENSION]= { 0,0,-size };
    EPoint z2[GM_VERTEX_DIMENSION]= { 0,0,+size };

    AddEPoints(x1,x1,p);
    AddEPoints(x2,x2,p);
    
    AddEPoints(y1,y1,p);
    AddEPoints(y2,y2,p);
    
    AddEPoints(z1,z1,p);
    AddEPoints(z2,z2,p);
    
    viewerDrawLine3D(x1, x2);            
    viewerDrawLine3D(y1, y2);            
    viewerDrawLine3D(z1, z2);   
       
    return 0;
}

int _plotterDrawTexts()
{
    unsigned long color = viewerGetDrawingColor();
    
    for (PlotterText *text =  _plotter_text_first; NULL != text; text = text->next)
    {
        viewerSetDrawingColor(text->color);
        
        viewerDrawText(text->x, text->y, text->text);
    }
    
    viewerSetDrawingColor(color);
    
    viewerFlush();
    
    return 0;
}



