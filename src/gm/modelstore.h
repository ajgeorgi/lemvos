#ifndef __MODELSTORE__
#define __MODELSTORE__

/* modelstore for GM 29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"

extern void initModelstore();

extern Model *modelstoreCreate(const char *name, const char *owner);

extern Solid *modelstoreCreateSolid(Model *model, const char* name);
extern Solid *modelstoreGetSolid(Model *model, const char* name);
extern int modelstoreMoveSolidToBottom(Solid *solid);

extern Mesh *modelstoreCreateMesh(Model *model, const char* name);
extern Mesh *modelstoreGetMesh(Model *model, const char* name);

extern Mea *modelstoreCreateMeasurement(CObject *parent, const char *name);

extern Material *modelstoreCreateMaterial(CObject *object, const char *name);
extern Material *modelstoreGetMaterial(const char*name);

extern Polygon *modelstoreCreatePolygon(Solid *solid);
extern Polygon *modelstoreCreateOriginalPolygon(Solid *solid);
extern Polygon *modelstoreGetPolygon(Solid *solid, IndexType index);

extern Vertex *modelstoreAddToPolygon(Polygon *polygon, EPoint *v);
extern Vertex *modelstoreInsertToPolygon(Polygon *polygon, EPoint *v);
extern const Vertex *modelstoreAddToPolygonOriginal(Polygon *polygon, EPoint *v, flag_type flags, IndexType index);
//extern Box *modelstoreAddBoxToPolygon(Polygon *polygon);

extern Vertex *modelstoreGetVertex(Polygon *polygon, IndexType index);

extern Model *modelstoreGetModel(const char* name);

extern void modelstoreClear();

extern unsigned int modelstoreGetVertexCount();
extern const char* modelstoreReport(const Model *model, char *buffer, size_t size);


#endif // __MODELSTORE__