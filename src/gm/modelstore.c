#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* modelstore for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "gobject.h"
#include "mea.h"
#include "modelstore.h"


static Model *_modelstore_first = NULL;
static Model *_modelstore_last = NULL;

static Material* _material_first = NULL;
static Material* _material_last = NULL;

static unsigned int _modelstore_vertex_count = 0;


void initModelstore()
{
    _modelstore_vertex_count = 0;    
}

unsigned int modelstoreGetVertexCount()
{
    return _modelstore_vertex_count;
}

Model *modelstoreGetModel(const char* name)
{
    if (name)
    {
        for (Model *model = _modelstore_first; isObject((GObject*)model); model = (Model*)model->next)
        {
            if (0 == strncmp(model->name,name,sizeof(model->name)))
            {
                return model;
            }
        }
    }
    return NULL;
}

Model *modelstoreCreate(const char *name, const char *owner)
{
    if (name && owner)
    {
        Model *model = createModel();
        
        if (model)
        {
            model->rep_type =  commenGetRepresentation(name);
            
            strncpy(model->name,name,sizeof(model->name));
            strncpy(model->owner,owner,sizeof(model->owner));
            
            if (NULL == _modelstore_first)
            {
                _modelstore_first = model;
                _modelstore_last = model;
            }
            else
            {
                Model *prev = _modelstore_last;
                _modelstore_last = model;
                prev->next = (GObject*)model;
            }
            
            LOG("Model \"%s\" created\n",name);
            
            return model;
        }
    }
    
    return NULL;
}

Solid *modelstoreCreateSolid(Model *model, const char* name)
{
    if (isCObject(OBJ_MODEL,model) && name)
    {
        Solid *solid = createSolid((GObject*)model);
        
        strncpy(solid->name,name,sizeof(solid->name));
        
        gobjectClearBoundingBox(&solid->box);
        gobjectClearBoundingBox(&solid->original_box);

        solid->rep_type =  model->rep_type | commenGetRepresentation(name);
        
        if (NULL == model->last)
        {
            model->first = (CObject*)solid;
            model->last = (CObject*)solid;
        }
        else
        {
            CObject *prev = model->last;
    
            model->last = (CObject*)solid;
            prev->next = (GObject*)solid;
            solid->refCount++;
        }
        
        return solid;
    }
    
    return NULL;
}

int modelstoreMoveSolidToBottom(Solid *solid)
{
    if (isCObject(OBJ_SOLID,solid) && solid->parent)
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,solid->parent);
        if (model)
        {
            CObject *prev = (CObject*)isCObject(ANY_COBJECT,model->first);
            for (CObject *object = model->first; NULL != object; object = isCObject(ANY_COBJECT,object->next))
            {     
                Solid *any_solid = (Solid*)isCObject(OBJ_SOLID,object);
                if (any_solid)
                {
                    if (solid == any_solid)
                    {
                        if (model->last == (CObject*)solid)
                        {
                            model->last = prev;
                        }
                        
                        prev->next = solid->next;
                        solid->next = (GObject*)model->first;
                        model->first = (CObject*)solid;
                        
                        return 0;
                    }
                }
                
                prev = object;
            }
            
        }
    } 
    
    return -1;
}

Mesh *modelstoreCreateMesh(Model *model, const char* name)
{
    if (isCObject(OBJ_MODEL,(GObject*)model) && name)
    {
        Mesh *mesh = createMesh((GObject*)model);

        strncpy(mesh->name,name,sizeof(mesh->name));
        
        gobjectClearBoundingBox(&mesh->box);
 
        mesh->rep_type =  model->rep_type | commenGetRepresentation(name);
        
        if (NULL == model->last)
        {
            model->first = (CObject*)mesh;
            model->last = (CObject*)mesh;
        }
        else
        {
            CObject *prev = model->last;
    
            model->last = (CObject*)mesh;
            prev->next = (GObject*)mesh;
            mesh->refCount++;
        }
        
        return mesh;
    }
    
    return NULL;
}

Mesh *modelstoreGetMesh(Model *model, const char* name)
{
    for (CObject *object = model->first; NULL != object; object = isCObject(ANY_COBJECT,object->next))
    {     
        Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
        if (mesh)
        {
            if (0 == strncasecmp(name,mesh->name,sizeof(mesh->name)))
            {
                return mesh;
            }
        }
    }
    
    return NULL;    
}


Material *modelstoreCreateMaterial(CObject *object, const char *name)
{
    if (name)
    {
        Material *material = memory_aloc(sizeof(Material));
        memset(material,0,sizeof(Material));
        
        objectInit((GObject*)material, (GObject*)object, 0, OBJ_MATERIAL);

        strncpy(material->name,name,sizeof(material->name));
        
        if (object)
        {
            vertexReplaceMaterial(object,material);
        }        
    
        if (NULL == _material_first)
        {
            _material_first = material;
            _material_last = material;            
        }
        else
        {
            Material *prev = _material_last;
            _material_last = material;                        
            prev->next = (GObject*)material;
        }
        material->refCount++;
        
#ifdef _DEBUG_CONFIG        
        LOG("Creating material %s\n",name);        
#endif  
        
        return material;
    }
    
    return NULL;
}

Mea *modelstoreCreateMeasurement(CObject *parent, const char *name)
{
    if (isCObject(ANY_COBJECT,parent) && name)
    {
        Mea *measurement = createMesurement((GObject*)parent);
        
        strncpy(measurement->name,name,sizeof(measurement->name));
        
        measurement->rep_type +=  commenGetRepresentation(name);

        if (NULL == parent->last_mea)
        {
            parent->first_mea = (CObject*)measurement;
            parent->last_mea = (CObject*)measurement;            
        }
        else
        {
            Mea *prev = (Mea*)parent->last_mea;
            parent->last_mea = (CObject*)measurement;                        
            prev->next = (GObject*)measurement;
        }
        measurement->refCount++;
        
        return measurement;
    }
    
    return NULL;    
}

Polygon *modelstoreCreatePolygon(Solid *solid)
{
    if (isObject((GObject*)solid))
    {
        Polygon *poly = memory_aloc(sizeof(Polygon));
        memset(poly,0,sizeof(Polygon));
        
        objectInit((GObject*)poly, (GObject*)solid, 0, OBJ_POLYGON);
                
        // Create just absolute polygones by now ???
        poly->flags |= GM_GOBJECT_FLAG_POLY_ABSOLUT;   
        
        if (NULL == solid->last)
        {
            solid->first = poly;
            solid->last = poly;
        }
        else
        {
            GObject *prev = (GObject*)solid->last;            
            solid->last = poly;            
            prev->next = (GObject*)poly;
        }
        return poly;    
    }
    
    return NULL;
}

Polygon *modelstoreCreateOriginalPolygon(Solid *solid)
{
    if (isObject((GObject*)solid))
    {
        Polygon *poly = memory_aloc(sizeof(Polygon));
        memset(poly,0,sizeof(Polygon));
        
        objectInit((GObject*)poly, (GObject*)solid, 0, OBJ_POLYGON);
                
        // Create just absolute polygones by now ???
        poly->flags |= (GM_GOBJECT_FLAG_POLY_ABSOLUT|GM_GOBJECT_FLAG_POLY_ORIGINAL);   
        
        if (NULL == solid->last_original)
        {
            solid->first_original = poly;
            solid->last_original = poly;
        }
        else
        {
            GObject *prev = (GObject*)solid->last_original;            
            solid->last_original = poly;            
            prev->next = (GObject*)poly;
        }
        return poly;    
    }
    
    return NULL;    
}

const Vertex *modelstoreAddToPolygonOriginal(Polygon *polygon, EPoint *v, flag_type flags, IndexType index)
{
    if (isObject((GObject*)polygon) && v)
    {
        if (polygon->flags & GM_GOBJECT_FLAG_POLY_ORIGINAL)
        {
            Vertex *vertex = createVertex((GObject*)polygon,v);
            
            if (polygon->parent)
            {
                CObject *cobj = isCObject(ANY_COBJECT,polygon->parent);
                if (cobj)
                {
                    if (!gobjectIsBoundingBoxCalculated(&cobj->box))
                    {                    
                        if (vertexAddToBoundingBox(cobj,vertex))
                        {
                            char s1[GM_VERTEX_BUFFER_SIZE];
                            ERROR("Failed to add [%s] to BB of \"%s\"",
                                vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG),
                                cobj->name
                                );
                        }
                    }
                }
                
                Solid *solid = (Solid*)isCObject(OBJ_SOLID,polygon->parent);
                if (solid)
                {
                    if (!gobjectIsBoundingBoxCalculated(&solid->original_box))
                    {                                        
                        if (vertexAddToBox(&solid->original_box,vertex))
                        {
                            char s1[GM_VERTEX_BUFFER_SIZE];
                            ERROR("Failed to add [%s] to BB of \"%s\"",
                                vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG),
                                cobj->name
                                );
                        }                    
                    }
                }
            }
            
            vertex->flags = flags | GM_GOBJECT_FLAG_POLY_ORIGINAL;
            vertex->index = index;
                         
            if (NULL == polygon->last)
            {
                polygon->first = vertex;
                polygon->last = vertex;
            }
            else
            {
                Vertex *prev = polygon->last;
                polygon->last = vertex;            
                prev->next = (GObject*)vertex;                       
            }
            
            vertex->refCount++;
            
            _modelstore_vertex_count++;
            
            return vertex;
        }
        
        char s1[GM_VERTEX_BUFFER_SIZE],s2[GM_VERTEX_BUFFER_SIZE];
        ERROR("Polygone [%s] not original! Can not add [%s]\n",
              vertexPath((GObject*)polygon,s1,sizeof(s1)),
              EPoint3ToString(v,s2,sizeof(s2)));
    }
    
    return NULL;
}

Vertex *modelstoreAddToPolygon(Polygon *polygon, EPoint *v)
{
    if (isObject((GObject*)polygon) && v)
    {
        Vertex *vertex = createVertex((GObject*)polygon,v);
        
        if (polygon->parent)
        {
            CObject *cobj = isCObject(ANY_COBJECT,polygon->parent);
            if (cobj)
            {
                if (!gobjectIsBoundingBoxCalculated(&cobj->box))
                {
                    if (vertexAddToBoundingBox(cobj,vertex))
                    {
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        ERROR("Failed to add [%s] to \"%s\"",
                                vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG),
                                cobj->name
                                );
                    }
                }
            }
        }        
                        
        if (NULL == polygon->last)
        {
            polygon->first = vertex;
            polygon->last = vertex;
        }
        else
        {
            Vertex *prev = polygon->last;
            polygon->last = vertex;            
            prev->next = (GObject*)vertex;                       
        }
        
        vertex->refCount++;
        
        return vertex;
    }
    
    return NULL;
}

Vertex *modelstoreInsertToPolygon(Polygon *polygon, EPoint *v)
{
    if (isObject((GObject*)polygon) && v)
    {
        Vertex *vertex = createVertex((GObject*)polygon,v);
        
        if (polygon->parent)
        {
            CObject *cobj = isCObject(ANY_COBJECT,polygon->parent);
            if (cobj)
            {
                if (!gobjectIsBoundingBoxCalculated(&cobj->box))
                {
                    if (vertexAddToBoundingBox(cobj,vertex))
                    {
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        ERROR("Failed to add [%s] to \"%s\"",
                                vertexToString(vertex,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG),
                                cobj->name
                                );
                    }
                }
            }
        }
                            
        if (NULL == polygon->last)
        {
            polygon->first = vertex;
            polygon->last = vertex;
        }
        else
        {            
            vertex->next = (GObject*)polygon->first;
            polygon->first = vertex;            
        }

        // ??? Indices resort?
        
        vertex->refCount++;
        
        return vertex;
    }
    
    return NULL;
}


Material *modelstoreGetMaterial(const char*name)
{
    if (name)
    {
        for (Material *material = _material_first; NULL != material; material = (Material*)material->next)
        {
            if (0 == strncasecmp(name,material->name,sizeof(material->name)))
            {
                return material;
            }
        }
       
        ERROR("No material \"%s\" found!\n",name?name:UNKNOWN_SYMBOL);
        
        return NULL;
    }
    
    return _material_first;
}


Solid *modelstoreGetSolid(Model *model, const char* name)
{
    for (CObject *object = model->first; NULL != object; object = isCObject(ANY_COBJECT,object->next))
    {     
        Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
        if (solid)
        {
            if (0 == strncasecmp(name,solid->name,sizeof(solid->name)))
            {
                return solid;
            }
        }
    }
    
    return NULL;
    
}

void modelstoreClear()
{
    LOG("Clearing: vertex count=%i\n",
        _modelstore_vertex_count);

        
    for (Model *model = _modelstore_first; NULL != isObject((GObject*)model);)
    {
        for (CObject *object = isCObject(ANY_COBJECT,model->first); NULL != isCObject(ANY_COBJECT,object); )
        {    
            Solid *solid = (Solid*)isCObject(OBJ_SOLID,object);
            Mesh *mesh = (Mesh*)isCObject(OBJ_MESH,object);
            if (solid)
            {                
                CObject *s = isCObject(ANY_COBJECT,solid->next);
                solidDestroy(solid);
                object = s;
            }
            else
            if (mesh)
            {
                CObject *s = isCObject(ANY_COBJECT,mesh->next);                
                meshDestroy(mesh);
                object = s;
            }
            else
            {
                if (object->methods.destroy)
                {
                    object->methods.destroy(object);
                }
            }
        }
        
        Model *m = (Model*)model->next;
        memory_free(model);
        model = m;
    }
    
    _modelstore_first = NULL;
    _modelstore_last = NULL;
 
    for (Material* material = _material_first; NULL != material; )
    {
        Material *m = (Material*)isObject(material->next);
        vertexDestroyMaterial(material);
        material = m;
    }
 
    _modelstore_vertex_count = 0;
    
    _material_first = NULL;
    _material_last = NULL;

#ifdef _DEBUG  
    memory_check();
#endif    
    
    initModelstore();
    
    // ???: Material and Solid should be cleared here too
}

Vertex *modelstoreGetVertex(Polygon *polygon, IndexType index)
{
    if (index)
    {
        for (Vertex *vertex = polygon->first; NULL != vertex; vertex = (Vertex*)vertex->next)
        {
            if (index == vertex->index)
            {
                return vertex;
            }
        }
    }    
    
    return NULL;
}

Polygon *modelstoreGetPolygon(Solid *solid, IndexType index)
{
    if (index)
    {
        for (Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
        {
            if (index == poly->index)
            {
                return poly;
            }
        }
    }    
    
    return NULL;    
}


const char* modelstoreReport(const Model *model, char *buffer, size_t size)
{
    if (isCObject(OBJ_MODEL,model))
    {
        double width,  length,  height;
        vertexGetCObjectSize((const CObject *)model, &width, &length, &height);

        int count = 0;
        int vcount = 0;
        int ovcount = 0;

        CIter modelIter = gobjectCreateConstIterator(model,0);
        for (const CObject *object = (const CObject *)gobjectConstIterate(modelIter);
                object;
                object = (const CObject *)gobjectConstIterate(modelIter))
        {
            count++;
            CIter iter = gobjectCreateConstIterator(object,0);            
            for (const Vertex *vertex = (const Vertex *)gobjectConstIterate(iter);
                    vertex;
                    vertex = (const Vertex *)gobjectConstIterate(iter))
            {
                vcount++;
            }
                
            const Solid *solid = (const Solid *)isCObject(OBJ_SOLID,object);
            if (solid)
            {
                for (const Vertex *vertex = vertexOriginalIterator(NULL, solid, GM_GOBJECT_FLAG_POLY_ORIGINAL);
                        vertex;
                        vertex = vertexOriginalIterator(vertex, solid, 0))
                    {
                        ovcount++;
                    }
            }                        
        }
        
        
        snprintf(buffer,size,"Model \"%s\" size=(%g,%g,%g)\n#obj=%i, #vertices=%i, #original=%i.\n", 
            model->name,
            width, length, height,
            count,vcount,ovcount);
    }
    
    return buffer;
}    