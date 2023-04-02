#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/* Popup for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "popup.h"
#include "viewer.h"
#include "widget.h"

#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 20
#define BUTTON_INCREMENT 25
#define BUTTON_X 10
#define BUTTON_START_Y 10

ViewerWidget *okButton = NULL;
#define OK_LABEL "Ok"
#define OK_BUTTON_X BUTTON_X
#define OK_BUTTON_Y BUTTON_START_Y
#define OK_BUTTON_WIDTH BUTTON_WIDTH
#define OK_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *yesButton = NULL;
#define YES_LABEL "Yes"
#define YES_BUTTON_X BUTTON_X
#define YES_BUTTON_Y BUTTON_START_Y
#define YES_BUTTON_WIDTH BUTTON_WIDTH
#define YES_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *noButton = NULL;
#define NO_LABEL "No"
#define NO_BUTTON_X BUTTON_X
#define NO_BUTTON_Y BUTTON_START_Y
#define NO_BUTTON_WIDTH BUTTON_WIDTH
#define NO_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *popupWidget= NULL;
#define POPUP_LABEL "popup"
#define POPUP_BUTTON_X BUTTON_X
#define POPUP_BUTTON_Y BUTTON_START_Y
#define POPUP_BUTTON_WIDTH BUTTON_WIDTH
#define POPUP_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *requestWidget= NULL;
#define REQUEST_LABEL "Request"
#define REQUEST_BUTTON_X BUTTON_X
#define REQUEST_BUTTON_Y BUTTON_START_Y
#define REQUEST_BUTTON_WIDTH BUTTON_WIDTH
#define REQUEST_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *choiceWidget= NULL;
#define CHOICE_LABEL "Choice"
#define CHOICE_BUTTON_X BUTTON_X
#define CHOICE_BUTTON_Y BUTTON_START_Y
#define CHOICE_BUTTON_WIDTH BUTTON_WIDTH + 10
#define CHOICE_BUTTON_HEIGHT BUTTON_HEIGHT

ViewerWidget *cancelButton = NULL;
#define CANCEL_LABEL "Cancel"
#define CANCEL_BUTTON_X BUTTON_X
#define CANCEL_BUTTON_Y BUTTON_START_Y
#define CANCEL_BUTTON_WIDTH BUTTON_WIDTH
#define CANCEL_BUTTON_HEIGHT BUTTON_HEIGHT

static PopupUserChoice popup_choice_visible = NULL;
static void *popup_data_visible = NULL;
static ViewerWidget* choice_buttons[15];
static int popupButtonCallback(ViewerWidget *w, int ev, int x, int y, void *data);
static char popupFileExtension[32];
static char fileChoiceText[1000];
static char textBuffer[100];

void fillFileChoices(const char* path);


void popupInit()
{    
    
    popupFileExtension[0] = 0;
    
    
    if (NULL == popupWidget)
    {
        popupWidget = widgetCreateWidget(NULL,POPUP_LABEL,VIEWERWIDGET_TYPE_WINDOW,POPUP_BUTTON_X,POPUP_BUTTON_Y,POPUP_BUTTON_WIDTH,POPUP_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        okButton = widgetCreateWidget(popupWidget,OK_LABEL, VIEWERWIDGET_TYPE_BUTTON,OK_BUTTON_X,OK_BUTTON_Y,OK_BUTTON_WIDTH,OK_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        requestWidget = widgetCreateWidget(NULL,REQUEST_LABEL, VIEWERWIDGET_TYPE_WINDOW,REQUEST_BUTTON_X,REQUEST_BUTTON_Y,REQUEST_BUTTON_WIDTH,REQUEST_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        yesButton = widgetCreateWidget(requestWidget,YES_LABEL, VIEWERWIDGET_TYPE_BUTTON,YES_BUTTON_X,YES_BUTTON_Y,YES_BUTTON_WIDTH,YES_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        noButton = widgetCreateWidget(requestWidget,NO_LABEL, VIEWERWIDGET_TYPE_BUTTON,NO_BUTTON_X,NO_BUTTON_Y,NO_BUTTON_WIDTH,NO_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        choiceWidget = widgetCreateWidget(NULL,CHOICE_LABEL, VIEWERWIDGET_TYPE_WINDOW,CHOICE_BUTTON_X,CHOICE_BUTTON_Y,CHOICE_BUTTON_WIDTH,CHOICE_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);

        cancelButton = widgetCreateWidget(choiceWidget,CANCEL_LABEL, VIEWERWIDGET_TYPE_BUTTON,CANCEL_BUTTON_X,CANCEL_BUTTON_Y,CANCEL_BUTTON_WIDTH,CANCEL_BUTTON_HEIGHT,
                                        popupButtonCallback,NULL);
        
        widgetInvisible(choiceWidget);
        widgetInvisible(popupWidget);
        widgetInvisible(requestWidget);
        
        memset(choice_buttons,0,sizeof(choice_buttons));
    }
}

void removeChoices()
{
    for (unsigned int j = 0; j < sizeof(choice_buttons)/sizeof(choice_buttons[0]);j++)
    {
        if (choice_buttons[j])
        {
            if (choice_buttons[j]->data)
            {
                memory_free(choice_buttons[j]->data);
            }
            widgetRemoveWidget(choice_buttons[j]);
        }
    }
    
    memset(choice_buttons,0,sizeof(choice_buttons));
}

int popupButtonCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)x;
    (void)y;
    (void)data;
    
#ifdef _DEBUG_EVENTS    
    LOG("popup Callback:W%i: %i at (%i,%i)\n",w->index,ev, x, y);
#endif    

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == okButton))
    {
        if (popup_choice_visible)
        {
            popup_choice_visible(POPUP_CHOICE_OK,popup_data_visible);
        }
        popup_choice_visible = NULL;
        popup_data_visible = NULL;
        widgetSetText(okButton,NULL);
        widgetSetText(popupWidget,NULL);
        widgetInvisible(popupWidget);  
        widgetEnableDesktop(1, NULL);
        return 1; // consume event
    }

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == yesButton))
    {
        if (popup_choice_visible)
        {
            popup_choice_visible(POPUP_CHOICE_YES,popup_data_visible);
        }
        popup_choice_visible = NULL;
        popup_data_visible = NULL;
        widgetSetText(yesButton,NULL);
        widgetSetText(noButton,NULL);
        widgetSetText(requestWidget,NULL);
        widgetInvisible(requestWidget);
        widgetEnableDesktop(1, NULL);        
        return 1; // consume event
    }

    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == noButton))
    {
        if (popup_choice_visible)
        {
            popup_choice_visible(POPUP_CHOICE_NO,popup_data_visible);
        }
        popup_choice_visible = NULL;  
        popup_data_visible = NULL;
        widgetSetText(yesButton,NULL);
        widgetSetText(noButton,NULL);
        widgetSetText(requestWidget,NULL);
        widgetInvisible(requestWidget);        
        widgetEnableDesktop(1, NULL);
        return 1; // consume event
    }
    
    if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == cancelButton))
    {
        if (popup_choice_visible)
        {
            popup_choice_visible(-1,popup_data_visible);
        }
        popup_choice_visible = NULL;  
        popup_data_visible = NULL;
        widgetInvisible(choiceWidget); 
        widgetEnableDesktop(1, NULL);
        removeChoices();
        return 1; // consume event
    }
    
    for (unsigned int i = 0; i < sizeof(choice_buttons)/sizeof(choice_buttons[0]);i++)
    {
        if (NULL == choice_buttons[i])
        {
            break;
        }
        
        if ((VIEWEREVENT_BUTTONRELEASE == ev) && (w == choice_buttons[i]))
        {   
            if (strrchr(w->name,'/'))
            {
                char path[128];
                strncpy(path,w->name,sizeof(path));
                char *sep = strrchr(path,'/');
                if (sep)
                {
                    *sep = 0;
                }
                if (chdir(path))
                {
                    perror("chdir");
                    viewerPrintError(errno,"Change dir failed:\n\"%s\"",path);                    
                }
                else
                if (getcwd(path, sizeof(path)))
                {
                    fillFileChoices(path);
                }
                else
                {
                    perror("getcwd");
                    viewerPrintError(errno,"Get cwd dir failed:\n\"%s\"",path);                                        
                }
            }
            else
            {
                if (popup_choice_visible)
                {
                    // char filename[100];
                    if (NULL == popup_data_visible)
                    {
                        popup_data_visible = choice_buttons[i]->data;
                        // snprintf(filename,sizeof(filename),"%s.%s",w->name,popupFileExtension);
                        // popup_data_visible = (void*)filename;
                    }
                    popup_choice_visible(i,popup_data_visible);
                }

                popup_choice_visible = NULL;  
                popup_data_visible = NULL;
                popupFileExtension[0] = 0;
                
                widgetEnableDesktop(1, NULL);
                widgetInvisible(choiceWidget);
                removeChoices();
            }   

            LOG_FLUSH;

            return 1;
        }        
    }

    return 0;
}

static int position(ViewerWidget *widget)
{
    int width,height;
    
    viewerGetWindowSize(&width, &height);

    int pwidth = widget->width;
    int pheight = widget->height;
    int x = (width - pwidth) / 2;
    int y = (pheight/2);

    // widgetSizeWidget(widget, pwidth, pheight);
    widgetMoveWidget(widget,x, y);
    
    if (widget == popupWidget)
    {
        int ok_x = (pwidth/2) - okButton->width/2;
        int ok_y = pheight - okButton->height - 15;
        okButton->flags |= VIEWERWIDGET_FLAG_ESCAPE;
        widgetMoveWidget(okButton, ok_x, ok_y);
    }
    else
    if (widget == requestWidget)
    {
        int no_x = pwidth - noButton->width - 10;
        int no_y = pheight - okButton->height - 15;

        noButton->flags |= VIEWERWIDGET_FLAG_ESCAPE;
        widgetMoveWidget(noButton, no_x, no_y);
        
        int yes_x = 10;
        int yes_y = pheight - okButton->height - 15;
        
        widgetMoveWidget(yesButton, yes_x, yes_y);
    }    
    else
    {
        int ca_x = pwidth - cancelButton->width - 10;
        int ca_y = pheight - cancelButton->height - 10;

        cancelButton->flags |= VIEWERWIDGET_FLAG_ESCAPE;
        widgetMoveWidget(cancelButton, ca_x, ca_y);
        
        const int width_ext = 5;
        const int height_ext = 10;
        const int width_offset = 10;
        const int height_offset = 50;
        
        const int num_columns = (pwidth - width_offset)/ (CHOICE_BUTTON_WIDTH + width_ext);
        const int num_rows = (pheight - height_offset)/ (CHOICE_BUTTON_HEIGHT + height_ext);
        
        int row = 0;
        int column = 0;
        for (unsigned int j = 0; j < sizeof(choice_buttons)/sizeof(choice_buttons[0]);j++)
        {
            if (choice_buttons[j])
            {
                int c_x = ((choice_buttons[j]->width + width_ext)* column) + width_offset;
                int c_y = ((choice_buttons[j]->height +height_ext)*row) + height_offset;                
                widgetMoveWidget(choice_buttons[j], c_x, c_y);
            
                row++;                
                if (row >= num_rows)
                {
                    row = 0;
                    column++;                    
                    if (column > num_columns)
                    {
                        break;
                    }
                }                
            }
        }

    }

//    fprintf(stderr,"positionPopup: width = %i, height = %i, p=(%i,%i), b=(%i,%i)\n",pwidth,pheight,x,y,ok_x,ok_y);
    
    return 0;
}

int popupResize(int width, int heigth)
{
    widgetSizeWidget(popupWidget, width, heigth);

    int wwidth,wheight;    
    viewerGetWindowSize(&wwidth, &wheight);     
    
    int x = (wwidth - width)/2;
    int y = (wheight- heigth)/2;
    widgetMoveWidget(popupWidget, x, y);
    
    int ok_x = (width/2) - okButton->width/2;
    int ok_y = heigth - okButton->height - 15;
    
    return widgetMoveWidget(okButton, ok_x, ok_y);
}

int popup(const char *text, PopupUserChoice ok)
{
    if (popupWidget && text)
    {
        popup_choice_visible = ok;
        popup_data_visible = NULL;

        widgetEnableDesktop(0, popupWidget);

        strncpy(fileChoiceText,text,sizeof(fileChoiceText));
        widgetSetText(popupWidget,fileChoiceText);
        
        int width,height;    
        viewerGetWindowSize(&width, &height);     
        
        widgetSizeWidget(popupWidget, width/2, height/2);
        
        position(popupWidget);
        widgetVisible(popupWidget);        
    }
    
    return 0;
}

int userRequest(const char* text, const char *yesName, const char* noName, PopupUserChoice choice, void *data)
{
    if (requestWidget && text)
    {        
        if (NULL == popup_choice_visible)
        {
            popup_choice_visible = choice;
            popup_data_visible = data;
            
            widgetEnableDesktop(0, requestWidget);           

            strncpy(fileChoiceText,text,sizeof(fileChoiceText));
            widgetSetText(requestWidget,fileChoiceText);

            strncpy(popupFileExtension,yesName,sizeof(popupFileExtension));
            widgetSetText(yesButton,popupFileExtension);

            strncpy(textBuffer,noName,sizeof(textBuffer));
            widgetSetText(noButton,textBuffer);
            
            int width,height;    
            viewerGetWindowSize(&width, &height);     
            
            widgetSizeWidget(requestWidget, width/2, height/2);
            
            position(requestWidget);
            widgetVisible(requestWidget);                
        }
    }
    
    return 0;
}

int userChoice(const char* text, const char** choices, PopupUserChoice choice, void *data)
{
    if (choiceWidget && text)
    {        
        if (NULL == popup_choice_visible)
        {
            popup_choice_visible = choice;
            popup_data_visible = data;
            widgetEnableDesktop(0, choiceWidget);

            strncpy(fileChoiceText,text,sizeof(fileChoiceText));
            widgetSetText(choiceWidget,fileChoiceText);

            for (unsigned int i = 0; i < sizeof(choice_buttons)/sizeof(choice_buttons[0]);i++)
            {
                if ((NULL == choices[i]) || (0 == *choices[i]))
                {
                    break;
                }
                
                choice_buttons[i] = widgetCreateWidget(choiceWidget, choices[i], VIEWERWIDGET_TYPE_BUTTON,CHOICE_BUTTON_X,CHOICE_BUTTON_Y,CHOICE_BUTTON_WIDTH,CHOICE_BUTTON_HEIGHT,
                                            popupButtonCallback,NULL);
                
                widgetVisible(choice_buttons[i]);
            }
            
            int width,height;    
            viewerGetWindowSize(&width, &height);     
            
            widgetSizeWidget(choiceWidget, width/2, height/2);
            
            position(choiceWidget);
            widgetVisible(choiceWidget);                
        }
    }
    
    return 0;
}

int fileFilter(const struct dirent *de)
{
    return (de && ((DT_DIR == de->d_type) || (DT_REG == de->d_type)));    
}

int fileCompar(const struct dirent **d1, const struct dirent **d2)
{
    if (d1 && d2)
    {
        const char *name1 = (*d1)->d_name;
        const char *name2 = (*d2)->d_name;
        
        
        if ((DT_DIR == (*d1)->d_type )&& (DT_DIR == (*d2)->d_type ))
        {
            return strcoll(name1,name2);
        }
        
        if (DT_DIR == (*d1)->d_type )
        {
            return 1;
        }
        if (DT_DIR == (*d2)->d_type )
        {
            return -1;
        }
        
        return strcoll(name1,name2);
    }
    
    return 0;
}

void fillFileChoices(const char* path)
{
    if (path)
    {     
        struct dirent **namelist = NULL;
        
        // fprintf(stderr,"Path: %s\n",path);
        
        // Update path on popup
        snprintf(fileChoiceText,sizeof(fileChoiceText),"%s\n%s",textBuffer,path);
        widgetSetText(choiceWidget,fileChoiceText);

        removeChoices();
        
        int num_dir = scandir(path, &namelist,fileFilter,fileCompar);
        
        unsigned int button_index = 0;
        int i = 0;
        // LOG("File extension \"%s\"\n",popupFileExtension);
        
        for (struct dirent *d = namelist[i]; d && (i<num_dir); d = namelist[++i])
        {
            int type = d->d_type;
            const char *ext = strrchr(d->d_name,'.');
            const char *sep = strrchr(d->d_name,'/');
            if (ext)
            {
                if (sep < ext)
                {
                    // No seperator but extension
                                        
                    char buffer[100];
                    char *b = buffer;
                    if (DT_REG == d->d_type)
                    {                    
                        type = 0;
                        const char *extension = popupFileExtension;

                        while(1)
                        {
                            if ((';' == *extension) || (0 == *extension))
                            {                            
                                // LOG("Checking extension: \"%s\" against \"%s\"\n",buffer, &ext[1]);
                                if (0 == strcasecmp(&ext[1],buffer))
                                {
                                    type = d->d_type;   
                                    break;
                                }
                                b = buffer;
                                extension++;
                            }
                            
                            if ((0 == *extension) || (extension - popupFileExtension >= (int)sizeof(buffer)))
                            {
                                break;
                            }
                            
                            *b = *extension;
                            b++;
                            *b = 0;
                            extension++;
                        }
                    }
                    else                         
                    if (DT_DIR != d->d_type)
                    {
                        type = 0;
                    }                                                  
                }
                else
                {
                    // seperator and extension
                    if (DT_DIR != type)
                    {                        
                        type = 0;
                    }
                }
            }
            else
            {
                // No extension and no seperator
                if (DT_DIR != type)
                {                        
                    type = 0;
                }                    
            }
            
            if (type)
            {
                if ((button_index < sizeof(choice_buttons)/sizeof(choice_buttons[0])))
                {
                    char label[200];
                    const char *name = basename(d->d_name);
                    strncpy(label,name,sizeof(label));

                    char *ext = strrchr(label,'.');
                    if (ext)
                    {
                        if (DT_DIR != type)
                        {
                            *ext = 0;                            
                        }
                        else
                        {
                            if (('.' == label[0]) && (0 == label[1]))
                            {
                                label[0] = 0;
                            }
                        }
                    }

                    if (('.' == label[0]) && (isalnum(label[1])))
                    {
                        label[0] = 0;
                    }                            

                    if (label[0])
                    {
                        if (DT_DIR == type)
                        {
                            commenStringCat(label,"/",sizeof(label));
                        }
                                                    
                        choice_buttons[button_index] = widgetCreateWidget(choiceWidget,label, VIEWERWIDGET_TYPE_BUTTON,
                                                                        CHOICE_BUTTON_X,
                                                                        CHOICE_BUTTON_Y,
                                                                        CHOICE_BUTTON_WIDTH,
                                                                        CHOICE_BUTTON_HEIGHT,
                                                                        popupButtonCallback, NULL);      
                        
                        if (DT_REG == type)
                        {
                            choice_buttons[button_index]->color = aperalGetColor(GAPERAL_CHOICE_BUTTON_HIGHLIGHT);
                            
                            choice_buttons[button_index]->data = memory_strdup(name);
                        }
                        
                        widgetVisible(choice_buttons[button_index]);
                        
                        button_index++;
                    }
                }
                else
                {
                    break;
                }
            }
            
            free(d);
        }
        
        free(namelist);
        
        position(choiceWidget);
    }
    
}

int fileChoice(const char* text, const char* path, const char* extension, PopupUserChoice choice)
{
    if (choiceWidget && text && path && extension)
    {                
        if (NULL == popup_choice_visible)
        {            
            popup_choice_visible = choice;
            popup_data_visible = NULL;                       

            widgetEnableDesktop(0, choiceWidget);

            strncpy(textBuffer,text,sizeof(textBuffer));
            strncpy(popupFileExtension,extension,sizeof(popupFileExtension));
            chdir(path);
            fillFileChoices(path);
            
            int width,height;    
            viewerGetWindowSize(&width, &height);     
            
            widgetSizeWidget(choiceWidget, width/2, height/2);
            
            position(choiceWidget);
            
            widgetVisible(choiceWidget);                            
        }
    }
    
    return 0;
}


