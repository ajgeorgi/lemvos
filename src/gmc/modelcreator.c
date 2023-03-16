#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Modelcreator for GMC 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "configuration.h"
#include "modelcreator.h"
#include "gmath.h"

typedef struct ModelcreatorT {
    const char *name;
    CreateModelT creator;
    InitT init;
    
    struct {
        char* name;
        CalcT calc;
        ProgressT progress;
    } calculator[5];
    int number_of_calculator;
    
    struct ModelcreatorT *next;
} Modelcreator;

static Modelcreator *_modelcreator_first = NULL;
static Modelcreator *_modelcreator_last = NULL;

static ProgressT _modelcreator_progress = NULL;
static const char* _modelcreator_progress_name = NULL;
static char* _modelcreator_calculator[sizeof(_modelcreator_first->calculator)/sizeof(_modelcreator_first->calculator[0]) + 1];

Model *modelcreatorCreate(const char *name)
{
    Model *model = NULL;
    
    if (name)
    {
        for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
        {
            if (0 == strcasecmp(creator->name,name))
            {
                if (creator->creator)
                {
                    if (creator->init)
                    {
                        creator->init(NULL);
                    }
                    
                    model = creator->creator();
                    
                    if (isCObject(OBJ_MODEL,model))
                    {
                        char modelName[MODEL_NAME_SIZE];
                        if (model->name[0])
                        {
                            // put model type and instance name together
                            snprintf(modelName,sizeof(modelName),"%s%c%s",creator->name,COMMEN_TYPE_SEPERATOR,model->name);
                            strncpy(model->name,modelName,sizeof(model->name));
                        }
                        else
                        {
                            strncpy(model->name,creator->name,sizeof(model->name));
                        }

                        for (Solid *solid = vertexSolidIterator(model, NULL, 0);solid;solid = vertexSolidIterator(model, solid, 0))
                        {
                            gmathRemoveDoubles(solid);
                        }
                        
                        break;
                    }
                    else
                    {
                        ERROR("Model creator \"%s\" failed\n",creator->name);
                    }
                }
            }
        }
    }
    
    return model;
}

int modelcreatorAddModel(const char *name, CreateModelT creator, InitT init)
{
    if (name && creator)
    {
        Modelcreator *modelcreator = (Modelcreator *)malloc(sizeof(Modelcreator));
        
        memset(modelcreator,0, sizeof(Modelcreator));
        
        modelcreator->name = strndup(name,MODEL_NAME_SIZE);
        modelcreator->creator = creator;
        modelcreator->init = init;
        
        if (NULL == _modelcreator_first)
        {
            _modelcreator_first = modelcreator;
            _modelcreator_last = modelcreator;
        }
        else
        {
            Modelcreator *prev = _modelcreator_last;
            _modelcreator_last = modelcreator;
            prev->next = modelcreator;
        }
        
        return 0;
    }
    
    return -1;
}

int modelcreatorRunSolidCalculator(const char* model, const char *name, CObject *object)
{
    if (model && name)
    {
        char modelName[MODEL_NAME_SIZE];
        
        strncpy(modelName,model,sizeof(modelName));
        
        char *s = strchr(modelName,COMMEN_TYPE_SEPERATOR);
        if (s)
        {
            *s = 0;
        }
        
        for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
        {
            if (0 == strcasecmp(creator->name,modelName))
            {
                for (int i = 0; i < creator->number_of_calculator; i++)
                {
                    if (creator->calculator[i].name)
                    {
                        if (0 == strcasecmp(creator->calculator[i].name,name))
                        {
                            if (creator->calculator[i].calc)
                            {
                                _modelcreator_progress = creator->calculator[i].progress;
                                _modelcreator_progress_name = creator->calculator[i].name;
                                if (creator->init)
                                {
                                    creator->init(object);
                                }
                                int ret = creator->calculator[i].calc(object);
                                _modelcreator_progress  = NULL;
                                return ret;
                            }
                        }
                    }
                }
            }
        }
    }    
    
    return -1;
}

int modelcreatorAddProgressToCalculator(const char* model, const char *name, ProgressT progress)
{
    if (name && model)
    {
        char modelName[MODEL_NAME_SIZE];
        
        strncpy(modelName,model,sizeof(modelName));
        
        char *s = strchr(modelName,COMMEN_TYPE_SEPERATOR);
        if (s)
        {
            *s = 0;
        }
        
        for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
        {
            if (0 == strcasecmp(creator->name,modelName))
            {
                for (int i = 0; i < creator->number_of_calculator; i++)
                {
                    if (creator->calculator[i].name)
                    {
                        if (0 == strcasecmp(creator->calculator[i].name,name))
                        {                
                            LOG("Adding progress %s%c%s\n",modelName,COMMEN_TYPE_SEPERATOR,name);
                            creator->calculator[i].progress = progress;
                            return 0;
                        }
                    }
                }
            }
        }
    }
    
    return -1;    
}

int modelcreatorAddSolidCalculator(const char* model, const char *name, CalcT calculator)
{
    if (name && model)
    {
        char modelName[MODEL_NAME_SIZE];
        
        strncpy(modelName,model,sizeof(modelName));
        
        char *s = strchr(modelName,COMMEN_TYPE_SEPERATOR);
        if (s)
        {
            *s = 0;
        }
        
        for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
        {
            if (0 == strcasecmp(creator->name,modelName))
            {
                if (creator->number_of_calculator < (int)(sizeof(creator->calculator)/sizeof(creator->calculator[0])))
                {
                    for (int i = 0; i < creator->number_of_calculator; i++)
                    {
                        if (creator->calculator[i].name)
                        {
                            if (0 == strcasecmp(creator->calculator[i].name,name))
                            {
                                // Already installed
                                return 0;
                            }
                        }
                    }
                    
                    LOG("Adding solid calculator %s%c%s\n",modelName,COMMEN_TYPE_SEPERATOR,name);
                    creator->calculator[creator->number_of_calculator].name = strdup(name);
                    creator->calculator[creator->number_of_calculator].calc = calculator;
                    creator->number_of_calculator++;                    
                    return 0;
                }
            }
        }
    }
    
    return -1;
}

char** modelcreatorGetSolidCalculators(const char* model, int *length)
{
    int index = 0;
    memset(_modelcreator_calculator,0,sizeof(_modelcreator_calculator));
    
    char modelName[MODEL_NAME_SIZE];

    strncpy(modelName,model,sizeof(modelName));

    char *s = strchr(modelName,COMMEN_TYPE_SEPERATOR);
    if (s)
    {
        *s = 0;
    }

    
    for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
    {
        if (0 == strcasecmp(creator->name,modelName))
        {
            if (creator->init)
            {
                creator->init(NULL);
            }

            for (int i = 0; i < creator->number_of_calculator; i++)
            {
                if (creator->calculator[i].name)
                {
                    _modelcreator_calculator[index++] = creator->calculator[i].name;
                }
            }
            
            if (length)
            {
                *length = creator->number_of_calculator;
            }
            
            break;
        }
    }
    
    return _modelcreator_calculator;
}

void modelcreatorProgress(int max, int pos)
{
    if (_modelcreator_progress)
    {
        _modelcreator_progress(_modelcreator_progress_name,max,pos);
    }
}

int modelcreatorInitModel(Model *model)
{
    char modelName[MODEL_NAME_SIZE];

    strncpy(modelName,model->name,sizeof(modelName));

    char *s = strchr(modelName,COMMEN_TYPE_SEPERATOR);
    if (s)
    {
        *s = 0;
    }
    
    for (Modelcreator *creator = _modelcreator_first; creator; creator = creator->next)
    {
        if (0 == strcasecmp(creator->name,modelName))
        {
            if (creator->init)
            {
                int err = creator->init((CObject*)model);
                
                if (0 == err)
                {
                    model->rep_type = commenGetRepresentation(model->name);
                }
                
                return err;
            }
        }
    }
    
    return -1;
}
