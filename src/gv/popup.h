#ifndef __POPUP__
#define __POPUP__

/* Popup for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define POPUP_CHOICE_YES 1
#define POPUP_CHOICE_NO  0
#define POPUP_CHOICE_OK  0

typedef void (*PopupUserChoice)(int choice, void *data);

void popupInit();

int popup(const char *text, PopupUserChoice ok);

int popupResize(int width, int heigth);

int userRequest(const char* text, const char *yesName, const char* noName, PopupUserChoice choice, void *data);
int userChoice(const char* text, const char** choices, PopupUserChoice choice, void *data);

int fileChoice(const char* text, const char* path, const char* extension, PopupUserChoice choice);

#endif