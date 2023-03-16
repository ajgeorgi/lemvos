#ifndef __TABSHETT__
#define __TABSHETT__

/* Tabsheet for GV 21.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"

#define BUTTON_WIDTH 120
#define TAB_BUTTON_WIDTH (BUTTON_WIDTH*2/3)

typedef struct TabT {
    ViewerWidget *button;
    ViewerWidget *window;
} Tab;

typedef struct TabSheetT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// --------------------------       
    ViewerWidget *window;
    Tab *tabs[15];
    int number_of_tabs;
} TabSheet;

TabSheet *tabsheetCreate(ViewerWidget *parent, const char* name, int x, int y, int width, int height);
Tab *tabsheetAddTab(TabSheet *tabsheet, const char* name);
Tab *tabsheetGetTab(TabSheet *tabsheet, int index);
void tabsheetResize(TabSheet *tabsheet, int x, int y, int width, int height);
int tabsheetSetTab(TabSheet *tabsheet, Tab *tab);

#endif // __TABSHETT__