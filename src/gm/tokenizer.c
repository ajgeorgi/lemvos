#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Tokenizer for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "tokenizer.h"


typedef struct TokenizerT {
    char tokenbuffer[500];
    char push_back_tokenbuffer[200];
    
    char *linebuffer;
    size_t linebuffer_size;
    ssize_t line_pointer;
    ssize_t line_end;
    FILE *_loaderFile;
    int inside_string;
    int push_back_consumed;
} Tokenizer;

static Tokenizer _tokenizer;

static int isAlpha(char c)
{
    return (isalnum(c) || ('_' == c) || ('.' == c));
}

static int isNumeric(char c)
{
    return (isdigit(c) || ('-' == c) || ('+' == c) || ('.' == c)  || ('e' == c));
}

static int isUsefulSeperator(char c)
{
    const char* seperator = "([\"\'";

    for (const char*s = seperator; *s; s++)
    {
        if (c == *s)
        {
            return 1;
        }
    }
    
    return 0;
}

static int isIgnoreSeperator(char c)
{
    const char* seperator = ":;= \n\t";

    for (const char*s = seperator; *s; s++)
    {
        if (c == *s)
        {
            return 1;
        }
    }
    
    return 0;
}

static int isSeperator(char c)
{
    return (isUsefulSeperator(c) || isIgnoreSeperator(c));
}

static int isCommend(char c)
{    
    return ('#' == c);
}

int tokenizerPushBack(const char* token)
{
    if (token && _tokenizer.push_back_consumed)
    {
        strncpy(_tokenizer.push_back_tokenbuffer,token,sizeof(_tokenizer.push_back_tokenbuffer));
    
        _tokenizer.push_back_consumed = 0;
        
        return 0;
    }
    
    return -1;
}

void resetTokenizer(FILE *loaderFile)
{
    if (_tokenizer.linebuffer)
    {
        free(_tokenizer.linebuffer);
    }
    
    memset(&_tokenizer,0,sizeof(_tokenizer));
    
    _tokenizer.push_back_consumed = 1;
    _tokenizer._loaderFile = loaderFile;
    
    if (loaderFile)
    {
        rewind(loaderFile);
    }
    
    // fprintf(stderr,"Reset tokenizer\n");
}

#define FLAGS_INSIDE_STRING (1<<0)
#define FLAGS_INSIDE_PARAM (1<<1)
#define FLAGS_INSIDE_QUOTE (1<<2)
#define FLAGS_INSIDE_INDEX (1<<3)

char* tokenizerToken()
{
    if (0 == _tokenizer.push_back_consumed )
    {
        _tokenizer.push_back_consumed = 1;
        return _tokenizer.push_back_tokenbuffer;
    }
    
    if (_tokenizer._loaderFile)
    {
        /*
        if (((_tokenizer.line_pointer >= _tokenizer.line_end) && (_tokenizer.line_pointer > 0)) ||
            (_tokenizer.line_end < 0))
        {
            return NULL;
        }
        */
        if (feof(_tokenizer._loaderFile))
        {
            return NULL;
        }
        
        _tokenizer.tokenbuffer[0] = 0;
        
        char *begin = NULL;
        char *end = NULL;

        while (NULL == begin)
        {
            if (_tokenizer.line_pointer >= _tokenizer.line_end)
            {     
                // fprintf(stderr,"Reading new line\n");
                begin = NULL;
                end = NULL;
                
                memset(_tokenizer.linebuffer, 0, _tokenizer.linebuffer_size);
                _tokenizer.line_end = getline(&_tokenizer.linebuffer, &_tokenizer.linebuffer_size, _tokenizer._loaderFile);
                _tokenizer.line_pointer = 0;  
                
                if (0 >= _tokenizer.line_end)
                {
                    return NULL;
                }
            }

            int flags = 0;
            
            if (_tokenizer.inside_string)
            {
                // Restart from inside a string
                flags |= FLAGS_INSIDE_STRING;    
                begin = &_tokenizer.linebuffer[_tokenizer.line_pointer];
            }
            
            for (; _tokenizer.line_pointer <= _tokenizer.line_end; _tokenizer.line_pointer++)
            {
                char *symbol = &_tokenizer.linebuffer[_tokenizer.line_pointer];
                
                if (_tokenizer.line_pointer == _tokenizer.line_end)
                {
                    // The finale char in line has been reched and no end has been found
                    
                    end = symbol;
                    break;
                }
                
                if (isCommend(*symbol))
                {
                    // commend line. just skip
                    _tokenizer.line_pointer = _tokenizer.line_end;
                    break;
                }
                
                if (0 == flags)
                {
                    if (begin && isSeperator(*symbol))
                    {
                        // Seperator has been reached and symbol is finished
                        end = symbol;
                        break;
                    }            
                }
                
                if ('\"' == *symbol)
                {
                    if (_tokenizer.inside_string)
                    {
                        _tokenizer.inside_string = 0;
                        flags &= ~FLAGS_INSIDE_STRING;
                        if (0 == flags)
                        {                            
                            _tokenizer.line_pointer++;
                            end = symbol + 1;
                            break;                        
                        }
                    }
                    else
                    {
                        _tokenizer.inside_string = 1;
                        flags |= FLAGS_INSIDE_STRING;
                    }                        
                }
                if (')' == *symbol)
                {
                    flags &= ~FLAGS_INSIDE_PARAM;
                    if (0 == flags)
                    {
                        _tokenizer.line_pointer++;
                        end = symbol + 1;
                        break;                        
                    }
                }
                if ('\'' == *symbol)
                {
                    if (flags & FLAGS_INSIDE_QUOTE)
                    {
                        flags &= ~FLAGS_INSIDE_QUOTE;
                        if (0 == flags)
                        {
                            _tokenizer.line_pointer++;
                            end = symbol + 1;
                            break;                        
                        }
                    }
                    else
                    {
                        flags |= FLAGS_INSIDE_QUOTE;
                    }                        
                }
                if ('(' == *symbol)
                {
                    flags |= FLAGS_INSIDE_PARAM;
                }
                if ('[' == *symbol)
                {
                    flags |= FLAGS_INSIDE_INDEX;
                }
                if (']' == *symbol)
                {
                    flags &= ~FLAGS_INSIDE_INDEX;
                    if (0 == flags)
                    {
                        _tokenizer.line_pointer++;
                        end = symbol + 1;
                        break;                        
                    }
                }
                                
                if ((NULL == begin) && (isAlpha(*symbol) || isUsefulSeperator(*symbol) || isNumeric(*symbol)))
                {
                    begin = symbol;
                }                
            }
            
        }
        
        if (end && begin)
        {
            if (begin < end)
            {            
                // Copy from line buffer to token buffer
                memcpy(_tokenizer.tokenbuffer,begin,end - begin);
                _tokenizer.tokenbuffer[end - begin] = 0;
            }
            else
            {
                _tokenizer.tokenbuffer[0] = 0;
            }
                        
            /*
            fprintf(stderr,"Begin: \"%s\", Token: \"%s\", lp = %zi, le = %zi\n",
                    begin?begin:"<nulll>",
                    _tokenizer.tokenbuffer,
                    _tokenizer.line_pointer,
                    _tokenizer.line_end);            
                    */
            return _tokenizer.tokenbuffer;
        }
    }
    
    return NULL;
}
