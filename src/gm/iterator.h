#ifndef __ITERATOR__
#define __ITERATOR__

/* Iterator for GV 23.11.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"


// This iterator is not thread safe due to internal state
// The flags are polygone flags
// Vertex *vertexIterator(Vertex *vertex, Solid *solid, flag_type flags);
// const Vertex *vertexConstIterator(const Vertex *vertex, const Solid *solid, flag_type flags);

const Vertex *vertexOriginalIterator(const Vertex *vertex, const Solid *solid, flag_type flags);

//Vertex *vertexIter(CObject *object, Vertex *vertex, flag_type mask);
//const Vertex *vertexConstIter(const CObject *object, const Vertex *vertex, flag_type mask);


Vertex *vertexPolygonIterator(Polygon *poly, Vertex *vertex);
const Vertex *vertexConstPolygonIterator(const Polygon *poly, const Vertex *vertex);




Polygon *vertexSolidPolygonIterator(Solid *solid, Polygon *poly);
const Polygon *vertexPolygonConstIterator(Solid *solid, const Polygon *poly);
const Polygon *vertexSolidOriginalPolygonIterator(Solid *solid, const Polygon *poly);


Solid *vertexSolidIterator(Model *model, Solid *solid, flag_type flags);
CObject *vertexCObjectIterator(Model *model, CObject *object, flag_type flags);
const CObject *vertexConstCObjectIterator(const Model *model, const CObject *object, flag_type flags);


#endif