#ifndef __COMBOBOX__
#define  __COMBOBOX__

/* ComboBox for GV 20.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"
#include "input.h"
#include "table.h"

struct ComboBoxT;

typedef struct ComboBoxButtonsT {
    ViewerWidget *button;
    void *data;
} ComboBoxButtons;

typedef void (*ComboChoice)(struct ComboBoxT* combo, void *data, int selected_line);

typedef struct ComboBoxT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
//----------------------------------    
    Input *input;
    ViewerWidget *button;
    TableWidget *table;
    int number_of_buttons;
    int selected;
    ComboChoice choice;
    ComboBoxButtons buttons[];
} ComboBox;


ComboBox *comboCreateComboBox(ViewerWidget *parent, char* name, int width, int height, ComboChoice choice);

int comboAddText(ComboBox *combo,char *text, void *data);
int comboAddWidget(ComboBox *combo, ViewerWidget *widget);
int comboSetText(ComboBox *combo, const char* text);

int comboPosition(ComboBox *combo, int x, int y);
int comboClear(ComboBox *combo);

int comboNumberOfChoices(ComboBox *combo);


#endif // __COMBOBOX__