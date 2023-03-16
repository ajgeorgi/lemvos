#ifndef __CONFIGURATION___
#define __CONFIGURATION___

#include <stdio.h>

/* Configuration for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "mea.h"

#define CONFIG_ARRAY_LENGTH_INDEX (-1)

typedef struct ConfigT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
// --------------------------    
    
    char name[32];
    
    enum ConfigType {
        ConfigType_Initial = 0, // This is a string too
        ConfigType_String,
        ConfigType_Double,
        ConfigType_Int,
        ConfigType_ULong,
        ConfigType_DoubleArray
    } ctype;
    
    char *string;
    int array_length;
    MeaUnit  unit;
    
    union {
        double d;
        unsigned long long ui;
        long long i;
        unsigned long ul;
        
        double *ad;
        unsigned long long *aui;
        long long *ai;
        const char** as;        
    } data;
} Config;

#define configGetUnit(_conf) (_conf->unit)

Config *configCreate(const char* name, MeaUnit unit);
void configDestroy(Config *confid);

Config *configGetConfig(const char *name);

int configStoreConfig(Config* config);
int configUpdateConfig(Config* config, char *string);

const char* configString(Config *config, const char* _default);

double configDouble(Config *config, double _default);
int configInt(Config *config, int _default);
unsigned long configULong(Config *config, unsigned long _default);
double configDoubleArray(int index, Config *config, double _default);

void configDump(FILE *file);

double *configStringToDoubleArray(const char *string, int *array_length);



#endif // __CONFIGURATION___