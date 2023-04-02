
#include <stdio.h>
#include <string.h>

#define __USE_GNU
#include <math.h>
#undef __USE_GNU

/* Vertex for GM 28.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/
#define __GOBJECT_PRIVATE
#include "gobject.h"
#undef __GOBJECT_PRIVATE

#include "vertex.h"
#include "commen.h"
#include "iterator.h"

#define VERTEX_EPSILON 0.0000001

static int _vertexRotateVertex(const EPointL *rotC, const EPointL *rotS, const EPoint *v1, EPoint *v2);

#ifdef _DEBUG
#define CHECKVERTEX(_v) _checkVertex(__FUNCTION__,__LINE__,_v)
#else
#define CHECKVERTEX(_v)
#endif

#ifdef _DEBUG
static int _checkVertex(const char* str, int line, const Vertex *vertex)
{
    if (vertex)
    {
        int err = 0;
        if (isGObject(OBJ_VERTEX,vertex))
        {            
            if (vertex->refCount <= 0)
            {
                // We accept staic memory here. Because they rest in unused arrays
                if (0 == (vertex->type & _SOBJ_MARKER))
                {
                    char vstr[GM_VERTEX_BUFFER_SIZE];
                    ERROR("%s:%i:Vertex %s with no reference in Solid\n",                        
                            str?str:UNKNOWN_SYMBOL,line,
                            vertexToString(vertex,vstr,sizeof(vstr),GM_V2S_FLAGS_DEBUG)
                        );
                    
                    err = -1;
                }
            }    
        }
        else
        {
            FATAL("%s:%i:Invalid vertex in solid \n",str?str:UNKNOWN_SYMBOL,line);
            err = -1;
        }
        
        return err;
    }
    
    return 0;
}
#endif

Vertex *createVertex(GObject *parent, EPoint *x)
{
    if (isObject((GObject*)parent) && x)
    {    
        Vertex *vertex = memory_aloc(sizeof(Vertex));
        memset(vertex,0,sizeof(Vertex));
        
        objectInit((GObject*)vertex, (GObject*)parent,/* index */  0, OBJ_VERTEX);

        CopyEPoint(vertex->x,x);
        
        return vertex;
    }
    
    return NULL;
}


Triangle *createTriangle(GObject *parent, EPoint *x1, EPoint *x2, EPoint *x3)
{
    if (isObject((GObject*)parent))
    {    
        Triangle *tri = memory_aloc(sizeof(Triangle));
        memset(tri,0,sizeof(Triangle));
        
        objectInit((GObject*)tri, (GObject*)parent,/* index */  0, OBJ_TRIANGLE);

        if (x1)
        {
            CopyEPoint(tri->v[0].x,x1);
        }
        
        if (x2)
        {
            CopyEPoint(tri->v[1].x,x2);
        }
        
        if (x3)
        {
            CopyEPoint(tri->v[2].x,x3);
        }
        
        return tri;
    }
    
    return NULL;
}

void vertexDestroyTriangle(Triangle *tri)
{
    if (isGObject(OBJ_TRIANGLE,tri))
    {
        char s1[GM_VERTEX_BUFFER_SIZE];
        vertexPath((GObject*)tri,s1,sizeof(s1));

        for (int i = 0; i < (int)(sizeof(tri->v)/sizeof(tri->v[0]));i++)
        {
            // This is a special case and vertexDestroy() can handle it :-)!
            vertexDestroy(&tri->v[i]);
        }
        
        
        tri->magic = 0;
        if (0 == (_SOBJ_MARKER & tri->type))
        {     
#ifdef _DEBUG_DELETING                                
            LOG("Deleting [%s] for real\n",s1);
#endif                
            
            memory_free(tri);
        }
    }
}

int vertexCloneVertex(Vertex *v, const Vertex *v1)
{
    if (v && v1)
    {
        const int len = ((const char*)&v->x)-((const char*)v);
        
        memcpy(&v->x,&v1->x,len);
        
        AreaAttributes *attrib = v1->first_attrib;
        while(attrib)
        {
            if (attrib->flags & GM_ATTRIB_FLAG_VALID)
            {
                vertexCloneAttribute(v, attrib);
            }
            attrib = (AreaAttributes*)attrib->next;
        }

        // Check what when v->connections exists???
        if (NULL == v->connections)
        {
            v->connections = memory_aloc(sizeof(v1->connections[0]) * v1->max_number_of_connections);
        }
        
        memcpy(v->connections,v1->connections,sizeof(v1->connections[0]) * v1->number_of_connections);
        v->number_of_connections = v1->number_of_connections;
        v->max_number_of_connections = v1->max_number_of_connections;
    }
    
    return 0;
}

int vertexIsNull(const EPoint *x, EPointL epsilon)
{        
    return vertexCompareEPoint(x[0],0,epsilon) &&
            vertexCompareEPoint(x[1],0,epsilon) &&
            vertexCompareEPoint(x[2],0,epsilon);
}

EPoint vertexBoxDistance(const EPoint *x1, const EPoint *x2)
{
    return ABS(x1[0]-x2[0]) +
            ABS(x1[1]-x2[1]) +
            ABS(x1[2]-x2[2]);
}

EPoint vertexSphereDistance(const EPoint *x1, const EPoint *x2)
{
    EPoint diff[3];
        
    DiffEPoints(diff,x1,x2);
    
    const double d = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
    
    return sqrt(d);
}

EPointL vertexNorm(const EPoint *x)
{
   long double la10 = x[0];
   long double la11 = x[1];
   long double la12 = x[2];
        
   long double n1 = la10*la10;
   long double n2 = la11*la11;
   long double n3 = la12*la12;

   long double sum = sqrtl(n1 + n2 + n3);
    
   return sum;
}

int vertexNormalize(EPoint *n, const EPoint *x)
{
   long double la10 = x[0];
   long double la11 = x[1];
   long double la12 = x[2];
        
   long double n1 = la10*la10;
   long double n2 = la11*la11;
   long double n3 = la12*la12;

   long double sum = sqrtl(n1 + n2 + n3);
   
   if (sum > 0.0000000001)
   {
      EPointL n1[3];
      long double sum1 = 1.0l/sum;
      EPointExtend(n1,sum1,x);
      CopyEPoint(n,n1);
      
      return 0;
   }

   ERROR1("Failed to normalize\n");
   
   return -1;
}

EPointL vertexNormL(const EPointL *x)
{
   long double la10 = x[0];
   long double la11 = x[1];
   long double la12 = x[2];
        
   long double n1 = la10*la10;
   long double n2 = la11*la11;
   long double n3 = la12*la12;

   long double sum = sqrtl(n1 + n2 + n3);
    
   return sum;    
}

int vertexNormalizeL(EPointL *n, const EPointL *x)
{
   long double la10 = x[0];
   long double la11 = x[1];
   long double la12 = x[2];
        
   long double n1 = la10*la10;
   long double n2 = la11*la11;
   long double n3 = la12*la12;

   long double sum = sqrtl(n1 + n2 + n3);
   
   if (sum > 0.0000000001)
   {
      EPointL n1[3];
      long double sum1 = 1.0l/sum;
      EPointExtend(n1,sum1,x);
      CopyEPoint(n,n1);
      
      return 0;
   }

   ERROR1("Failed to normalize\n");
   
   return -1;    
}

EPoint* vertexVectorProduct(EPoint *r, const EPoint *x1, const EPoint *x2)
{
   long double la10 = x1[0];
   long double la11 = x1[1];
   long double la12 = x1[2];

   long double lb20 = x2[0];
   long double lb21 = x2[1];
   long double lb22 = x2[2];
   
   r[0] = la11*lb22 - la12*lb21;
   r[1] = la12*lb20 - la10*lb22;
   r[2] = la10*lb21 - la11*lb20;
    
   return r;
}

EPointL vertexScalarProduct(const EPoint *x1, const EPoint *x2)
{
   if (x1 && x2)
   {
        long double la10 = x1[0];
        long double la11 = x1[1];
        long double la12 = x1[2];

        long double lb20 = x2[0];
        long double lb21 = x2[1];
        long double lb22 = x2[2];
            
        long double r = la10*lb20 + la11*lb21 + la12*lb22;

        return r;
   }
   
   return 0;
}

EPointL* vertexVectorProductL(EPointL *r, const EPointL *x1, const EPointL *x2)
{
   long double la10 = x1[0];
   long double la11 = x1[1];
   long double la12 = x1[2];

   long double lb20 = x2[0];
   long double lb21 = x2[1];
   long double lb22 = x2[2];
   
   r[0] = la11*lb22 - la12*lb21;
   r[1] = la12*lb20 - la10*lb22;
   r[2] = la10*lb21 - la11*lb20;
    
   return r;    
}

EPointL vertexScalarProductL(const EPointL *x1, const EPointL *x2)
{
   long double la10 = x1[0];
   long double la11 = x1[1];
   long double la12 = x1[2];

   long double lb20 = x2[0];
   long double lb21 = x2[1];
   long double lb22 = x2[2];
    
   long double r = la10*lb20 + la11*lb21 + la12*lb22;

   return r;    
}


EPoint* vertexVectorDiff(EPoint *r, const EPoint *x1, const EPoint *x2)
{
    r[0] = x1[0] - x2[0];
    r[1] = x1[1] - x2[1];
    r[2] = x1[2] - x2[2];
    
    return r;    
}

EPoint* vertexVectorAdd(EPoint *r, const EPoint *x1, const EPoint *x2)
{
    r[0] = x1[0] + x2[0];
    r[1] = x1[1] + x2[1];
    r[2] = x1[2] + x2[2];
    
    return r;    
}

EPoint* vertexVectorCopy(EPoint *r, const EPoint *x1)
{
    CopyEPoint(r,x1);
    return r;
}


const char* vertexPath(const GObject *v, char *buffer, size_t size)
{
    if (isObject(v) && buffer && (1<size))
    {
        char *mbuffer = buffer;
        if (v->parent)
        {
            if (vertexPath(v->parent, buffer, size))
            {
                size_t len = strnlen(buffer,size); 
                
                if (len < size)
                {
                    size =  size - len;
                    mbuffer = &buffer[len];            
                }
                else
                {
                    return NULL;
                }
            }
            else
            {
                return NULL;
            }
        }        
        
        char pindex[30];
        pindex[0] = 0;
        const char *tname = objectName(v->type);  
        char name_buffer[30];
        const char *trans = "";
        
        if (NULL == tname)
        {
            tname = "?";
        }
        
        if (0 == (_WOBJ_MARKER & v->type))
        {
            if (v->flags & GM_GOBJECT_FLAG_POLY_ORIGINAL)
            {
                // This is the original data from a file and should not be modified
                trans = "O";
            }
        }
        
        if (OBJ_MODEL == v->type)
        {
            tname = name_buffer;
            const CObject *cobj = (const CObject *)isCObject(ANY_COBJECT,v);
            strncpy(name_buffer,cobj->name,sizeof(name_buffer));
            
            name_buffer[2] = 0;
        }
        else
        {
            pindex[0] = '?';
            pindex[1] = 0;
            
            if (v->index > 0)
            {
                snprintf(pindex,sizeof(pindex),"%i",v->index);
            }
            else
            {
                if (0 == v->index)
                {
                    if (_WOBJ_MARKER & v->type)
                    {
                        // Toplevel widget. Has no parent
                        pindex[0] = '0';
                        pindex[1] = 0;                    
                    }
                }
            }
            
            strncpy(name_buffer,tname,sizeof(name_buffer));
            tname = name_buffer;            

            name_buffer[1] = 0;
        }
        
        if (v->parent)
        {    
            // When a parent present we need a dot
            int sret = snprintf(mbuffer,size,".%s%s%s",trans,tname,pindex);
            
            if ((sret < 0) || ((int)size <= sret))
            {
                return NULL;
            }
        }
        else
        {
            //No more parent. No dot needed
            int sret = snprintf(mbuffer,size,"%s%s%s",trans,tname,pindex);

            if ((sret < 0) || ((int)size <= sret))
            {
                return NULL;
            }
        }
                
        return buffer;
    }
    
    if (buffer && (1 < size))
    {
        ERROR("Invalid object found at %p!\n",v);
    }
    
    return UNKNOWN_SYMBOL;
}

const char* vertexFlagsToString(const GObject *object, char *buffer, size_t size)
{
    if (isObject(object))
    {            
        return vertexJustFlagsToString(object->flags,buffer,size);
    }

    return NULL;
}

const char* vertexJustFlagsToString(flag_type flags, char *buffer, size_t size)
{
   if (buffer && (0 < size))
   {
        static const char *flags_string[] = { 
                                /*0*/        "mod","vis","snorm","stri",
                                /*4*/        "sbb","surf","tri","nocalc",
                                /*8*/        "load","calc","col","abs",
                                /*12*/       "Orig","mark","str","nval",
                                /*16*/       "inv","mcon","rep","mark2",
                                /*20*/       "emark","seen","?","?",
                                /*24*/       "u1","u2","u3","u4",
                                /*28*/       "u5","u6","u7","u8",
                                /*32*/       "sel","mse" };
        
        buffer[0] = 0;

        for (int i = 0; i < (int)(sizeof(flags_string)/sizeof(flags_string[0]));i++)
        {
            if (flags & ((flag_type)1<<i))
            {
                if (buffer[0])
                {
                    commenStringCat(buffer,",",size);
                }
                commenStringCat(buffer,flags_string[i],size);                
            }
        }    
        
        return buffer;
   }
   
   return NULL;
}


const char*vertexToString(const Vertex* x, char* buffer, size_t size, int text_flags)
{
    if (x && buffer)
    {
        // Recursion here. It calls vertexToString()
        // CHECKVERTEX(x);

        if (text_flags)
        {
            char index[30];            
            index[0] = 0;
            char vector[100];
            vector[0] = 0;
            char flags[150];
            flags[0] = 0;
            char connections[100];
            connections[0] = 0;
            
            if (x->parent)
            {
                if (text_flags & GM_V2S_FLAG_INDEX)
                {
                    vertexPath((const GObject*)x, index, sizeof(index));
                }
                
                if (text_flags & GM_V2S_FLAG_VECTOR)
                {
                    snprintf(vector,sizeof(vector),"%g,%g,%g",x->x[0],x->x[1],x->x[2]);
                }                
                if (text_flags & GM_V2S_FLAG_FLAGS)
                {
                    if (text_flags & GM_V2S_FLAG_FLAGS_VERBOSE)
                    {                        
                        vertexFlagsToString((const GObject*)x,flags,sizeof(flags));
                    }
                    else
                    {
                        if (text_flags & GM_V2S_FLAG_FILE_FORMAT)
                        {        
                            // No need to persist these flags
                            flag_type flags1 = x->flags & ~(GM_FLAG_MASK_NOT_PERSISTENT);
                            if (flags1)
                            {
                                snprintf(flags,sizeof(flags),"0x%LX",flags1);
                            }
                        }
                        else
                        {
                            snprintf(flags,sizeof(flags),"0x%LX",x->flags);
                        }
                    }
                }                

                if (text_flags & GM_V2S_FLAG_CONNECTIONS)
                {
                    for (int i = 0; i < x->number_of_connections;i++)
                    {
                        const Vertex *connected = x->connections[i];
                        
                        if (connected)
                        {
                            char path[50];
                            vertexPath((const GObject*)connected, path, sizeof(path));
                            if (connections[0])
                            {
                                commenStringCat(connections,",",sizeof(connections));
                            }
                            commenStringCat(connections,path,sizeof(connections));
                        }
                    }
                }
                
                buffer[0] = 0;
                if (index[0])
                {
                    commenStringCat(buffer,index,size);
                }

                if (vector[0])
                {
                    if (text_flags & GM_V2S_FLAG_FILE_FORMAT)
                    {
                        commenStringCat(buffer,"(",size);
                    }
                    else
                    {
                        commenStringCat(buffer,"=(",size);
                    }
                    commenStringCat(buffer,vector,size);
                }

                if (flags[0])
                {
                    if (text_flags & GM_V2S_FLAG_VECTOR)
                    {
                        commenStringCat(buffer,",",size);
                    }
                    else
                    {
                        commenStringCat(buffer,"(",size);
                    }
                    commenStringCat(buffer,flags,size);
                    commenStringCat(buffer,")",size);
                }
                else
                {
                    if (vector[0])
                    {
                        commenStringCat(buffer,")",size);
                    }
                }
                
                if (connections[0])
                {
                    commenStringCat(buffer,"->(",size);
                    commenStringCat(buffer,connections,size);
                    commenStringCat(buffer,")",size);
                }
            }
        }
        else
        {
            if (x->flags)
            {
                snprintf(buffer,size,"(%g,%g,%g,0x%LX)",x->x[0],x->x[1],x->x[2],x->flags);
            }
            else
            {
                snprintf(buffer,size,"(%g,%g,%g)",x->x[0],x->x[1],x->x[2]);
            }
        }
    }
    else
    {
        if (buffer)
        {
            strncpy(buffer,"<null>",size);
        }
    }
    
    return buffer;
}

const char*EPoint3ToString(const EPoint* x, char* buffer, size_t size)
{
    if (x)
    {
        snprintf(buffer,size,"(%g,%g,%g)",x[0],x[1],x[2]);    
    }
    else
    {
        strncpy(buffer,"<null>",size);
    }
    return buffer;    
}

const char*EPointL3ToString(const EPointL* x, char* buffer, size_t size)
{
    if (x)
    {
        snprintf(buffer,size,"(%Lg,%Lg,%Lg)",x[0],x[1],x[2]);    
    }
    else
    {
        strncpy(buffer,"<null>",size);
    }
    return buffer;    
}



void vertexRemoveConnectionsFromPoly(Polygon *poly)
{
    for(Vertex *v = vertexPolygonIterator(poly,NULL); v; v = vertexPolygonIterator(poly,v))
    {
        vertexRemoveConnections(v);
    }
}


int vertexDestroyAttribute(AreaAttributes *attrib)
{
    if (isGObject(OBJ_ATTRIB,attrib))
    {
        attrib->refCount--;
        if (0 >= attrib->refCount)
        {
            attrib->magic = 0;
            memory_free(attrib);
            return 0;
        }
    }
    
    return 1;
}

int vertexDestroy(Vertex *vertex)
{
    if (isGObject(OBJ_VERTEX,vertex))
    {
#ifdef _DEBUG    
    if (vertex)
    {
        CHECKVERTEX(vertex);
    }
#endif    
        
        vertex->refCount--;
        char s1[GM_VERTEX_BUFFER_SIZE];
        vertexPath((GObject*)vertex,s1,sizeof(s1));
        
        if (0 >= vertex->refCount)
        {            
            vertex->magic = 0;
            if (vertex->connections)
            {
                memory_free(vertex->connections);
                vertex->connections = NULL;
            }
            if (vertex->first_attrib)
            {
                AreaAttributes *norm = vertex->first_attrib;
                while(norm)
                {
                    norm->refCount--;
                    if (0 >= norm->refCount)
                    {
                        AreaAttributes *n = norm;
                        norm = (AreaAttributes *)norm->next;
                        vertexDestroyAttribute(n);
                    }
                }
                
                vertex->first_attrib = NULL;
                vertex->last_attrib = NULL;
            }    
            if (0 == (_SOBJ_MARKER & vertex->type))
            {     
#ifdef _DEBUG_DELETING                                
                LOG("Deleting [%s] for real\n",s1);
#endif                
                
                memory_free(vertex);
            }
            
            return 0;
        }
        else
        {
            if (_SOBJ_MARKER & vertex->type)
            {
                ERROR("Did not delete static vertex [%s] due to references\n",s1);
            }
        }
    }
    
    return 1;
}

void vertexRemoveConnections(Vertex *vertex)
{
    if (isGObject(OBJ_VERTEX,vertex))
    {
        vertex->number_of_connections = 0;
        vertex->flags &= ~GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;
    }    
}


void vertexDestroyMaterial(Material *material)
{
    if (isGObject(OBJ_MATERIAL,material))
    {
        material->refCount--;
        if (0 >= material->refCount)
        {
            material->magic = 0;
            memory_free(material);
        }
    }    
}


int vertexReplaceMaterial(CObject* object, Material *material)
{
    if (isCObject(ANY_COBJECT,object))
    {
        if (isGObject(OBJ_MATERIAL,material))
        {
            if (object->material != material)
            {
                if (object->material)
                {
                    vertexDestroyMaterial(object->material);
                    object->material = NULL;
                }

    #ifdef _DEBUG        
                char s1[GM_VERTEX_BUFFER_SIZE];
                LOG("Adding material \"%s\" to [%s]\n",material->name,vertexPath((GObject*)object,s1,sizeof(s1)));
    #endif
                
                object->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
                if (object->parent)
                {
                    object->parent->flags &= ~GM_GOBJECT_FLAG_CALCULATED;
                }
                object->material = material;
                material->refCount++;        
            }
            
            return 0;
        }
    }
    
    return -1;
}


int vertexCountPolygones(const CObject *object, flag_type flag_mask)
{
    int count = 0;    
    Solid *solid =(Solid*)isCObject(OBJ_SOLID,object);
    if (solid)
    {
        for (const Polygon *poly = solid->first; NULL != poly; poly = (const Polygon*)poly->next)
        {    
            if (flag_mask == (poly->flags & flag_mask))
            {
                count++;
            }
        }    

        for (const Polygon *poly = solid->first_original; NULL != poly; poly = (const Polygon*)poly->next)
        {    
            if (flag_mask == (poly->flags & flag_mask))
            {
                count++;
            }
        }    
    }
    
    return count;
}



int vertexCountMeasurements(const CObject *object, flag_type flag_mask)
{
    if (isCObject(ANY_COBJECT,object))
    {
        int count = 0;
        for (const CObject *mea = object->first_mea; NULL != mea; mea = (const CObject*)mea->next)
        {    
            if (flag_mask == (mea->flags & flag_mask))
            {
                count++;
            }
        }  
        
        return count;
    }
    
    return 0;
}

int vertexCountVertices(const CObject *object, flag_type flag_mask)
{
    int count = -1;
    if (isCObject(ANY_COBJECT,object))
    {   
        count = 0;
        CIter iter = gobjectCreateConstIterator(object,flag_mask);
        for (const GObject *obj = gobjectConstIterate(iter); obj; obj = gobjectConstIterate(iter))
        {
            if ((obj->flags & flag_mask) == flag_mask)
            {
                count++;
            }
        }
    }
    
    return count;
}


int vertexSpawnSurface(const EPoint *n, const EPoint *p, EPoint *e1, EPoint *e2)
{
    vertexMakeNorm(e1,n,p,NULL);
    vertexMakeNorm(e2,n,e1,NULL);
        
    double e1n = vertexNorm(e1);    
    double ne1lu = vertexScalarProduct(e1,n);
    
    if ((fabs(e1n) <= 0.01) || (fabs(ne1lu) > 0.2))
    {
        e1[0] = n[0];
        e1[1] = n[2];
        e1[2] = n[1];
        e1n = vertexNorm(e1);    
    }    
    
    EPointExtend(e1,1/e1n,e1);
    
    double e2n = vertexNorm(e2);
    
    if (fabs(e2n) <= 0.1)
    {
        e2[0] = n[2];
        e2[1] = n[1];
        e2[2] = n[0];        
        e2n = vertexNorm(e2);    
    }    
    
    EPointExtend(e2,1/e2n,e2);
        
    double e2e1lu = vertexScalarProduct(e1,e2);    
    double ne2lu = vertexScalarProduct(e2,n);
    
    if ((fabs(e2e1lu) > 0.2) || (fabs(ne2lu) > 0.2))
    {
        e2[0] = n[1];
        e2[1] = n[0];
        e2[2] = n[2];
        e2n = vertexNorm(e2);            
    }    
    
    EPointExtend(e2,1/e2n,e2);
    
    e2e1lu = vertexScalarProduct(e1,e2);
    
    if (fabs(e2e1lu) > 0.2)
    {
        ERROR("Error: Failed to spawn surface. <e1,e2> = %.3f\n",e2e1lu);
        return -1;
    }
    
    return 0;
}

int vertexGetModelSize(Model *model, double *width, double *length, double *height)
{
    if (isCObject(OBJ_MODEL,model))
    {
        return vertexGetBoxSize(&model->box, width, length, height);
    }
    
    return -1;
}

int vertexGetCObjectSize(const CObject *object, double *width, double *length, double *height)
{
    if (isCObject(ANY_COBJECT,object))
    {
        return vertexGetBoxSize(&object->box, width, length, height);
    }
    
    return -1;
}

int vertexGetBoxSize(const BoundingBox *box, double *width, double *length, double *height)
{
    if (isGObject(OBJ_BOX,box))
    {
        if (width)
        {
            if ((-HUGE_VAL != box->max[0]) && (HUGE_VAL != box->min[0]))
            {
                *width = fabs(box->max[0] - box->min[0]);
            }
        }
        if (length)
        {
            if ((-HUGE_VAL != box->max[2]) && (HUGE_VAL != box->min[2]))
            {            
                *length = fabs(box->max[2] - box->min[2]);
            }
        }
        if (height)
        {
            if ((-HUGE_VAL != box->max[1]) && (HUGE_VAL != box->min[1]))
            {            
                *height = fabs(box->max[1] - box->min[1]);
            }
        }        
        
        return 0;
    }
    
    return -1;
}


EPointL vertexDet3(const EPoint *x1, const EPoint *x2, const EPoint *x3)
{
    const long double la10 = x1[0];
    const long double la11 = x1[1];
    const long double la12 = x1[2];

    const long double lb20 = x2[0];
    const long double lb21 = x2[1];
    const long double lb22 = x2[2];

    const long double lc30 = x3[0];
    const long double lc31 = x3[1];
    const long double lc32 = x3[2];

    // SARRUS ???
//     const long double s1 = la10+lb21+lc32;
//     const long double s2 = lb20+lc31+la12;
//     const long double s3 = lc30+la11+lb22;

//     const long double ns1 = la12+lb21+lc30;
//     const long double ns2 = lb22+lc31+la10;
//     const long double ns3 = lc32+la11+lb20;
// 
//     return ((s1+s2+s3) - (ns1+ns2+ns3));
    
     const long double d1 = la10*(lb21*lc32 - lb22*lc31);
     const long double d2 = la11*(lb20*lc32 - lb22*lc30);
     const long double d3 = la12*(lb20*lc31 - lb21*lc30);

     return d1 - d2 + d3;
}

EPointL vertexDet2(const EPoint *x1, const EPoint *x2)
{
   EPointL la10 = x1[0];
   EPointL la11 = x1[1];

   EPointL lb20 = x2[0];
   EPointL lb21 = x2[1];

   const EPointL d1 = la10*lb21;
   const EPointL d2 = la11*lb20;
    
    return d1 - d2;
}

int _vertexAdjustNorm(EPoint *norm, const EPoint *x1, const EPoint *x2)
{
    EPointL eps[3];
    EPoint e1[3], e2[3];
    
    vertexNormalize(e1,x1);
    vertexNormalize(e2,x2);
    
    const EPointL ne1 = vertexNorm(e1);
    const EPointL ne2 = vertexNorm(e2);
    
    const EPointL sp1 = vertexScalarProduct(norm,e1);
    EPointL nn = vertexNorm(norm);

    // A fixed correction eps can be used here too instead of the angle???
    
    if (fabsl(sp1) > 0.000001)
    {
        EPointL sign = sp1/(nn*ne1);
        EPointL alpha1 = GMATH_PI/2 - acosl(sign);
        if (fabsl(alpha1) > GMATH_PI/720.0)
        {        
            const EPointL d = sinl(alpha1) * nn;
#ifdef _DEBUG_NORM            
            LOG("Adjust: d1=%.6Lf, alpha1=%.4Lf° (nn=%.4Lf, nx1=%.4Lf, sp1=%.6Lf)\n",d,alpha1/GMATH_PI*180.0,nn,ne1,sp1);
#endif            
            EPointExtend(eps,-d,e1);        
            AddEPoints(norm,norm,eps);
            vertexNormalize(norm,norm);
        }
    }

    const EPointL sp2 = vertexScalarProduct(norm,e2);
    nn = vertexNorm(norm);

    if (fabsl(sp2) > 0.000001)
    {
        EPointL sign = sp2/(nn*ne2);
        EPointL alpha2 = GMATH_PI/2 - acosl(sign);
        if (fabsl(alpha2) > GMATH_PI/720.0)
        {
            const EPointL d = sinl(alpha2) * nn;
#ifdef _DEBUG_NORM
            LOG("Adjust: d2=%.6Lf, alpha2=%.4Lf° (nn=%.4Lf, nx2=%.4Lf, sp2=%.6Lf)\n",d,alpha2/M_PI*180.0,nn,ne2,sp2);
#endif            
            EPointExtend(eps,-d,e2);
            
            AddEPoints(norm,norm,eps);
            vertexNormalize(norm,norm);
        }
    }

    return 0;
} 

EPoint* vertexMakeNorm(EPoint *norm, const EPoint *x1, const EPoint *x2, const EPoint *x3)
{
    if (x1 && x2)
    {
        const EPoint *gx1 = x1;
        const EPoint *gx2 = x2;
        const EPoint *triangle[] = { x1,x2,x1,x3,x2,x3,NULL,NULL };
        
        if (x3)
        {
            double sp12 = fabsl(vertexScalarProduct(x1,x2));
            double sp13 = fabsl(vertexScalarProduct(x1,x3));
            double sp23 = fabsl(vertexScalarProduct(x2,x3));
            
            if ((sp12 < sp13) && (sp12 < sp23))
            {
                gx1 = x1;
                gx2 = x2;
            }
            else
            if ((sp13 < sp12) && (sp13 < sp23))
            {
                gx1 = x1;
                gx2 = x3;
            }
            else
            if ((sp23 < sp12) && (sp23 < sp13))
            {
                gx1 = x2;
                gx2 = x3;
            }       
        }
        
        long double n1[3];
        long double n2[3];
        long double r[3];
        long double s;
        const EPoint **t = triangle;
        
        while(gx1 && gx2)
        {
            n1[0] = ((long double)gx1[0]) * 1000ll;
            n1[1] = ((long double)gx1[1]) * 1000ll;
            n1[2] = ((long double)gx1[2]) * 1000ll;
            
            n2[0] = ((long double)gx2[0]) * 1000ll;
            n2[1] = ((long double)gx2[1]) * 1000ll;
            n2[2] = ((long double)gx2[2]) * 1000ll;
            
            // Vector product
            r[0] = n1[1]*n2[2] - n1[2]*n2[1];
            r[1] = n1[2]*n2[0] - n1[0]*n2[2];
            r[2] = n1[0]*n2[2] - n1[1]*n2[0];

            s = r[0]*r[0] + r[1]*r[1] + r[2]*r[2];
        
            if (s > 0.01)
            {
                // Vector combination can be used!
                break;
            }
            
            // Try to find a usable vector combination           
            gx1 = *(t++);
            gx2 = *(t++);            
        }
        
        if (gx1 && gx2)
        {
            // Norm
            long double no = sqrtl(s);
        
            if ( fabsl(no) > 0.000000001 )
            {
                norm[0] = ((long double)r[0])/((long double)no);
                norm[1] = ((long double)r[1])/((long double)no);
                norm[2] = ((long double)r[2])/((long double)no);

        #ifdef _DEBUG_NORM                
                const EPointL nx1 = vertexNorm(gx1);
                const EPointL nx2 = vertexNorm(gx2);
                EPointL sp1 = vertexScalarProduct(norm,gx1)/nx1;
                EPointL sp2 = vertexScalarProduct(norm,gx2)/nx2;
                LOG("Norm: sp1=%.6Lf, sp2=%.6Lf\n",sp1,sp2);
        #endif
                
                // Do 4 itarations to get a better normal vector
                _vertexAdjustNorm(norm,gx1,gx2);
                _vertexAdjustNorm(norm,gx1,gx2);
                _vertexAdjustNorm(norm,gx1,gx2);
                _vertexAdjustNorm(norm,gx1,gx2);
                
        #ifdef _DEBUG_NORM        
                sp1 = vertexScalarProduct(norm,gx1)/nx1;
                sp2 = vertexScalarProduct(norm,gx2)/nx2;
                LOG("Adjusted: Norm: sp1=%.6Lf, sp2=%.6Lf\n",sp1,sp2);
        #endif        
                
                // Make sure the length is one
                vertexNormalize(norm, norm);

                return norm;
            }
            else
            {
                char str1[50],str2[50];
                ERROR("Error: Parallel vectors for normale: gx1=%s, gx2=%s\n",
                        EPoint3ToString(gx1,str1,sizeof(str1)),
                        EPoint3ToString(gx2,str2,sizeof(str2)));
            }
        }
        else
        {
            ERROR1("Error: Parallel vectors for normale!\n");
        }
    }
    else
    {
        ERROR1("Missing vector!\n");
    }
    
    return NULL;
}


AreaAttributes *vertexCreateAttribute(Vertex *v)
{
#ifdef _DEBUG    
    if (v)
    {
        CHECKVERTEX(v);
    }
#endif    
    if (isObject(v))
    {
        for (AreaAttributes *attrib = (AreaAttributes *)isGObject(OBJ_ATTRIB,v->first_attrib); isGObject(OBJ_ATTRIB,attrib);attrib = (AreaAttributes *)attrib->next)
        {            
            if (0 == (attrib->flags & GM_ATTRIB_FLAG_VALID))
            {
                // need to preserve the next pointer here
                // memset(attrib,0,sizeof(AreaAttributes));
                
                attrib->flags = 0;
                attrib->flags |= GM_ATTRIB_FLAG_VALID;
                
                return attrib;
            }
        }
        
        
        AreaAttributes *attrib = (AreaAttributes*)memory_aloc(sizeof(AreaAttributes));
        memset(attrib,0,sizeof(AreaAttributes));
        
        objectInit((GObject*)attrib, (GObject*)v,/* index */  0, OBJ_ATTRIB);

        
        if (NULL == v->first_attrib)
        {
            v->first_attrib = attrib;
            v->last_attrib = attrib;
        }
        else
        {
            v->last_attrib->next = (GObject*)attrib;
            v->last_attrib = attrib;
        }    

        attrib->flags |= GM_ATTRIB_FLAG_VALID;

        return attrib;
    }
    
    return NULL;
}

AreaAttributes *vertexCloneAttribute(Vertex *v, const AreaAttributes *attrib)
{
    if (isObject(v) && isGObject(OBJ_ATTRIB,attrib))
    {
        AreaAttributes *new_attrib = vertexCreateAttribute(v);
        
        memcpy(&new_attrib->normale,&attrib->normale,sizeof(AreaAttributes)-sizeof(GObject)-sizeof(attrib->refCount));
        new_attrib->flags = attrib->flags;
        
        return new_attrib;
    }
    
    return NULL;
}

int vertexAddConnection(Vertex *v1, Vertex *v2)
{
    if (isGObject(OBJ_VERTEX,(GObject*)v1) && isGObject(OBJ_VERTEX,(GObject*)v2) && (v1 != v2))
    {
        CHECKVERTEX(v1);
        
        if (v1->max_number_of_connections <= v1->number_of_connections)
        {
            unsigned int old_size = v1->max_number_of_connections;
            v1->max_number_of_connections += 10;
            v1->connections = (Vertex**)memory_realloc(v1->connections ,sizeof(Vertex*)*v1->max_number_of_connections);
            
            if (old_size > 0)
            {
                memset(&v1->connections[old_size],0,(v1->max_number_of_connections-old_size)* sizeof(Vertex*));
            }
            else
            {
                memset(v1->connections,0,v1->max_number_of_connections* sizeof(Vertex*));
            }            
        }
        
        for (int i = 0 ; i < v1->number_of_connections; i++)
        {
            if (v1->connections[i] == v2)
            {
                char s1str[GM_VERTEX_BUFFER_SIZE],s2str[GM_VERTEX_BUFFER_SIZE];
                ERROR("Error: [%s] already connected to [%s] at connection #%i/%i\n",
                        vertexToString(v1, s1str, sizeof(s1str), GM_V2S_FLAGS_INDEX_VERBOSE),
                        vertexToString(v2, s2str, sizeof(s2str), GM_V2S_FLAGS_INDEX_VERBOSE),
                        i,v1->number_of_connections                        
                       );
                
                return -1;
            }
        }

#ifdef _DEBUG_VERTEX                
        char s1str[GM_VERTEX_BUFFER_SIZE],s2str[GM_VERTEX_BUFFER_SIZE];
        LOG("Connecting: [%s] -> [%s]\n",
                        vertexToString(v1, s1str, sizeof(s1str), GM_V2S_FLAGS_DEBUG_INDEX),
                        vertexToString(v2, s2str, sizeof(s2str), GM_V2S_FLAGS_DEBUG_INDEX)
               );
#endif
        
        v1->connections[v1->number_of_connections++] = v2;
        
        return 0;
    }  
    
    return -1;
}

Vertex *vertexShareParentConnection(const Vertex *v1, const Vertex *v2)
{
    if (v1 && v2)
    {
        for (int j = 0 ; j < v2->number_of_connections; j++)
        {    
            for (int i = 0 ; i < v1->number_of_connections; i++)
            {
                if (v1->connections[i] == v2->connections[j])
                {
                    return v2->connections[j];
                }
            }
        }
    }
    
    return NULL;
}

Vertex *vertexIsConnected(Vertex *v1, Vertex *v2)
{
    if (v1 && v2)
    {
        for (int j = 0 ; j < v2->number_of_connections; j++)
        {    
            if (v2->connections[j] == v1)
            {
                return v2;
            }
        }
        
        for (int i = 0 ; i < v1->number_of_connections; i++)
        {
            if (v1->connections[i] == v2)
            {
                return v1;
            }
        }        
    }
    
    return NULL;
}

Vertex *vertexConnectedParent(CObject *object, Vertex *v1, flag_type mask)
{
    Iter iter = gobjectCreateIterator(object,mask);
    for (Vertex *v = (Vertex *)gobjectIterate(iter); v; v = (Vertex *)gobjectIterate(iter))
    {
        Vertex *parent = vertexIsConnected(v1, v);
        if (parent)
        {
            return parent;
        }
    }
    
    return NULL;
}


int vertexAddToBox(BoundingBox *box, const Vertex *v)
{
    if (isGObject(OBJ_BOX,box))
    {
        if (0 == (box->flags & GO_BB_CALCULATED))
        {
            box->max[0] = MAX(box->max[0],v->x[0]);
            box->max[1] = MAX(box->max[1],v->x[1]);
            box->max[2] = MAX(box->max[2],v->x[2]);

            box->min[0] = MIN(box->min[0],v->x[0]);
            box->min[1] = MIN(box->min[1],v->x[1]);
            box->min[2] = MIN(box->min[2],v->x[2]);            
        }
        
        return 0;
    }
    return -1;
}

int vertexAddToBoundingBox(CObject *cobject, const Vertex *v)
{
    CObject *cobj = isCObject(ANY_COBJECT,cobject);
    if (cobj)
    {
        int err = vertexAddToBox(&cobj->box,v);

        if (isCObject(ANY_COBJECT,cobj->parent))
        {
            err = vertexAddToBoundingBox((CObject *)cobj->parent, v);
        }
        
        return err;
    }
    return -1;
}

int vertexLineIntersection(EPoint *x, const EPoint *dir1, const EPoint *p1, const EPoint *dir2, const EPoint *p2)
{
    int ret = -1;

    EPoint b[3];
    DiffEPoints(b,p1,p2);

    EPoint normal[3];
    
    if (vertexVectorProduct(normal,dir1,dir2))
    {    
        const long double det = vertexDet3(dir2,dir1,normal);
        
        long double d1 = 0;
        long double d2 = 0;
        long double di = 0;
        EPoint x1[3],x2[3];
        EPoint dc1[3],dc2[3];
        if (fabsl(det) > 0.0001)
        {
            d1 = - vertexDet3(dir2,b,normal)/det;
            d2 = vertexDet3(b,dir1,normal)/det;
            
            EPointExtend(dc1,d1,dir1);
            AddEPoints(x1,dc1,p1);
        //    For avarage of both intersection points
            EPointExtend(dc2,d2,dir2);
            AddEPoints(x2,dc2,p2);
            
            EPoint diff[3];
            DiffEPoints(diff,x2,x1);            
            
            di = vertexNorm(diff);
            if (1 > di)
            {   
                EPoint eps[3];
                EPointExtend(eps,0.5,diff);
                                
                AddEPoints(x,dc2,eps);
                AddEPoints(x,x,p2);
                
                ret = 0;
            }
        }
        
#ifdef _DEBUG_TRIANG        
        char cstr[50],xstr[50];
        char c1str[50],x1str[50];
        char c2str[50],x2str[50];
        char c3str[50],x3str[50];                
        LOG("d1=%.4Lf, d2=%.4Lf, det=%.4Lf, diff=%.4Lf\n"
                        "x1=%s, c1=%s, v1=%s, dc1=%s\n"
                        "x2=%s, c2=%s, v2=%s, dc2=%s\n",
                d1,d2,det,di,
                EPoint3ToString(x1,cstr,sizeof(cstr)),
                EPoint3ToString(dir1,xstr,sizeof(xstr)),
                EPoint3ToString(p1,c1str,sizeof(c1str)),
                EPoint3ToString(dc1,x1str,sizeof(x1str)),           
                EPoint3ToString(x2,c2str,sizeof(c2str)),
                EPoint3ToString(dir2,x2str,sizeof(x2str)),
                EPoint3ToString(p2,c3str,sizeof(c3str)),
                EPoint3ToString(dc2,x3str,sizeof(x3str))                                           
            );
#endif    
    }
    
    return ret;
}

int vertexMakeBBNormal(BoundingBox *box)
{
    if (isGObject(OBJ_BOX,box) && (0 == (box->flags & GO_BB_CALCULATED)))
    {
        EPoint l1_d[3];
        EPoint l2_d[3];
        EPoint l2_p1[3];
        EPoint l2_p2[3];
        
        DiffEPoints(l1_d,box->max,box->min);
        
        l2_p1[0] = box->min[0];
        l2_p1[1] = box->max[1];
        l2_p1[2] = box->max[2];

        l2_p2[0] = box->max[0];
        l2_p2[1] = box->min[1];
        l2_p2[2] = box->min[2];

        DiffEPoints(l2_d,l2_p2,l2_p1);

        vertexLineIntersection(box->center, l1_d, box->min, l2_d, l2_p1);
        
        // AddEPoints(box->center,box->p,box->center);
        
        EPoint x1[3] = { box->min[0],box->min[1],box->min[2] };
        EPoint x2[3] = { box->max[0], 
                            box->min[1],
                            box->min[2]};
        EPoint x3[3] = { box->max[0], 
                            box->min[1],
                            box->max[2]};
        EPoint x4[3] = { box->min[0], 
                            box->min[1],
                            box->max[2]};
        EPoint x5[3] = { box->min[0], 
                            box->max[1],
                            box->max[2]};
        EPoint x6[3] = { box->max[0],box->max[1],box->max[2] };
        EPoint x7[3] = { box->max[0], 
                            box->max[1],
                            box->min[2]};
        EPoint x8[3] = { box->min[0], 
                            box->max[1],
                            box->min[2]};
         
                            
        vertexVectorCopy(&box->corner[0*3],x1);
        vertexVectorCopy(&box->corner[1*3],x2);
        vertexVectorCopy(&box->corner[2*3],x3);
        vertexVectorCopy(&box->corner[3*3],x4);
        vertexVectorCopy(&box->corner[4*3],x5);
        vertexVectorCopy(&box->corner[5*3],x6);
        vertexVectorCopy(&box->corner[6*3],x7);
        vertexVectorCopy(&box->corner[7*3],x8);   
        
        
/*      x5             x6  
 *     /|------------/|
 *  x8/ |         x7/ |
 *   /--|----------/  |
 *   |  |          |  |
 *   |  |----------|--|x3
 *   | / x4        |  /
 *   |/            | /
 * x1/--------------/x2
 */        
        EPoint d1[3],d2[3];
        DiffEPoints(d1,x6,x8);
        DiffEPoints(d2,x7,x5);
        
        vertexLineIntersection(box->top_center, d1, x8, d2, x5);
        
        // AddEPoints(box->top_center,box->top_center,box->p);
                
        box->flags |= GO_BB_CALCULATED;
        box->flags |= GO_BB_NEED_ROTATION;
        
        return 0;
    }
    
    return -1;
}

int vertexRotateBB(CObject *object)
{
    char s1[GM_VERTEX_BUFFER_SIZE];
    
    if (isCObject(ANY_COBJECT,object) && (object->box.flags & GO_BB_CALCULATED))
    {
        if (0 == (object->box.flags & GO_BB_ORIGINAL_DATA))
        {
            const BoundingBox *box = &object->box;
            
            const Solid *solid = (const Solid*)isCObject(OBJ_SOLID,object);
            if (solid)
            {
                box = &solid->original_box;
            }
            
            if (isGObject(OBJ_BOX,box))
            {                            
                _vertexRotateVertex(object->box.rotC, object->box.rotS, box->center, object->box.center);
                _vertexRotateVertex(object->box.rotC, object->box.rotS, box->top_center, object->box.top_center);

                // vertexRotateVertex(object, box->p,object->box.p);

                const int len = (int)(sizeof(box->corner)/sizeof(box->corner[0])/GM_VERTEX_DIMENSION);
#ifdef _DEBUG                
                if (8 != len)
                {
                    FATAL("Too many Box corners %i\n",len);
                }
#endif                
                
                for (int i = 0; i < len; i++)
                {
                    _vertexRotateVertex(object->box.rotC, object->box.rotS, &box->corner[i*3],&object->box.corner[i*3]);
                }

                EPoint yaw = object->box.yaw/M_PI * 180.0;
                EPoint roll = object->box.roll/M_PI * 180.0;
                EPoint pitch = object->box.pitch/M_PI * 180.0;   
#ifdef _DEBUG                
                LOG("BB rotated (%g,%g,%g,%g) on [%s]\n",yaw,roll,pitch,box->p[1],vertexPath((GObject*)object,s1,sizeof(s1)));
#endif                
                
                object->box.flags &= ~GO_BB_NEED_ROTATION;
                object->box.flags |= GO_BB_CALCULATED;
                
                return 0;
            }
        }
    }
    
    LOG1("Can not rotate BB\n");
    
    return -1;
}

int vertexLinePlaneIntersection(EPoint *x, const EPoint *plane_p, const EPoint *plane_n, const EPoint *line_p, const EPoint *line_d) 
{
    // line surface intersection:
    // c = (( plane_p - line_p) * plane_n) / (line_d * plane_n)
    // x = c * line_d + line_p
    
    EPoint pdiff[3];
    DiffEPoints(pdiff,plane_p,line_p);
    
    const long double c1 = vertexScalarProduct(pdiff,plane_n);
    const long double c = c1/vertexScalarProduct(line_d,plane_n);

    EPointExtend(x,c,line_d);
    AddEPoints(x,x,line_p);
    
    return 0;
} 

Polygon *vertexGetPolygon(Solid *solid, flag_type flag_mask, Polygon *iterator)
{
    if (NULL == iterator)
    {
        iterator = solid->first;
    }
    else
    {
        iterator = (Polygon*)iterator->next;
    }

    for (Polygon *poly = iterator; NULL != poly; poly = (Polygon*)poly->next)
    {    
        if (flag_mask == (poly->flags & flag_mask))
        {
            return poly;
        }
    }
    
    return NULL;
}

int _vertexRotateVertex(const EPointL *rotC, const EPointL *rotS, const EPoint *v1, EPoint *v2)
{
    const long double *c = rotC;
    const long double *s = rotS;
    
    const long double rx1 = v1[0]*(c[1]*c[2] + s[0]*s[1]*s[2]) +  
                       v1[1]*(c[1]*-s[2] + s[0]*s[1]*c[2]) + 
                       v1[2]*(-c[0]*s[1]);
                       
    const long double ry1 = v1[0]*(c[0]*s[2]) +  
                       v1[1]*(c[0]*c[2]) + 
                       v1[2]*s[0];

    const long double rz1 = v1[0]*(s[1]*c[2] - s[0] * c[1]*s[2]) +
                       v1[1]*(- s[1]*s[2] - s[0] *c[1] * c[2] ) + 
                       v1[2]*(c[0] * c[1]);

    EPoint x1[3] = { rx1,ry1,rz1 };
    
    CopyEPoint(v2,x1);
        
    return 0;
}


int vertexRotateVertex(CObject *object, const EPoint *v1, EPoint *v2)
{
    const long double *c = object->box.rotC;
    const long double *s = object->box.rotS;
    
    int ret = _vertexRotateVertex(c,s,v1,v2);
    
    return ret;
}

EPoint *vertexProject(EPoint *result, const EPoint *dir, const EPoint *vector)
{
    EPoint s = ScalarProduct(dir,vector);
    EPointExtend(result,s,dir);
    
    return result;
}

int vertexCompactConnections(Vertex* v)
{
    for (int k = 0; k < v->number_of_connections; k++)
    {
        Vertex *vert2 = v->connections[k];
        for (int i = 0; i < v->number_of_connections; i++)
        {
            Vertex *vert1= v->connections[i];
            if ((vert1 == vert2) && (i != k))
            {
                memmove(&v->connections[i],&v->connections[i+1],v->number_of_connections-i);
                v->number_of_connections--;
            }
        }
    }
    
    return 0;
}

// Replace all connections in <find> by <replace>
int vertexReplaceConnections(CObject *object, Vertex* find, Vertex *replace, flag_type mask)
{
    int number_of_connections_replaced = 0;
    
    if (isCObject(ANY_COBJECT,object) && isObject(find) && isObject(replace))
    {
        Iter iter = gobjectCreateIterator(object,mask);
        for (Vertex *v = (Vertex *)gobjectIterate(iter); v; v = (Vertex *)gobjectIterate(iter))
        {
            for (int k = 0; k < v->number_of_connections; k++)
            {
                Vertex *vert2 = v->connections[k];
                if (find == vert2)
                {
                    v->connections[k] = replace;
                    number_of_connections_replaced++;
                }
            }
            
            vertexCompactConnections(v);
            
            AreaAttributes *attributes = (AreaAttributes*)isGObject(OBJ_ATTRIB,v->first_attrib);
            while(attributes)
            {            
                int match = 0;
                
                if (attributes->v1 == find)
                {
                    attributes->v1 = replace;
                    match = 1;
                }
                if (attributes->v2 == find)
                {
                    attributes->v2 = replace;
                    match = 1;
                }
                if (attributes->v3 == find)
                {
                    attributes->v3 = replace;
                    match = 1;
                }
                /*
                if (attributes->v4 == find)
                {
                    attributes->v4 = replace;
                    match = 1;
                }
                */
                
                if (match)
                {
                    attributes->flags &= ~(GM_ATTRIB_FLAG_VALID|GM_ATTRIB_FLAG_NORMALE_VALID);                    
                }
                
                attributes = (AreaAttributes*)isGObject(OBJ_ATTRIB,attributes->next);
            }
            
        }
    }

    return number_of_connections_replaced;
}

// Merge v2 into v1 and delete v2
int vertexMergeVertices(CObject *object, Vertex *v1, Vertex *v2)
{
    if (object && v1 && v2)
    {
        EPoint g[3];
        DiffEPoints(g,v1->x,v2->x);
        EPointExtend(g,0.5,g);
        
        int number_of_connections_replaced = vertexReplaceConnections(object, v2, v1, 0);
        
#ifdef _DEBUG        
        char s1[GM_VERTEX_BUFFER_SIZE];
        char s2[GM_VERTEX_BUFFER_SIZE];
        char s3[GM_VERTEX_BUFFER_SIZE];

        LOG("Merging [%s]<-[%s]: dist=%s, connections replaced %i\n",
            vertexToString(v1,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
            vertexToString(v2,s2,sizeof(s2),GM_V2S_FLAGS_DEBUG_VERBOSE),
            EPoint3ToString(g, s3, sizeof(s3)),
            number_of_connections_replaced
        );
#endif        

        AddEPoints(v1->x,v2->x,g);
        
        // All calculations on the triangle are not valid anymore.
        v1->flags &= ~GM_GOBJECT_VERTEX_FLAG_NORMAL_VALID;
        
        AreaAttributes *attributes = (AreaAttributes*)isGObject(OBJ_ATTRIB,v1->first_attrib);
        while(attributes)
        {            
            attributes->flags &= ~(GM_ATTRIB_FLAG_VALID|GM_ATTRIB_FLAG_NORMALE_VALID);
            attributes = (AreaAttributes*)isGObject(OBJ_ATTRIB,attributes->next);
        }

        gobjectRemoveObject(object,v2);
        
        return 0;
    }
    
    return -1;
}    

Model *vertexGetModel(CObject *object)
{
    while (isObject(object))
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,object);
        if (model)
        {
            return model;
        }
        object = (CObject *)object->parent;
    }
    
    return NULL;
}


int gmathRemoveDublicates(CObject *object)
{
    Iter iter = gobjectCreateIterator(object,0);
    for (Vertex *v = (Vertex *)gobjectIterate(iter); v; v = (Vertex *)gobjectIterate(iter))
    {
        if (v)
        {
            Iter iter2 = gobjectCreateIterator(object,0);
            for (Vertex *v1 = (Vertex *)gobjectIterate(iter2); v1; v1 = (Vertex *)gobjectIterate(iter2))
            {        
                if (v1 && v && (v1 != v))
                {                
                    const double dist = vertexSphereDistance(v->x,v1->x);
                    
                    if (dist < 0.00001)
                    {
                        char s1[GM_VERTEX_BUFFER_SIZE];
                        char s2[GM_VERTEX_BUFFER_SIZE];
                        
                        ERROR("Dublicate vertex found:\n [%s],\n [%s]\n",
                                vertexToString(v,s1,sizeof(s1),GM_V2S_FLAGS_DEBUG_VERBOSE),
                                vertexToString(v1,s2,sizeof(s2),GM_V2S_FLAGS_DEBUG_VERBOSE));
                              
                       vertexMergeVertices(object, v, v1);
                    }                        
                }
                
            }
        }
    }
    
    return 0;
}


static Iter createIterate(CObject *object, flag_type mask)
{
    Iter iter;
    memset(&iter,0,sizeof(Iter));
    
    iter.object = object;
    iter.mask = mask;
    
    return iter;
}

static CIter createConstIterate(const CObject *object, flag_type mask)
{
    CIter iter;
    memset(&iter,0,sizeof(CIter));
    
    iter.object = object;
    iter.mask = mask;
    
    return iter;
}

static const GObject *citerateObject(CIter *iter)
{
    if (NULL == iter->s1)
    {
        const Model *model = (const Model*)isCObject(OBJ_MODEL,iter->object);
        if (model)
        {
            iter->s1 = (const GObject*)model->first;
        }
    }
    else
    {    
        iter->s1 = iter->s1->next;
    }
    
    return iter->s1;
}

static GObject *iterateObject(Iter *iter)
{
    if (NULL == iter->s1)
    {
        Model *model = (Model*)isCObject(OBJ_MODEL,iter->object);
        if (model)
        {        
            iter->s1 = (GObject*)model->first;
        }
    }
    else
    {    
        iter->s1 = iter->s1->next;
    }
    
    return iter->s1;
}

Model *createModel()
{
    Model *model = memory_aloc(sizeof(Model));
    memset(model,0,sizeof(Model));
    if (objectInit((GObject*)model, (GObject*)NULL, 0, OBJ_MODEL))
    {
        gobjectClearBoundingBox(&model->box);
                
        model->methods.iterate = iterateObject;
        model->methods.citerate = citerateObject;
        // model->methods.remove = modelRemoveObject;
        model->methods.creatIterator = createIterate;
        model->methods.creatConstIterator = createConstIterate;

       return model;
    }
    
    return NULL;
}
