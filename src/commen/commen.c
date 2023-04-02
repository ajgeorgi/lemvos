#include <string.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#define __USE_XOPEN  
#include <time.h>
#undef __USE_XOPEN  

#ifdef _DEBUG_MEMORY
#include <mcheck.h>
#endif

/* Commen for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "commen.h"


#define MEMORY_MAGIC 0xFEEFABC1

CommonErrorHandler commen_error_handler = NULL;


#ifdef _DEBUG    

#define LOG_FILE_PATH "/tmp"

FILE* __err_log = NULL;

int _commenTerminate()
{
    fprintf(stderr,"abnormal program termination!!!\n");

    if (__err_log)
    {
        LOG1("abnormal program termination!!!\n");
        fclose(__err_log);
        __err_log = NULL;
    }
    
    abort();
    
    return -1;
}

static const char *_min_mem = NULL;
static const char *_max_mem = NULL;

#ifdef _DEBUG_MEMORY
static void mcheckAbortfunc(enum mcheck_status mstatus)
{
    static const char *mchech_str[] = {
        "MCHECK_DISABLED",         /* = -1, Consistency checking is not turned on.  */
        "MCHECK_OK",                    /* Block is fine.  */
        "MCHECK_FREE",                  /* Block freed twice.  */
        "MCHECK_HEAD",                  /* Memory before the block was clobbered.  */
        "MCHECK_TAIL"            
    } ;             
    
    ERROR("mcheck failed with %s\n",mchech_str[mstatus + 1]);
    
    if (__err_log)
    {
        fflush(__err_log);
    }
    
    _commenTerminate();
}
#endif

int _check_memory(const void *obj)
{
    if (obj)
    {
#ifdef _DEBUG_MEMORY

        static const char *mchech_str[] = {
            "MCHECK_DISABLED",         /* = -1, Consistency checking is not turned on.  */
            "MCHECK_OK",                    /* Block is fine.  */
            "MCHECK_FREE",                  /* Block freed twice.  */
            "MCHECK_HEAD",                  /* Memory before the block was clobbered.  */
            "MCHECK_TAIL"            
        } ;             
#endif

        char *memory = ((char*)obj) -  sizeof(magic_number);
        magic_number *mem_num= (magic_number *)memory;

        if (_min_mem)
        {
            if (((const char*)obj > _max_mem) || ((const char*)obj < _min_mem))
            {
                FATAL("Corrupt mem at %p\n",obj);

                if (__err_log)
                {
                    fflush(__err_log);
                }

                return 0;
            }
        }

        if (MEMORY_MAGIC == *mem_num)
        {
#ifdef _DEBUG_MEMORY
            enum mcheck_status mstatus = mprobe((void*)memory);
            
            if (MCHECK_OK == mstatus)
            {
                return 1;
            }

            if (__err_log)
            {
                fflush(__err_log);
            }

            FATAL("mprobe failed with %s\n",mchech_str[mstatus + 1]);

            return 0; //  Memory has been proven corrupt

#else
            // no memcheck possible so make it sane
            return 1;
#endif
            
        }
        else
        {
            // Not my memory. No checks possible here. Assume sane memory
            return 1;
        }
    }
    
    return 1; // No memory is sane
}
#endif

void initCommen(const char* appName, CommonErrorHandler errHandler)
{
    commen_error_handler = errHandler;

#ifdef _DEBUG_MEMORY
    if (0 == mcheck_pedantic(mcheckAbortfunc))
    {
        LOG1("Activated mcheck!\n");
    }
    else
    {
        ERROR1("Failed to activate mcheck\n");
        perror("mcheck");
    }
#endif

#ifdef _DEBUG
    char fileName[100];
    snprintf(fileName,sizeof(fileName),"%s/lemvos_%s.log",LOG_FILE_PATH,appName?appName:"XX");
    fprintf(stderr,"-->> See %s for messages\n",fileName);
    __err_log = fopen(fileName,"w");
#endif

}

void finalizeCommen()
{
#ifdef _DEBUG    
    if (__err_log)
    {
        LOG1("normal program termination.\n");
        fclose(__err_log);
        __err_log = NULL;
    }    
#endif    

    fprintf(stderr,"normal termination.\n");
}


void *memory_aloc(size_t size)
{
    char *memory = (char*) malloc(size + sizeof(magic_number));
    
    if (memory)
    {
        magic_number *mem_num= (magic_number *)memory;
        mem_num[0] = MEMORY_MAGIC;
            
        if (NULL == _min_mem)
        {
            _min_mem = memory;
        }
                    
        if (memory < _min_mem)
        {
            _min_mem = memory;
        }

        if ((memory + size  + sizeof(magic_number)) > _max_mem)
        {
            _max_mem = memory + size  + sizeof(magic_number);
        }
        
        return memory + sizeof(magic_number);
    }
    
    FATAL("Deep trouble! No more memory! Requested size is %lu\n",size);
    
    return NULL;
}

void memory_free(void *mem)
{
    char *memory = ((char*)mem) -  sizeof(magic_number);
    magic_number *mem_num = (magic_number *)memory;
    
    if (MEMORY_MAGIC == *mem_num)
    {    
        free(memory);
    }
    else
    {
        FATAL("Can not free memory at %p\n",mem);
    }
}

int memory_is_my_memory(void *mem)
{
    char *memory = ((char*)mem) -  sizeof(magic_number);
    magic_number *mem_num = (magic_number *)memory;
    
    if (MEMORY_MAGIC == *mem_num)
    {    
        return 1;
    }
    
    return 0;
}

void *memory_realloc(void* mem, size_t size)
{
    if (mem)
    {
        char *memory = ((char*)mem) -  sizeof(magic_number);
        magic_number *mem_num = (magic_number *)memory;

        if (MEMORY_MAGIC == *mem_num)
        {
            memory = (char*) realloc(memory,size + sizeof(magic_number));
             
            if (memory)
            {
                if (NULL == _min_mem)
                {
                    _min_mem = memory;
                }
                            
                if (memory < _min_mem)
                {
                    _min_mem = memory;
                }

                if ((memory + size + sizeof(magic_number)) > _max_mem)
                {
                    _max_mem = memory + size + sizeof(magic_number);
                }       
                
                magic_number *mem_num = (magic_number *)memory;
                if (MEMORY_MAGIC == *mem_num)
                {            
                    return memory + sizeof(magic_number);
                }
            }
            else
            {
                FATAL("Deep trouble! No more memory! Requested size is %lu\n",size);            
            }
        }
    }
    else
    {
        return memory_aloc(size);
    }
    
    FATAL("Failed to realocate memory. Requested size is %lu\n",size);
    
    return NULL;
}

char *memory_strdup(const char* str)
{
    size_t length = strlen(str);
    char* mem = memory_aloc(length + 1);
    strncpy(mem,str,length + 1);
    
    return mem;
}

char *memory_strndup(const char* str, size_t size)
{
    size_t length = strnlen(str,size);
    char* mem = memory_aloc(length + 1);
    strncpy(mem,str,length + 1);
    
    return mem;
}

#ifdef _DEBUG_MEMORY
void memory_check()
{
    mcheck_pedantic(mcheckAbortfunc);
    
    if (__err_log)
    {
        fflush(__err_log);
    }
}
#endif


void commenPrintError(int err, const char* text, ...)
{
    char textBuffer[256];
    textBuffer[0] = 0;
    
    va_list param;
    va_start(param, text);    
    vsnprintf(textBuffer, sizeof(textBuffer), text, param);    
    va_end(param);    
    
    if (commen_error_handler)
    {
        commen_error_handler(err,textBuffer);
    }
    else
    {
        fputs("**************", stderr);
        fputs(textBuffer, stderr);
    }    
    
#ifdef  _DEBUG
    if (__err_log)
    {
        fflush(__err_log);
    }
#endif
}


void commenStringCat(char *dest, const char* src, int len)
{
    if (dest && src && (0 < len))
    {
        char *end = dest + len;
        char *s = dest;
        const char *t = src;
        
        // Looking for end of string
        for (s = dest; (s < end) && *s; s++);
        
        if ((0 == *s) && (s < end))
        {
            // Append src to dest
            while((s < end) && *t)
            {
                *s = *t;
                t++;
                s++;
            }
            
            // Make sure we got a termination
            if (s < end)
            {
                *s = 0;
            }
            else
            {
                *(end-1) = 0;
            }
        }
    }
}

// Don't pass malloced or strduped strings here.
// The pointer will be modified so you can't free() it anymore.
// Use static string arrays here
char* commenStripString(char *string)
{        
    const char *begin_stripchar = "\" ({\t\n\r";
    const char *end_stripchar = "\")}\t\n\r";
    
    if (string)
    {
        char termchar = 0;
        char *begin = NULL;
        for (char *s = string; *s; s++)
        {
            const char *found_stripchar = NULL;
            
            if (NULL == begin)
            {
                found_stripchar = begin_stripchar;
            }
            else
            {
                found_stripchar = end_stripchar;
            }        
            
            for (;*found_stripchar && (*found_stripchar != *s) ; found_stripchar++)
            {
                // intentionaly left blank
            }
            
            if (NULL == begin)
            {
                if ('\"' == *found_stripchar)
                {
                    begin = s + 1;
                    termchar = *found_stripchar;
                }
                else
                if (0 == *found_stripchar)
                {
                    begin = s;
                }
            }
            else
            {
                if (termchar)
                {
                    if (*s == termchar)
                    {
                        *s = 0;
                        break;
                    }
                }
                else
                if (*found_stripchar)
                {
                    *s = 0;
                    break;
                }            
            }
        }
        
        return begin;
    }
    
    return NULL;
}

int commenCopyString(char *str, const char* str1, size_t strsize)
{
    if (NULL == str1)
    {
        str1 = UNKNOWN_SYMBOL;
    }
    
    if (str)
    {
        strncpy(str,str1,strsize);
    }
    
    str[strsize-1] = 0;
    
    return (str && str1) ? 0 : -1;
}



int commenTimeFromString(char *timestr, LTime *time)
{
    if (time)
    {
        if (timestr)
        {
            memset(time,0,sizeof(LTime));

            char *first = NULL;
            int bopen = 0;
            for(char *s = timestr; *s; s++)
            {        
                if ((NULL == first ) && isalnum(*s))
                {
                    first = s;
                }

                if (('"' == *s) && (1==bopen))
                {
                    *s = 0;
                    break;
                }

                if (('"' == *s) && (0 == bopen))
                {
                    bopen = 1;
                }
                
                if (')' == *s)
                {
                    *s = 0;
                    break;
                }
            }

            if (first)
            {
                int err = 0;
                const char*ptr = strptime(first, GM_TIME_FORMAT,time);

                if ((NULL == ptr) || (*ptr != 0))
                {
                    commenPrintError(errno,"Failed to convert time \"%s\" at \"%s\"\n",timestr,ptr?ptr:"<null>");
                    err = -1;
                }
#ifdef _DEBUG_TIME        
                fprintf(stderr,"time conversion \"%s\" -> %s\n",timestr,asctime(time));
#endif                
                if (!commenIsTime(time))
                {
                    fprintf(stderr,"Conversion: Time failed!!! %s\n",asctime(time));
                    err = -1;
                }
                
                return err;
            }
            else
            {
                commenPrintError(errno,"Failed to convert time \"%s\"\n",timestr);
            }
        }
        else
        {
            if (0 > commenNow(time))
            {
                commenPrintError(errno,"Failed to get real time\n");
            }
#ifdef _DEBUG_TIME        
            fprintf(stderr,"time now = %s\n",asctime(time));
#endif            
            return 1;
        }
    }
    
    return -1;
}


int commenGetFileModTime(const char *filename, LTime *modification)
{
    if (FILEMODTIME_CURRENT_FILENAME == filename)
    {
        if (modification)
        {
            memset(modification,0,sizeof(LTime));
        }
        
        return 1;
    }
    
    if (filename)
    {
        struct stat times;
        
        if (0 == stat(filename, &times))
        {
            time_t mod_time = MAX(times.st_atim.tv_sec,times.st_mtim.tv_sec);
            mod_time = MAX(times.st_ctim.tv_sec,mod_time);
            
            if (modification)
            {
                localtime_r((const time_t*) &mod_time, modification);
                
                if (!commenIsTime(modification))
                {
                    fprintf(stderr,"Conversion: Time failed!!! %s\n",asctime(modification));
                }
                
                return 0;
            }
        }
    }
    
    return -1;
}

double commenStringToDouble(const char* str)
{
    if (str)
    {
        char *sep = strchr(str,',');
        if (sep)
        {
            *sep = '.';
        }
        
        char *ptr = NULL;
        double x= strtod(str,&ptr);
        
        if (ptr && *ptr)
        {
            ERROR("Error: Failed to convert \"%s\" to value at \"%s\".\n",str,ptr?ptr:"<null>");
        }
        
        return x;
    }
    
    ERROR1("Error: Can not convert empty string to double.\n");
    
    return 0;
}

long commenStringToLong(const char* str)
{
    if (str)
    {
        const char* hexchar = "ABCDEFabcdefxX";
        int base = 10;
        
        for (const char* h = hexchar; *h; h++)
        {
            if (strchr(str,*h))
            {
                base = 16;
                break;
            }
        }
        
        char *endptr = NULL;
        long x = strtol(str, &endptr, base);
        
        if (endptr && *endptr)
        {
            ERROR("Error: Failed to convert \"%s\" to value at \"%s\".\n",str,endptr?endptr:"<null>");
        }
        
        return x;
    }
    
    ERROR1("Error: Can not convert empty string to long.\n");
    
    return 0;
}

unsigned long long commenStringToULongLong(const char* str)
{
    if (str)
    {
        const char* hexchar = "ABCDEFabcdefxX";
        int base = 10;
        
        for (const char* h = hexchar; *h; h++)
        {
            if (strchr(str,*h))
            {
                base = 16;
                break;
            }
        }
        
        char *endptr = NULL;
        unsigned long long x = strtoull(str, &endptr, base);
        
        if (endptr && *endptr)
        {
            ERROR("Error: Failed to convert \"%s\" to value at \"%s\".\n",str,endptr?endptr:"<null>");
        }
        
        return x;
    }
    
    ERROR1("Error: Can not convert empty string to unsigned long.\n");
    
    return 0;
}

long long commenStringToLongLong(const char* str)
{
    if (str)
    {
        const char* hexchar = "ABCDEFabcdefxX";
        int base = 10;
        
        for (const char* h = hexchar; *h; h++)
        {
            if (strchr(str,*h))
            {
                base = 16;
                break;
            }
        }
        
        char *endptr = NULL;
        long long x = strtoll(str, &endptr, base);
        
        if (endptr && *endptr)
        {
            ERROR("Error: Failed to convert \"%s\" to value at \"%s\".\n",str,endptr?endptr:"<null>");
        }
        
        return x;
    }
    
    ERROR1("Error: Can not convert empty string to unsigned long.\n");
    
    return 0;
}

unsigned long commenStringToULong(const char* str)
{
    if (str)
    {
        const char* hexchar = "ABCDEFabcdefxX";
        int base = 10;
        
        for (const char* h = hexchar; *h; h++)
        {
            if (strchr(str,*h))
            {
                base = 16;
                break;
            }
        }
        
        char *endptr = NULL;
        unsigned long x = strtoul(str, &endptr, base);
        
        if (endptr && *endptr)
        {
            ERROR("Error: Failed to convert \"%s\" to value at \"%s\".\n",str,endptr?endptr:"<null>");
        }
        
        return x;
    }
    
    ERROR1("Error: Can not convert empty string to unsigned long.\n");
    
    return 0;
}


EPoint* commenVertexFromString(char* param, EPoint *v, unsigned long long *flags)
{
    char *x1 = param;
    char *x2 = NULL;
    char *x3 = NULL;
    char *flagsString = NULL;
    
    for (char *i = param; *i; i++)
    {
        if (('(' == *i) && ((NULL == x1) || (param == x1)))
        {            
            x1 = i + 1;
        }        

        if ((',' == *i) && (NULL == x2))
        {            
            x2 = i + 1;
            *i = 0;
        }        
        
        if ((',' == *i) && (NULL == x3))
        {
            x3 = i + 1;
            *i = 0;
        }          

        if ((',' == *i) && (NULL == flagsString))
        {
            flagsString = i + 1;
            *i = 0;
        }          
        
        if (')' == *i)
        {
            *i = 0;
        }
    }
    
    v[0] = commenStringToDouble(x1);
    v[1] = commenStringToDouble(x2);
    v[2] = commenStringToDouble(x3);
    
    if (flags && flagsString)
    {
        *flags = commenStringToULongLong(flagsString);
    }
    
    return v;
}

int commenNow(LTime *time)
{
    if (time)
    {
        struct timespec tp;
        if (0 != clock_gettime(CLOCK_REALTIME, &tp))
        {
            commenPrintError(errno,"Failed to get real time\n");
        }
        else
        {
            localtime_r((const time_t*) &tp.tv_sec, time);    
            return 0;
        }
    }
    
    return -1;
}

int commenIsTime(LTime *time)
{
    if (time)
    {
        if (100 < time->tm_year)
        {
            LTime now;
            // commenNow(&now);
            // ???
            if ((0>commenNow(&now)) || (86400 < commenTimeDiff(time,&now)))
            {
                fprintf(stderr,"Time %s in future!\n",asctime(time));
            }
            
            return 1;
        }
#ifdef _DEBUG_TIME        
        fprintf(stderr,"Time failed: %s\n",asctime(time));
#endif
    }
    
    return 0;
}

long commenTimeDiff(LTime *time1, LTime *time2)
{
    return  timelocal(time1) - timelocal(time2);
}

const char *commenNameFromModel(const char* modelName)
{
    if (modelName && modelName[0])
    {
        const char *name = strchr(modelName,COMMEN_TYPE_SEPERATOR);
        if (name)
        {            
            return name +1;
        }
        
        return modelName;
    }
    return UNKNOWN_SYMBOL;
}


int commenIsLabel(const char* label)
{
    static const char extra_chars[] = "_.";
    
    if (label)
    {
        const char *s = label;
        while(*s)
        {
            if (!isalnum(*s))
            {
                const char *es = extra_chars;
                while (*es)
                {
                    if (*es == *s)
                    {
                        break;
                    }
                    es++;
                }
                if (0 == *es)
                {
                    return 0;
                }
            }
            s++;
        }
        
        return 1;
    }
    
    return 0;
}

RepresentationType commenGetRepresentation(const char* name)
{
    if (name)
    {
        int rep = 0;
        const char *s = name;
        const char *dot = strchr(name,COMMEN_TYPE_SEPERATOR);

        while (*s && ((NULL == dot) || (s < dot)))
        {
            rep += *s;
            s++;
        }
        
        if (dot)
        {
            char s1[100];
            strncpy(s1,name,dot-name);
            s1[dot-name-1] = 0;
            if (commenIsLabel(s1))
            {
                rep = rep << COMMEN_REPRESENTATION_SHIFT;
            }
        }
        
        return rep;
    }
    
    return 0;
}

const char *commenPrefixString(const char *prefix, char *string, int bufferSize)
{
    int prefixlen = strlen(prefix);
    int stringlen = strlen(string);

    if (prefixlen + stringlen + 1 < bufferSize)
    {
        memmove(&string[prefixlen],string,stringlen);
        memcpy(string,prefix,prefixlen);

        string[prefixlen+stringlen] = 0;
    }

    return string;
}

int isFileExists(const char*fileName)
{
   struct stat file_info;

   if (0  == stat(fileName,&file_info))
   {
       if (S_ISREG(file_info.st_mode))
       {
           return 1;
       }
   }

   return 0;
}


const char *commenNextInStringList(const char* strlist, char sep, char *buffer, int bufferSize)
{
    if (strlist)
    {
        const char* s = strlist;
        while (*s)
        {
            if (*s == sep)
            {
                s++;
                if (0 == *s)
                {
                    return NULL;
                }
                
                const char*e = strchr(s,sep);
                if (e)
                {
                    if (e-s < bufferSize)
                    {
                        memcpy(buffer,s,e-s);
                        buffer[e-s] = 0;
                        return s;
                    }
                }
                
                break;
            }
                        
            s++;
        }
        
        strncpy(buffer,strlist,bufferSize);
        
        return NULL;
    }
    
    return NULL;
}

unsigned long commenFileSize(const char* fileName)
{
    struct stat file_info;

   if (0  == stat(fileName,&file_info))
   {
       return file_info.st_blocks * 512;
   }
   
   return 0;
}