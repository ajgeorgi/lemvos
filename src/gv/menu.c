#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Core.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>


#include "menu.h"
#include "viewer.h"

// extern XtAppContext _viewerKontext;
extern Display* _viewerDisplay;
extern Window _viewerMainWindow;

XtActionsRec _menu_actions[5]; 

/*
= {
    String       string;
    XtActionProc proc;
} XtActionsRec;
*/

void WMActionProc(
    Widget   widget,
    XEvent*  event,
    String*  params,
    Cardinal* num_params
)
{
    (void)widget;
    (void)event;
    (void)params;
    (void)num_params;
    
    fprintf(stderr,"***  WMAction\n");
}


void initMenu()
{
    _menu_actions[0].string = "GV Info";
    _menu_actions[0].proc = WMActionProc;
    /*
    XtAppAddActions(
        _viewerKontext         
        _menu_actions         
        1            
    );
    */
/*
    XtActionList *act;
    Cardinal num = 0;
    XtGetActionList(
        GV_APPL_CLASS,
        &act       ,
        &num        
    );
    
    for (int i = 0; i < num; i++)
    {
        fprintf(stderr,"Act %s\n",act[i]->string);
    }
    
    num = 0;
    Atom *atoms = XListProperties(_viewerDisplay,_viewerMainWindow,&num);

    for (int i = 0; i < num;i++)
    {
    }
    */
}
