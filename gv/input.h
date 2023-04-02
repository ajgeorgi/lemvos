#ifndef __INPUT__
#define __INPUT__

/* Input for GV 21.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"
#include "configuration.h"

struct InputT;

typedef void (*ValidationCallback)(struct InputT *input);

typedef struct InputT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
//----------------------------------    
    
    ViewerWidget *widget;
    char textBuffer[100];
    
    int cursor_index;
    int cursor_pos;
    int text_length;
    
    enum InputType {
        Input_None = 0,
        Input_Text,
        Input_Number,
        Input_Config
    } itype;
    
    Config *config;
    
    ValidationCallback valid;
} Input;

Input* inputCreate(ViewerWidget *parent,int x, int y, int width, ValidationCallback valid);

#define INPUT_KEEP_WIDTH 0
int inputResize(Input *input, int x, int y, int width);


int inputSetText(Input *input, const char *text);
int inputSetNumber(Input *input, double number);
int inputSetConfig(Input *input, Config *config);
int inputDisable(Input *input);
int inputEditable(Input *input, int editable);

void inputClearText(Input *input); 


int inputIsChanged(Input *input);
int inputIsValid(Input *input);
int inputSetLabel(Input *input, char *label);

int inputGetTextLength(Input *input); 
const char* inputGetText(Input *input); 
double inputGetNumber(Input *input); 
int inputCommit(Input *input);

#endif // __INPUT__
