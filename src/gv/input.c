#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Input for GV 21.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "input.h"
#include "aperal.h"
#include "viewer.h"
#include "commen.h"

#define BACKSPACE 0xff08
#define DEL 0xffff

#define RETURN 0xff0d
#define LEFT 0xff51
#define RIGHT 0xff53
#define END 0xff57
#define BEGIN 0xff50
#define TAB 0xff09

#define NUM_0 0xff9e
#define NUM_1 0xff9c
#define NUM_2 0xff99
#define NUM_3 0xff9B
#define NUM_4 0xff96
#define NUM_5 0xff9d
#define NUM_6 0xff98
#define NUM_7 0xff95
#define NUM_8 0xff97
#define NUM_9 0xff9a

#define NUM_PLUS 0xffab
#define NUM_ENTER 0xff8d

#define INPUT_FLAG_EDITED (1<<0)
#define INPUT_FLAG_VALID  (1<<1)
#define INPUT_FLAG_NOEDIT  (1<<2)

static int cursor_height = 0;

static int num_key[] = { NUM_0,NUM_1,NUM_2,NUM_3,NUM_4,NUM_5,NUM_6,NUM_7,NUM_8,NUM_9 };

static int inputCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static int updateInput(Input *input);
static int inputValidate(Input *input);

int inputEditable(Input *input, int editable)
{
    if (input)
    {
        if (editable)
        {
            input->flags &= ~INPUT_FLAG_NOEDIT;
        }
        else
        {
            input->flags |= INPUT_FLAG_NOEDIT;
        }
        
        return 0;
    }
    
    return -1;
}

const char* inputGetText(Input *input)
{
    if (input)
    {
        return input->textBuffer;
    }
    return NULL;
}

int inputGetTextLength(Input *input)
{
    if (input)
    {    
        return input->text_length;
    }
    
    return 0;
}

void inputClearText(Input *input)
{
    input->textBuffer[0] = 0;
    input->cursor_index = 0;
    input->text_length = 0;
    
    input->config = NULL;
    DISABLE_WIDGET(input->widget);
    
    updateInput(input);
}

int inputSetText(Input *input, const char *text)
{
    if (input)
    {
        if (text)
        {
            strncpy(input->textBuffer,text,sizeof(input->textBuffer));
        }
        else
        {
            input->textBuffer[0] = 0;
        }
        input->flags &= ~INPUT_FLAG_EDITED;

        int len = strnlen(input->textBuffer,sizeof(input->textBuffer));
        input->cursor_index = len;
        input->text_length = len;
        input->itype = Input_Text;
        ENABLE_WIDGET(input->widget);
        return updateInput(input);
    }
    
    return -1;
}

int inputSetNumber(Input *input, double number)
{
    if (input)
    {
        snprintf(input->textBuffer,sizeof(input->textBuffer),"%.3f",number);
        input->flags &= ~INPUT_FLAG_EDITED;
        
        int len = strnlen(input->textBuffer,sizeof(input->textBuffer));
        input->cursor_index = len;
        input->text_length = len;
        input->itype = Input_Number;
        ENABLE_WIDGET(input->widget);
        return updateInput(input);
    }
    
    return -1;    
}

int inputSetLabel(Input *input, char *label)
{
    if (input && input->widget)
    {
        char *lab = strchr(label,'.');
        if (NULL == lab)
        {
            lab = label;
        }
        else
        {
            lab++;
        }
        
        return widgetSetName(input->widget,lab);
    }
    
    return -1;    
}

int inputSetConfig(Input *input, Config *config)
{
    if (input && config)
    {
        const char *text = configString(config, "");

        strncpy(input->textBuffer,text,sizeof(input->textBuffer));
        input->flags &= ~INPUT_FLAG_EDITED;
        
        int len = strnlen(input->textBuffer,sizeof(input->textBuffer));
        input->cursor_index = len;
        input->text_length = len;
        input->config = config;
        input->itype = Input_Config;
        ENABLE_WIDGET(input->widget);
        inputSetLabel(input,config->name);
        return updateInput(input);
    }
    
    return -1;        
}

double inputGetNumber(Input *input)
{
    if (input)
    {
        input->itype = Input_Number;
        return commenStringToDouble(input->textBuffer);
    }
    
    return -1;
}

int inputCommit(Input *input)
{
    if (input && input->config)
    {
        if (inputIsValid(input) && inputIsChanged(input))
        {
            return configUpdateConfig(input->config,input->textBuffer);
        }
    }
    
    return -1;
}

int inputDisable(Input *input)
{
    if (input)
    {
        DISABLE_WIDGET(input->widget);
    }
    return -1;
}

int inputIsChanged(Input *input)
{
    if (input && (INPUT_FLAG_EDITED & input->flags))
    {
        return 1;
    }
    
    return 0;
}

int inputIsValid(Input *input)
{
    if (input && (INPUT_FLAG_VALID & input->flags))
    {
        return 1;
    }
    
    return 0;
}

Input* inputCreate(ViewerWidget *parent,int x, int y, int width, ValidationCallback valid)
{
    int character_height;
    
    Input *input = (Input*)memory_aloc(sizeof(Input));
    memset(input,0,sizeof(Input));
    objectInit((GObject *)input, (GObject *)parent, 0, OBJ_INPUT);   
    
    
    viewerStringSize("j", 1,NULL, NULL, NULL, &character_height, NULL);

    if (0 == cursor_height)
    {
        cursor_height = character_height - 2; 
    }

    input->valid = valid;
    input->widget = widgetCreateWidget(parent, NULL, VIEWERWIDGET_TYPE_INPUT, 
                                       x, y, 
                                       width, 
                                       character_height + 2, inputCallback, input);
    
    input->widget->frame_color = aperalGetColor(GAPERAL_INPUT_FRAME);
    
    DISABLE_WIDGET(input->widget);
    
    updateInput(input);
    
    return input;
}

int inputResize(Input *input, int x, int y, int width)
{
    if (width > INPUT_KEEP_WIDTH)
    {
        int character_height;
        viewerStringSize("j", 1,NULL, NULL, NULL, &character_height, NULL);
        
        widgetSizeWidget(input->widget, width, character_height + 2);
    }
    widgetMoveWidget(input->widget, x, y);

    return 0;
}

int updateInput(Input *input)
{
    int width;
    int height;
    int character_width;
    int character_height;
    int baseline;
    
    if (input && ISWIDGET_VISIBLE(input->widget))
    {
        unsigned long color = aperalGetColor(GAPERAL_INPUT_FOREGROUND);
        
        viewerStringSize(input->textBuffer, input->text_length,&width, &height, &character_width, &character_height, &baseline);
        
        if (width >= input->widget->width - character_width)
        {
            if (0 < input->text_length)
            {
                input->text_length--;
                width -= character_width;
            }
            input->textBuffer[input->text_length] = 0;        
        }

        if (!inputValidate(input))
        {
            color = aperalGetColor(GAPERAL_FAILED);
        }

        widgetClearWidget(input->widget);
        
        if ((input->widget->flags & VIEWERWIDGET_FLAG_HAS_FOCUS) && (0 == (input->flags & INPUT_FLAG_NOEDIT)))
        {
            viewerStringSize(input->textBuffer, input->cursor_index,&input->cursor_pos, NULL, NULL, NULL, NULL);
            
            unsigned long frame_color = aperalGetColor(GAPERAL_CURSOR);
                
            viewerFillRectanlge(input->widget->x + input->cursor_pos + 1,
                                input->widget->y + character_height - baseline - 2, 
                                2, cursor_height, 
                                frame_color);
        }
        
        viewerSetDrawingColor(color);
        return widgetDrawTextWidget(input->widget, 
                                2, baseline, 
                                input->textBuffer, sizeof(input->textBuffer));
    }
    
    return 0;
}

int addCharacter(Input *input, int character, int key_state)
{
    (void)key_state;
    
    if (!ISWIDGET_DISABLED(input->widget) && (0 == (input->flags & INPUT_FLAG_NOEDIT)))
    {
        // printf("C= %x, S= %x\n",character,key_state);
        
        // Decode number block keys
        if ((0xff00 == (0xff00 & character)) && (0x10 == key_state))
        {
            for (int i = 0; i < (int)(sizeof(num_key)/sizeof(num_key[0])); i++)
            {
                if (character == num_key[i])
                {
                    character = '0' + i;
                    // printf("Found: '%c'\n",(char)character);
                    break;
                }
            }
        }
        
        if (0 == (character & 0xff00))
        {                     
            if (input->text_length < (int)(sizeof(input->textBuffer)/sizeof(input->textBuffer[0])))
            {
                if (31 < character)
                {
                    memmove(&input->textBuffer[input->cursor_index+1],
                            &input->textBuffer[input->cursor_index],
                            input->text_length - input->cursor_index + 1);
                    
                    input->textBuffer[input->cursor_index] = (char)character;
                    input->text_length++;
                    input->flags |= INPUT_FLAG_EDITED;

                    if (input->cursor_index < input->text_length)
                    {
                        input->cursor_index++;
                    }
                    else
                    {
                        input->cursor_index = input->text_length-1;
                    }
                    
                    if (input->valid && (1 == input->text_length))
                    {
                        input->valid(input);
                    }
                }
            }
        }
        input->textBuffer[input->text_length] = 0;    
    }
    
    return 0;
}

int inputCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)w;
    (void)y;
    
    Input *input = (Input*)data;
    int ret = 0;
#ifdef _DEBUG_INPUT
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("IN:%s:%s:EV(%s):(%i,%i)\n",vertexPath((GObject*)w,s1,sizeof(s1)),w->name?w->name:"?",widgetEventToText(ev),x,y);
#endif    
        
    if (input && input->widget)
    {
        switch(ev)
        {
            case VIEWEREVENT_BUTTONPRESS: {
                ret = 1;
                break;
            }
            
            case VIEWEREVENT_BUTTONRELEASE: {
                break;                
            }            
            
            case VIEWEREVENT_EXPOSURE: {    
                updateInput(input);
                ret = 1;
                break;
            }
            
            case VIEWEREVENT_FOCUS: {
                updateInput(input);
                ret = 1;
                break;
            }
            case VIEWEREVENT_FOCUS_LOST: {
                updateInput(input);
                ret = 1;
                break;                
            }
            case VIEWEREVENT_KEY: {     
                // char c = (char) x;
                
                // int high = x >> 8;
                // int low = x & 0xff;
                
                // fprintf(stderr,"Input: '%c', key(%i,%i) = %i, state =%i\n",c,high,low,x,y);
                if (0 == (input->flags & INPUT_FLAG_NOEDIT))
                {
                    switch(x)
                    {                    
                        case BACKSPACE: {
                            if ((0 < input->cursor_index) && (0 < input->text_length))
                            {
                                input->flags |= INPUT_FLAG_EDITED;
                                memmove(&input->textBuffer[input->cursor_index-1],
                                    &input->textBuffer[input->cursor_index],
                                    input->text_length - input->cursor_index + 1);

                                input->text_length--;                            
                                
                                input->cursor_index--;
                                if (input->cursor_index > input->text_length)
                                {
                                    input->cursor_index = input->text_length;
                                }

                                if (input->valid && (0 >= input->text_length))
                                {
                                    input->valid(input);
                                }
                            }
                            break;
                        }
                        case DEL: {
                            if ((0 <= input->cursor_index) && (0 < input->text_length))
                            {
                                input->flags |= INPUT_FLAG_EDITED;
                                memmove(&input->textBuffer[input->cursor_index],
                                    &input->textBuffer[input->cursor_index+1],
                                    input->text_length - input->cursor_index + 1);

                                input->text_length--;                            
                                
                                if (input->cursor_index > input->text_length)
                                {
                                    input->cursor_index = input->text_length;
                                }
                                
                                if (input->valid && (0 >= input->text_length))
                                {
                                    input->valid(input);
                                }                            
                            }
                            break;                        
                        }
                        case LEFT: {
                            if (0 < input->cursor_index)
                            {
                                input->cursor_index--;
                            }
                            break;
                        }
                        case RIGHT: {
                            if (input->cursor_index < input->text_length)
                            {
                                input->cursor_index++;
                            }
                            break;
                        }
                        case END: {
                            if (0 < input->text_length)
                            {
                                input->cursor_index = input->text_length;
                            }
                            else
                            {
                                input->cursor_index = 0;
                            }
                            break;
                        }
                        case BEGIN: {
                            input->cursor_index = 0;
                            break;
                        }  
                        
                        case NUM_ENTER:
                        case RETURN:
                        case TAB: {
                            if ((INPUT_FLAG_VALID & input->flags) && input->widget->sibling)
                            {
                                widgetFocus(input->widget->sibling);
                            }
                            break;
                        }
                        default: {   
                            addCharacter(input,x,y);
                        }
                    }
                    
                    input->textBuffer[input->text_length] = 0;
                    
                    updateInput(input);
                    
                    ret = 1;
                }
                break;
            }
        }
    }
    
    return ret;
}

int inputValidate(Input *input)
{
    if (input)
    {
        enum InputType type = input->itype;
        
        const int initialy_valid = (input->flags & INPUT_FLAG_VALID);
        
        input->flags &= ~INPUT_FLAG_VALID;
        
        if (0 == input->textBuffer[0])
        {
            input->flags |= INPUT_FLAG_VALID;
            
            if (!initialy_valid)
            {
                if (input->valid)
                {
                    input->valid(input);
                }
                
            }            
            return 1;
        }
        
        if (Input_Config == type)
        {
            if (input->config)
            {
                if (ConfigType_String == input->config->type)
                {
                    type = Input_Text;
                }
                else
                if (ConfigType_Double == input->config->type)
                {
                    type = Input_Number;
                }
                else
                if (ConfigType_Int == input->config->type)
                {
                    type = Input_Number;
                }
            }
        }

        switch(type)
        {
            case Input_None: {
                break;
            }
            
            case Input_Config:
                ERROR1("Error: typeless config!\n");
            case Input_Text: {
                    input->flags |= INPUT_FLAG_VALID;
                    if (!initialy_valid)
                    {
                        if (input->valid)
                        {
                            input->valid(input);
                        }
                        
                    }                                
                break;
            }
            case Input_Number: {
                int num_comma = 0;
                int num_dot = 0;
                int num_sign = 0;
                
                const char *s = NULL; 
                for (s = input->textBuffer; *s; s++)
                {
                    if ('.' == *s)
                        num_dot++;
                    else
                    if (',' == *s)
                        num_comma++;
                    else
                    if ('-' == *s)
                        num_sign++;                    
                    else                        
                    if (!isdigit(*s))
                    {
                        break;
                    }
                }
                
                if (0 == *s)
                {
                    if (((num_dot + num_comma) >= 0) &&
                        ((num_dot + num_comma) <= 1) &&
                        (num_sign <= 1) && 
                        (num_sign >= 0))
                    {
                        input->flags |= INPUT_FLAG_VALID;
                        if (!initialy_valid)
                        {
                            if (input->valid)
                            {
                                input->valid(input);
                            }
                        }
                    }
                }
                
                if (initialy_valid && (0 == (input->flags &INPUT_FLAG_VALID)))
                {
                    if (input->valid)
                    {
                        input->valid(input);
                    }
                }                        
                
                break;
            }
            default:
                ;
         }
            
        return (input->flags & INPUT_FLAG_VALID);
    }
    
    return 0;
}


