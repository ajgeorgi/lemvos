#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Configuration for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "configuration.h"
#include "loader.h"
#include "commen.h"

#define MAXIMUM_STRING_LENGTH 512

static Config* _configuration_first = NULL;
static Config* _configuration_last = NULL;
static int _top_level_config_index = 1;

Config *configCreate(const char* name, MeaUnit unit)
{
    Config* config = (Config*) memory_aloc(sizeof(Config));
    
    memset(config,0,sizeof(Config));
    objectInit((GObject *)config, NULL, _top_level_config_index++, OBJ_CONFIG);
    
    strncpy(config->name,name,sizeof(config->name));
    config->ctype = ConfigType_Initial;
    config->unit = unit;
    
#ifdef _DEBUG_MEM    
    memory_check();
#endif    
    
    return config;
}

void configDestroy(Config *config)
{
    if (isGObject(OBJ_CONFIG,config))
    {
        config->magic = 0;
        if (config->string)
        {
            memory_free(config->string);
        }
        
        if (config->ctype == ConfigType_DoubleArray)
        {
            if (config->data.ad)
            {
                memory_free(config->data.ad);
            }
        }
        
        memory_free(config);
    }
}


Config *configGetConfig(const char *name)
{
    for (Config *config = _configuration_first; NULL != config; config = (Config *)config->next)
    {
        if (!isGObject(OBJ_CONFIG,(GObject*)config))
        {
            FATAL1("Corrupt configuration\n");
            return NULL;
        }
        
        if (0 == strcasecmp(config->name,name))
        {
            return config;
        }
    }
    
    LOG("Warning: Could not find config for \"%s\".\n",name?name:"<null>");
    
    return NULL;
}

int configUpdateConfig(Config* config, char *string)
{
    if (config)
    {
        enum ConfigType ctype = config->ctype;
        config->ctype = ConfigType_Initial;
        if (config->string)
        {
            memory_free(config->string);
        }
        config->string = memory_strndup(string,MAXIMUM_STRING_LENGTH);
        
        switch(ctype)
        {
           case ConfigType_String:
               configString(config,NULL);
           break;
           case ConfigType_Double:
               configDouble(config,0);
               break;
            case ConfigType_Int:
                configInt(config,0);
                break;
            case ConfigType_ULong:
                configULong(config,0);
                break;
            case ConfigType_DoubleArray:
                configDoubleArray(0,config,0);
                break;
            default:
                ;
        }
        
        return 0;
    }
    
    return -1;
}

int configStoreConfig(Config* config)
{
    Config *found = NULL;
    Config *prev = NULL;
    
    if (isGObject(OBJ_CONFIG,config))
    {
        if (NULL == _configuration_last) 
        {
            _configuration_first = config;
            _configuration_last = config;
        }
        else
        {
            for (Config *_config = _configuration_first; _config; _config = (Config *)_config->next)
            {
                if (!isGObject(OBJ_CONFIG,(GObject*)_config))
                {
                    FATAL1("Corrupt configuration\n");
                    return -1;
                }
                if (0 == strcasecmp(_config->name,config->name))
                {
                    found = _config;
                    break;
                }
                prev = _config;
            }        
        }

        if (found)
        {
            LOG("Warning: Updating config \"%s\"\n",config->name);
            if (found != config)
            {
                found->ctype = ConfigType_Initial;
                if (found->string)
                {
                    memory_free(found->string);
                    found->string = NULL;
                }
                if (config->ctype == ConfigType_DoubleArray)
                {                
                    if (found->data.ad)
                    {
                        memory_free(found->data.ad);
                        found->data.ad = NULL;
                    }
                }
                found->string = config->string;
                config->string = NULL;
                
                configDestroy(config);
                config = NULL;
#ifdef _DEBUG_CONFIG                
                config = found; // For logging only
#endif                
            }
        }
        else
        {
            prev = _configuration_last;
            _configuration_last= config;
            if (prev)
            {
                prev->next = (GObject*)config;
            }        
            config->next = NULL;
        }
        
    #ifdef _DEBUG_CONFIG    
        LOG("Stored: %s = %s\n",config->name, config->string);
    #endif
        
        return 0;
    }
    
    return -1;
}

const char* configString(Config *config, const char* _default)
{
    if (config && config->string)
    {
        int len = strnlen(config->string,MAXIMUM_STRING_LENGTH);
        if ((0<len) && ('"' == config->string[len-1]))
        {
            config->string[len-1] = 0;
        }           
        
        if ('"' == config->string[0])
        {
            return &config->string[1];
        }
        
        return config->string;
    }
    
    return _default;
}

double configStringToDouble(const char* string)
{
    return commenStringToDouble(string);
}

double configDouble(Config *config, double _default)
{
    if (config)
    {
        switch(config->ctype)
        {
            case ConfigType_Initial: {
                config->ctype = ConfigType_Double;
                config->data.d = configStringToDouble(commenStripString(config->string));
                return config->data.d;
            }
            
            case ConfigType_Double: {
                return config->data.d;
            }
            default:
                ERROR("Error: failed to convert \"%s\" to double.\n",config->name);
        }
    }
    
    return _default;
}

double *configStringToDoubleArray(const char *string, int *array_length)
{
    char numBuffer[30];
    char *num = numBuffer;
    int alen = 10;
    int aidx = 0;
    
    *num = 0;
    double *array = memory_aloc(sizeof(double)*alen);
    
    for (const char *i = string; *i; i++)
    {
        if (',' == *i)
        {            
            *num = 0;
            if (aidx >= alen)
            {
                alen += 10;
                array = memory_realloc(array,sizeof(double)*alen);
            }
            array[aidx] = configStringToDouble(commenStripString(numBuffer));
            aidx++;
            num = numBuffer;
            *num = 0;
        }
        else
        {
            *num = *i;
            num++;
            *num = 0;
        }
    }

    if (numBuffer[0])
    {
        *num = 0;
        if (aidx >= alen)
        {
            alen += 10;
            array = memory_realloc(array,sizeof(double)*alen);
        }
        array[aidx] = configStringToDouble(commenStripString(numBuffer));        
        aidx++;
    }
    
    if (array_length)
    {
        *array_length = aidx;
    }
    
    return array;
}


double configDoubleArray(int index, Config *config, double _default)
{
    if (config)
    {        
        switch(config->ctype)
        {
            case ConfigType_Initial: {
                if (config->data.ad)
                {
                    memory_free(config->data.ad);
                }
                
                config->data.ad = configStringToDoubleArray(config->string, &config->array_length);
                if (config->data.ad)
                {
                    config->ctype = ConfigType_DoubleArray;
                    if ((0<=index) && (index < config->array_length))
                    {
                        return config->data.ad[index];
                    }
                }
                break;
            }
            
            case ConfigType_DoubleArray: {
                if ((0<=index) && (index < config->array_length))
                {
                    if (config->data.ad)
                    {
                        return config->data.ad[index];
                    }
                }
                break;
            }
            default:
                ERROR("Error: failed to convert \"%s\" to double.\n",config->name);
        }
        
        if (CONFIG_ARRAY_LENGTH_INDEX == index)
        {
            return (double) config->array_length;
        }        
    }
        
    return _default;
}

unsigned long configULong(Config *config, unsigned long _default)
{
    if (config)
    {
        switch(config->ctype)
        {
            case ConfigType_Initial: {
                config->ctype = ConfigType_ULong;
                config->data.ul = commenStringToULong(commenStripString(config->string));
                return config->data.ul;
            }
            
            case ConfigType_ULong: {
                return config->data.ul;
            }
            default:
                LOG("Error: failed to convert \"%s\" to double.\n",config->name);
        }
    }
    
    return _default;
}

int configInt(Config *config, int _default)
{
    if (config)
    {
        switch(config->ctype)
        {
            case ConfigType_Initial: {
                config->ctype = ConfigType_Int;
                config->data.i = (int)commenStringToLong(commenStripString(config->string));
                return config->data.i;
            }
            
            case ConfigType_Int: {
                return config->data.i;
            }
            default:
                LOG("Error: failed to convert \"%s\" to int.\n",config->name);
        }
    }
    
    return _default;
}

void configDump(FILE *file)
{
    for (Config *config = _configuration_first; NULL != config; config = (Config*)config->next)
    {
        fprintf(file,"%s = %s\n",config->name,configString(config,"?"));
    }
}

