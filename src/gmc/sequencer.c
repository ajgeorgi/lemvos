
#include <stdlib.h>
#include <string.h>

/* Sequencer for GMC 20.12.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "commen.h"
#include "vertex.h"
#include "sequencer.h"
#include "gmath.h"
#ifdef _DEBUG
#include "modelstore.h"
#endif

#define COLOR_WHITE 0xffffff

static GetIndexedColor _getColor = NULL;

static Calculation *_calculations= NULL;
static int _number_of_calculations = 0;
static int _max_number_of_calculations = 0;

int sequencerInit(GetIndexedColor visualization)
{
    _getColor = visualization;
    
    return 0;
}

int sequencerCheckCalculator(const char* name, CalculationFunction calc)
{
    if (name)
    {
        for (int i = 0; i < _number_of_calculations; i++)
        {
            if (calc == _calculations[i].function)
            {
                return 1;
            }
            if (0 == strncmp(name,_calculations[i].name,sizeof(_calculations[i].name)))
            {
                LOG("Warning calculator name \"%s\" already in use.\n",name);
            }
        }
        
        return 0;
    }
    
    return -1;
}

int sequencerAddCalculation(const char*name, CalculationFunction calc, int color_index)
{
    if (0 == sequencerCheckCalculator(name,calc))
    {
        if (_number_of_calculations >= _max_number_of_calculations)
        {
            _max_number_of_calculations += 10;
            _calculations = memory_realloc(_calculations, sizeof(Calculation)*_max_number_of_calculations);
        }
        
        memset(&_calculations[_number_of_calculations],0,sizeof(Calculation));
        
        _calculations[_number_of_calculations].function = calc;
        _calculations[_number_of_calculations].color_index = color_index;
        strncpy(_calculations[_number_of_calculations].name,name,sizeof(_calculations[_number_of_calculations].name));

        _number_of_calculations++;
    
        return 0;
    }
    
    ERROR("Calculator \"%s\" already assigned.\n",name?name:UNKNOWN_SYMBOL);
    
    return -1;
}

int sequencerAddInitializer(const char*name, CalculationFunction calc, int color_index)
{
    if (0 == sequencerCheckCalculator(name,calc))
    {    
        if (_number_of_calculations >= _max_number_of_calculations)
        {
            _max_number_of_calculations += 10;
            _calculations = memory_realloc(_calculations, sizeof(Calculation)*_max_number_of_calculations);
        }
        
        memset(&_calculations[_number_of_calculations],0,sizeof(Calculation));
        
        _calculations[_number_of_calculations].function = calc;
        _calculations[_number_of_calculations].color_index = color_index;
        strncpy(_calculations[_number_of_calculations].name,name,sizeof(_calculations[_number_of_calculations].name));
        _calculations[_number_of_calculations].flags |= GSEQ_FLAG_INIT;

        _number_of_calculations++;
            
        return 0;
    }
    
    ERROR("Calculator \"%s\" already assigned.\n",name?name:UNKNOWN_SYMBOL);    
    
    return -1;
}

int sequencerAddFinalizer(const char*name, CalculationFunction calc, int color_index)
{
    if (0 == sequencerCheckCalculator(name,calc))
    {        
        if (_number_of_calculations >= _max_number_of_calculations)
        {
            _max_number_of_calculations += 10;
            _calculations = memory_realloc(_calculations, sizeof(Calculation)*_max_number_of_calculations);
        }
        
        memset(&_calculations[_number_of_calculations],0,sizeof(Calculation));
        
        _calculations[_number_of_calculations].function = calc;
        _calculations[_number_of_calculations].color_index = color_index;
        strncpy(_calculations[_number_of_calculations].name,name,sizeof(_calculations[_number_of_calculations].name));
        _calculations[_number_of_calculations].flags |= GSEQ_FLAG_FINAL;

        _number_of_calculations++;
        
        return 0;        
    }
    
    ERROR("Calculator \"%s\" already assigned.\n",name?name:UNKNOWN_SYMBOL);    
    
    return -1;    
}

static int _sequencerRunStep(CObject *object, Calculation *calc)
{
    if (object && calc && calc->function)
    {
        int err = 0;
        if (0 == (calc->flags & GSEQ_FLAG_DISABLE))
        {
            color_type color = COLOR_WHITE;
            int color_index = calc->color_index;
            char s1[GM_VERTEX_BUFFER_SIZE];

            if (0 >= color_index)
            {
                color_index = GAPERAL_MEASUREMENT;
            }
            
            if (object->material)
            {
                if (0 < object->material->color_index)
                {
                    color_index = object->material->color_index;
                }
            }    

            if (_getColor)
            {
                color = _getColor(color_index,object->index);
            }
            
            LOG("Running: \"%s\" on [%s]\n",calc->name,vertexPath((GObject*)object,s1,sizeof(s1)));
            
            CObject *obj = calc->function(calc->name, object, color);
            
            Mea *mea = (Mea *)isCObject(OBJ_MEASUREMENT,obj);
            if (obj)
            {
                if (mea)
                {
                    int results = meaNumberOfValues(mea);

                    LOG("Successfull calculation of %s [%s] created %i results.\n",
                        calc->name,vertexPath((GObject*)object,s1,sizeof(s1)),
                        results);
                    
                    mea->created_by = calc->name;
                }
                else
                {
                    LOG("Successfull finalized/initialized %s [%s].\n",
                        calc->name,vertexPath((GObject*)object,s1,sizeof(s1)));                    
                }
                
                
                return 0;
            }
            else
            {
                ERROR("Failed \"%s\" on [%s]\n",calc->name,vertexPath((GObject*)object,s1,sizeof(s1)));
                return -1;
            }
        }
        
        return err;
    }
    
    return -1;
}

int sequencerRunObject(CObject *object)
{
    int err = 0;
    if (0 == (object->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
    {
        char s1[GM_VERTEX_BUFFER_SIZE];

        for (int i = 0; i < _number_of_calculations; i++)
        {
            if (0 == (_calculations[i].flags & (GSEQ_FLAG_DISABLE|GSEQ_FLAG_INIT)))
            {
                if (_sequencerRunStep(object, &_calculations[i]))
                {
                    ERROR("Failed to run: \"%s\" on \"%s\"\n",_calculations[i].name,vertexPath((GObject*)object,s1,sizeof(s1)));
                    err = 1;
                }
                else
                {
                     object->flags |= GM_GOBJECT_FLAG_CALCULATED;
                }
            }
        }
    }  
    
    return err;
}

int sequencerInitRun(Model *model)
{
    int err = 0;
    if (0 == (model->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        for (CObject *object = isCObject(ANY_COBJECT,model->first); object; object = isCObject(ANY_COBJECT,object->next))
        {   
            if (0 == object->rep_type)
            {
                object->rep_type = model->rep_type | commenGetRepresentation(object->name);
            }
            
            if (0 == (object->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
            {            
                if (object->methods.rotate)
                {
                    LOG("Rotating: \"%s\" [%s]\n",object->name,vertexPath((GObject*)object,s1,sizeof(s1)));
                    
                    if (object->methods.rotate(object, object->box.yaw, object->box.roll, object->box.pitch, object->p[Y_PLANE]))
                    {
                        ERROR1("Rotation failed\n");
                        err = 1;
                    }

                    // We need to recalculate all triangulations and BBox because of more precision
                    object->flags &= ~GM_GOBJECT_FLAG_TRIANGULATED;
                    gmathRemoveAttributes(object);    
                }            

                if (object->methods.triangulate)
                {
                    LOG("Triangulating: [%s]\n",vertexPath((GObject*)object,s1,sizeof(s1)));
                    if (0 == object->methods.triangulate((CObject*)object))
                    {
                        object->flags |= GM_GOBJECT_FLAG_TRIANGULATED;
                    }
                    else
                    {
                        ERROR1("Triangulation failed\n");
                        err = 1;
                    }
                }

                object->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
            }
        }
        
        for (int i = 0; i < _number_of_calculations; i++)
        {
            if (0 == (_calculations[i].flags & (GSEQ_FLAG_DISABLE)))
            {
                if (_calculations[i].flags & GSEQ_FLAG_INIT)
                {
                    if (_sequencerRunStep((CObject*)model, &_calculations[i]))
                    {
                        ERROR("Failed to init run: \"%s\" on \"%s\"\n",_calculations[i].name,vertexPath((GObject*)model,s1,sizeof(s1)));
                        err = 1;
                    }
                }
            }
        }
    }  
    
    return err;    
}

int sequencerFinalRun(Model *model)
{
    int err = 0;
    if (0 == (model->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        
        for (int i = 0; i < _number_of_calculations; i++)
        {
            if (0 == (_calculations[i].flags & (GSEQ_FLAG_DISABLE)))
            {
                if (_calculations[i].flags & GSEQ_FLAG_FINAL)
                {
                    if (_sequencerRunStep((CObject*)model, &_calculations[i]))
                    {
                        ERROR("Failed to final run: \"%s\" on \"%s\"\n",_calculations[i].name,vertexPath((GObject*)model,s1,sizeof(s1)));
                        err = 1;
                    }
                }
            }
        }        
    }
    
    return err;
}

int sequencerRun(Model *model)
{
    if (isObject(model))
    {
        int err = 0;
        if (0 == (model->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
        {        
            model->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
            
            LOG1("Initialize sequence\n");
            if (sequencerInitRun(model))
            {
                ERROR("Initialization failed on model \"%s\"",model->name);
                
                return -1;
            }

            LOG1("Running sequence\n");
            int count = 0;
            for (CObject *object = isCObject(ANY_COBJECT,model->first); object; object = isCObject(ANY_COBJECT,object->next))
            {
                count++;
                
                if (0 == (object->flags & GM_GOBJECT_FLAG_DO_NOT_CALCULATE))
                {
                    err |= sequencerRunObject(object);
                }
            }
            
            LOG1("Finalize sequence\n");
            if (sequencerFinalRun(model))
            {
                ERROR("Finalization failed on model \"%s\"",model->name);
                
                return -1;
            }

            model->flags |= GM_GOBJECT_FLAG_CALCULATED;
            
            LOG("%i objects in \"%s\" and %i calculations available.\n",count, model->name, _number_of_calculations);
        }
            
        return err;        
    }
    
    return -1;
}

int sequencerEnableCalculation(const char*name, int enable)
{
    for (int i = 0; i < _number_of_calculations; i++)
    {
        if (0 == strncmp(name,_calculations[i].name,sizeof(_calculations[i].name)))
        {
            if (enable)
            {
                _calculations[i].flags &= ~GSEQ_FLAG_DISABLE;
            }
            else
            {
                _calculations[i].flags |= GSEQ_FLAG_DISABLE;
            }
        }
    }
    
    return 0;
}

Calculation *sequencerGetCalculation(int index)
{
    if ((0 <= index) && (index < _number_of_calculations))
    {
       return &_calculations[index];
    }
    
    return NULL;
}

