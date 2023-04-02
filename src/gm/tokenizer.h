#ifndef __TOKENIZER__
#define __TOKENIZER__

#include <stdio.h>

/* Tokenizer for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

int resetTokenizer(FILE *loaderFile);

char* tokenizerToken();

int tokenizerPushBack(const char* token);

#endif // __TOKENIZER__