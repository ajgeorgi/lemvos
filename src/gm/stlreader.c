
#define _GNU_SOURCE  

#include <string.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

/* STL Reader for GV 23.03.2023


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "reader.h"
#include "stlreader.h"
#include "modelstore.h"
#include "loader.h"
#include "mea.h"
#include "commen.h"
#include "matter.h"

#define KWORD_STL_SOLID 1
#define KWORD_FACET 2
#define KWORD_NORMAL 3
#define KWORD_OUTER 4
#define KWORD_LOOP 5
#define KWORD_VERTEX 6
#define KWORD_ENDLOOP 7
#define KWORD_ENDFACET 8

#define STL_FILE_ASCII 1
#define STL_FILE_BIN 2

static Model *_reader_model = NULL;
static Component *_reader_component = NULL;

static ReadObject _reader_object_reader = NULL;

typedef struct STLHeadT {
    char text[80];
    unsigned int count;
} /* __attribute__((__packed__)) */ STLHead ;

typedef struct STLTriangleT {
    float normal[3];
    float v1[3];
    float v2[3];
    float v3[3];
    unsigned short acount;
} __attribute__((__packed__)) STLTriangle ;

static const GObject *stlreaderReadSTLSolid(IndexType index, ReaderScanner* scanner);
static const GObject *stlreaderReadVertex(IndexType index, ReaderScanner* scanner);
static const GObject *stlreaderCreate(ReaderScanner *scanner);

static const GObject *stlreaderReadSTLBinSolid(IndexType index, ReaderScanner* scanner);
static const GObject *stlreaderReadBinVertex(IndexType index, ReaderScanner* scanner);

static double _stlreader_scale = 1.0;
static char _stlreader_filename[250];
static char _stlreader_headline[100];

ReaderT initSTLReader(double scale, const char* filename)
{
    _stlreader_scale = scale;
        
    const char *sep = strrchr (filename,'.');
    
    size_t len = strlen (filename);
    if (sep)
    {
        len = sep- filename;
    }
    
    if (len > sizeof(_stlreader_filename))
    {
        len = sizeof(_stlreader_filename) - 1;
    }
    strncpy(_stlreader_filename,filename,len);
    _stlreader_filename[len] = 0;
    
    
    int fd = open(filename,O_RDONLY);
    if (0 >= fd)
    {
        fd = 0;
        return NULL;
    }
    
    STLHead head;
    memset(&head,0,sizeof(head));
    read(fd,&head,sizeof(head));    
    
    close(fd);
    fd = 0;
    
    head.text[sizeof(head.text)-1] = 0;
       
    char * solid_string = strcasestr(head.text,"solid");
    if (solid_string)
    {        
        // readerRegisterObject(OBJ_MODEL, stlreaderReadSTLSolid);
        // readerRegisterObject(OBJ_VERTEX, stlreaderReadVertex);    
        
        _reader_object_reader = stlreaderReadSTLSolid;

        LOG1("STL ASCII reader initialized.\n");        
    }
    else
    {
        // readerRegisterObject(OBJ_MODEL, stlreaderReadSTLBinSolid);
        // readerRegisterObject(OBJ_VERTEX, stlreaderReadBinVertex);    
        
        _reader_object_reader = stlreaderReadSTLBinSolid;        
        
        LOG("STL bin reader initialized: \"%s\".\nExpecting %i triangles.\n",head.text,head.count);
    }
 
    char *text = head.text;
    char *eol = strchr(head.text,'\n');
    if (eol)
    {
        *eol = 0;
    }
    
    if (solid_string)
    {
        text = &solid_string[sizeof("solid")];
    }
    
    text = commenStripString(text);
    if (text)
    {
        strncpy(_stlreader_headline,text,sizeof(_stlreader_headline));
        
        if (strstr(_stlreader_headline,"artec"))
        {
            _stlreader_scale = _stlreader_scale / 10.0;
        }        
    }
        
    return stlreaderCreate;
}

static int kwordType(char *keyw)
{
    if (keyw)
    {
        if (0 == strncmp(keyw,"solid",sizeof("solid")-1))
        {
            return KWORD_STL_SOLID;
        }
        else
        if (0 == strncmp(keyw,"facet",sizeof("facet")-1))
        {
            return KWORD_FACET;
        }
        else
        if (0 == strncmp(keyw,"normal",sizeof("normal")-1))
        {
            return KWORD_NORMAL;
        }
        else
        if (0 == strncmp(keyw,"outer",sizeof("outer")-1))
        {
            return KWORD_OUTER;
        }
        else
        if (0 == strncmp(keyw,"loop",sizeof("loop")-1))
        {
            return KWORD_LOOP;
        }
        else
        if (0 == strncmp(keyw,"vertex",sizeof("vertex")-1))
        {
            return KWORD_VERTEX;
        }
        else
        if (0 == strncmp(keyw,"endloop",sizeof("endloop")-1))
        {
            return KWORD_ENDLOOP;
        }  
        else
        if (0 == strncmp(keyw,"endfacet",sizeof("endfacet")-1))
        {
            return KWORD_ENDFACET;
        }          
    }
    
    return 0;
}

const GObject *stlreaderCreate(ReaderScanner *scanner)
{
    const GObject *object = NULL;

    if (scanner)
    {
        IndexType index = 0;
        
        if (_reader_object_reader)
        {
            object = _reader_object_reader(index, scanner);
            
            if (NULL == object)
            {
                LOG1("No more objects found!\n");                
            }
            else
            {
                if (STL_FILE_ASCII == scanner->file_type)
                {
                    // LOG1("Reading objects from ASCII file\n");
                    _reader_object_reader = stlreaderReadVertex;
                }
                else
                if (STL_FILE_BIN == scanner->file_type)
                {
                    // LOG1("Reading objects from BIN file\n");
                    _reader_object_reader = stlreaderReadBinVertex;
                }
                else
                {
                    _reader_object_reader = NULL;
                }
            }
        }
        else
        {
            ERROR1("Could not find a reader\n");
        }
        
        if (object)
        {
            if (0 == object->index)
            {
                LOG("Missing index of type %c%i, %s\n",object->type & _COBJ_MARKER ? 'C' :'G',object->type & _OBJ_TYPE_MASK,objectName(object->type));
            }  
            if (NULL == object->parent)
            {
                LOG("Parent missing of type %c%i, %s\n",object->type & _COBJ_MARKER ? 'C' :'G',object->type & _OBJ_TYPE_MASK,objectName(object->type));
            }
            if (0 == object->type)
            {
                LOG("Type missing of type %c%i!\n",object->type & _COBJ_MARKER ? 'C' :'G',object->type & _OBJ_TYPE_MASK);
            }
        }
    }
    
    return object;
}

const GObject *stlreaderReadSTLSolid(IndexType index, ReaderScanner* scanner)
{
    (void)index;
    
    if (scanner)
    {     
        scanner->file_type = STL_FILE_ASCII;
        
        for (char *keyword = readFromScanner(scanner);
             (0 == kwordType(keyword)) || (KWORD_STL_SOLID == kwordType(keyword));
             keyword = readFromScanner(scanner))
        {
            if (KWORD_STL_SOLID == kwordType(keyword))
            {
                LOG1("Creating model for STL mesh\n");
                _reader_model = modelstoreCreate(_stlreader_filename, NULL);
                
                if (_reader_model)
                {
                    _reader_model->index = index;

                    fseek(scanner->stream, 0, SEEK_END);        
                    scanner->number_of_elements = ftell(scanner->stream)/260;
                    scanner->element_index = 0;
                    fseek(scanner->stream, 0, SEEK_SET);
                    
                    LOG("Going to read %i STL triangles with %i vertices\n",scanner->number_of_elements, scanner->number_of_elements *3 );
                    
                    LOG1("Creating mesh for STL file\n");
                    int mesh_size = 500;
                    if (mesh_size < scanner->number_of_elements * 3)
                    {
                        mesh_size = scanner->number_of_elements * 3;
                    }
                    _reader_component = modelstoreCreateComponent(_reader_model,_stlreader_filename,mesh_size);
                    
                    if (_reader_component)
                    {
                        _reader_component->flags |= GM_GOBJECT_FLAG_LOADED;
                        
                        objectAddComment((CObject*)_reader_component,_stlreader_headline,sizeof(_stlreader_headline));                        
                    }                                         
                }
                
                break;
            }
        }        
    }
        
    LOG_FLUSH;
    
    return (GObject*)_reader_model;
}

const GObject *stlreaderReadVertex(IndexType index, ReaderScanner* scanner)
{
    (void)index;
    const GObject *object = NULL;
    
    if (scanner)
    {
        if (NULL == _reader_model)
        {
            ERROR1("No model created\n");
            return NULL;
        }
        
        if (NULL == _reader_component)
        {
            ERROR1("No component created\n");
            return NULL;
        }

        int point_count = 0;
        EPoint points[10];
        EPoint normal[3];
        
        for (char *keyword = readFromScanner(scanner);
             keyword;
             keyword = readFromScanner(scanner))
        {
            if (KWORD_LOOP == kwordType(keyword))
            {
                point_count = 0;
            }
            else
            if (KWORD_NORMAL == kwordType(keyword))
            {
                normal[0] = commenStringToDouble(readFromScanner(scanner)) * _stlreader_scale;
                normal[1] = commenStringToDouble(readFromScanner(scanner)) * _stlreader_scale;
                normal[2] = commenStringToDouble(commenStripString(readFromScanner(scanner))) * _stlreader_scale;
            }
            else                
            if (KWORD_VERTEX == kwordType(keyword))
            {
                points[(point_count * 3) + 0] = commenStringToDouble(readFromScanner(scanner)) * _stlreader_scale;
                points[(point_count * 3) + 1] = commenStringToDouble(readFromScanner(scanner)) * _stlreader_scale;
                points[(point_count * 3) + 2] = commenStringToDouble(commenStripString(readFromScanner(scanner))) * _stlreader_scale;
                
                point_count++;                
            }
            else
            if (KWORD_ENDLOOP == kwordType(keyword))
            {
                Triangle *triangle = componentAddTriangle(_reader_component,
                                                        &points[0],
                                                        &points[3],
                                                        &points[6]);
                
                CopyEPoint(triangle->normale.normal,normal);
                
                object = (const GObject*) triangle;
                break;                
            }                     
        }        
    }
    
    return (GObject*)object;
}

const GObject *stlreaderReadSTLBinSolid(IndexType index, ReaderScanner* scanner)
{
    (void)index;
    
    if (scanner)
    {
        scanner->file_type = STL_FILE_BIN;
        
        LOG1("Creating model for bin STL component\n");
        _reader_model = modelstoreCreate(_stlreader_filename, NULL);
        
        if (_reader_model)
        {
            _reader_model->index = index;
                        
            fseek(scanner->stream, sizeof(STLHead), SEEK_END);        
            scanner->number_of_elements = ftell(scanner->stream)/sizeof(STLTriangle);
            scanner->element_index = 0;
            fseek(scanner->stream, sizeof(STLHead), SEEK_SET);
            
            LOG("Going to read %i STL triangles with %i vertices\n",scanner->number_of_elements, scanner->number_of_elements *3 );
            
            int mesh_size = 500;
            if (mesh_size < scanner->number_of_elements * 3)
            {
                mesh_size = scanner->number_of_elements * 3;
            }
            
            LOG1("Creating component for bin STL file\n");
            _reader_component = modelstoreCreateComponent(_reader_model,_stlreader_filename, mesh_size);            
            
            if (_reader_component)
            {
                _reader_component->flags |= GM_GOBJECT_FLAG_LOADED;
                
                objectAddComment((CObject*)_reader_component,_stlreader_headline,sizeof(_stlreader_headline));
            }            
        }

        return (GObject*)_reader_model;    
    }
    
    return NULL;
}

const GObject *stlreaderReadBinVertex(IndexType index, ReaderScanner* scanner)
{
    (void)index;
    (void)scanner;
    const GObject *object = NULL;
    
    if (!scnannerEOF(scanner))
    {
        if (!scnannerEOF(scanner))
        {
            if (NULL == _reader_component)
            {
                ERROR1("No component while reading vertices!");
            }
            
            STLTriangle triangle;
            if (1 == scannerRead(scanner,&triangle,sizeof(triangle)))
            {
                EPoint v1[3];
                v1[0] = triangle.v1[0] * _stlreader_scale;
                v1[1] = triangle.v1[1] * _stlreader_scale;
                v1[2] = triangle.v1[2] * _stlreader_scale;

                EPoint v2[3];
                v2[0] = triangle.v2[0] * _stlreader_scale;
                v2[1] = triangle.v2[1] * _stlreader_scale;
                v2[2] = triangle.v2[2] * _stlreader_scale;

                EPoint v3[3];
                v3[0] = triangle.v3[0] * _stlreader_scale;
                v3[1] = triangle.v3[1] * _stlreader_scale;
                v3[2] = triangle.v3[2] * _stlreader_scale;
                
                if (triangle.acount)
                {
                    char s1[GM_VERTEX_BUFFER_SIZE];
                    char s2[GM_VERTEX_BUFFER_SIZE];
                    char s3[GM_VERTEX_BUFFER_SIZE];
                    int acount = triangle.acount;
                    
                    LOG("v1 = %s, v2 = %s, v3 = %s\n",
                        EPoint3ToString(v1,s1,sizeof(s1)),
                        EPoint3ToString(v2,s2,sizeof(s2)),
                        EPoint3ToString(v3,s3,sizeof(s3)));
                    
                    // scannerShutdown(scanner);
                    
                    ERROR("atttributes in STL file (%i)\n",acount);
                }
//                else
                {         
                    Triangle * tri = componentAddTriangle(_reader_component,&v1[0],&v2[0],&v3[0]);

                    object = (const GObject*) tri;
                    
#ifdef _DEBUG_READER                
                    putc('.',stderr);                
#endif                                        
                }
            }
            else
            {
                LOG1("File done\n");
            }
        }    
    }
    
    return object;
}
