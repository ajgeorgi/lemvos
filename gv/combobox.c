#include <stdlib.h>
#include <string.h>

/* ComboBox for GV 20.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "combobox.h"


#define BUTTON_HEIGHT 20
#define BUTTON_WIDTH 120
#define MAX_NUMBER_OF_WIDGETS 20



void inputValidationCallback(struct InputT *input);
int buttonCallback(ViewerWidget *w, int ev, int x, int y, void *data);


ComboBox *comboCreateComboBox(ViewerWidget *parent, char* name, int width, int height, ComboChoice choice)
{
    const int combo_size = sizeof(ComboBox) + sizeof(ComboBoxButtons) * MAX_NUMBER_OF_WIDGETS;
    ComboBox *combo = (ComboBox*)memory_aloc(combo_size);
    memset(combo,0,combo_size);
    
    objectInit((GObject *)combo, (GObject *)parent, 0, OBJ_COMBO);

    combo->choice = choice;
    combo->input = inputCreate(parent,0, 0, width, inputValidationCallback);
    combo->table = tableCreateTable(parent, NULL, 1,MAX_NUMBER_OF_WIDGETS);
    combo->button = widgetCreateWidget(parent, "?", VIEWERWIDGET_TYPE_CHECK, 0, 0, height, height, buttonCallback, combo);

    inputSetLabel(combo->input , name);
    inputEditable(combo->input ,0);
    
    widgetInvisible(combo->input->widget);
    widgetInvisible(combo->table->widget);
    widgetInvisible(combo->button);

    combo->table->widget->flags |= VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;
    
    return combo;
}


int comboPosition(ComboBox *combo, int x, int y)
{
    inputResize(combo->input, x, y, INPUT_KEEP_WIDTH);
    tablePosition(combo->table, x, y + combo->input->widget->height);
    widgetMoveWidget(combo->button, x + combo->input->widget->width, y);
    
    return 0;
}

int comboAddWidget(ComboBox *combo, ViewerWidget *widget)
{
#ifdef _DEBUG_COMBO    
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("Adding \"%s\" to [%s]\n",widget->name,vertexPath((GObject*)combo,s1,sizeof(s1)));
#endif    
    return tableAddWidgetToCell(combo->table, 0, TABLE_APPEND_ROW,widget);
}

int comboClear(ComboBox *combo)
{
    combo->selected = -1;
    combo->number_of_buttons = 0;
    inputClearText(combo->input);
    return tableClear(combo->table);
}

int comboAddText(ComboBox *combo, char *name, void *data)
{
    if (combo->number_of_buttons < MAX_NUMBER_OF_WIDGETS)
    {
#ifdef _DEBUG_COMBO    
        char s1[GM_VERTEX_BUFFER_SIZE];
        LOG("Adding \"%s\" to [%s]\n",name,vertexPath((GObject*)combo,s1,sizeof(s1)));
#endif    

        int rows = tableGetNumberOfRows(combo->table);
        
        int y = rows*BUTTON_HEIGHT;
                
        if (NULL == combo->buttons[combo->number_of_buttons].button)
        {
            combo->buttons[combo->number_of_buttons].button = widgetCreateWidget(combo->table->widget, 
                                                    name, 
                                                    VIEWERWIDGET_TYPE_BUTTON, 
                                                    1, 
                                                    y, 
                                                    BUTTON_WIDTH, 
                                                    BUTTON_HEIGHT, 
                                                    buttonCallback, combo);

        }
        else
        {
            widgetSetName(combo->buttons[combo->number_of_buttons].button,name);
        }
        
        combo->buttons[combo->number_of_buttons].data = data;
        
        // widgetMoveWidget(combo->buttons[combo->number_of_buttons].button, 0, y);
        
        int ret = comboAddWidget(combo,combo->buttons[combo->number_of_buttons].button);
        if (ret != 0)
        {
            ERROR1("Error: fialed to add text to combo box\n");
        }
        combo->number_of_buttons++;
        
        return ret;
    }
    
    return -1;
}

int comboSetText(ComboBox *combo, const char* text)
{
    for (int b = 0; b < combo->number_of_buttons ; b++)
    {
        if (0 == strcasecmp(combo->buttons[b].button->name,text))
        {
            inputSetText(combo->input,combo->buttons[b].button->name);
            combo->selected = b;
            return 0;
        }
    }    
    
    return -1;
}


void inputValidationCallback(struct InputT *input)
{
    (void)input;
}

int buttonCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)ev;
    (void)x;
    (void)y;
    
    ComboBox *combo = (ComboBox *)isGObject(OBJ_COMBO,data);
    
    if (combo)
    {
        if (ev == VIEWEREVENT_BUTTONRELEASE)
        {
            if (w == combo->button)
            {
                if (WIDGETCHECKMARK(combo->button))
                {
                    // We have to do it to keep stacking order of tabs
                    combo->table->widget->flags &= ~VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;
                    widgetVisible(combo->table->widget);
                }
                else
                {
                    widgetInvisible(combo->table->widget);
                    combo->table->widget->flags |= VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;                    
                }
                return 1;
            }
            else
            {
                for (int i = 0; i < combo->number_of_buttons; i++)
                {
                    if (combo->buttons[i].button == w)
                    {
                        inputSetText(combo->input,w->name);
                        widgetSetCheck(combo->button,0);
                        combo->selected = i;
                        combo->table->widget->flags &= ~VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;
                        widgetInvisible(combo->table->widget);
                        combo->table->widget->flags |= VIEWERWIDGET_FLAG_MANUAL_VISIBILITY;                    
                        
                        if (combo->choice)
                        {
                            combo->choice(combo,combo->buttons[i].data,combo->selected);
                        }
                        
                        return 1;
                    }
                }            
            }
        }
    }
    
    return 0;
}

int comboNumberOfChoices(ComboBox *combo)
{
    return combo->number_of_buttons;
}
