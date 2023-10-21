#ifndef SLIC3RWRAPPER_UTIL_1632383314974_H
#define SLIC3RWRAPPER_UTIL_1632383314974_H

#include "stl.h"
#include "Slice3rBase/ExPolygon.hpp"
#include "trimesh2/Vec.h"

namespace Slic3r {
	class TriangleMesh;
}

namespace trimesh {
	class TriMesh;
}

namespace ccglobal
{
	class Tracer;
}

namespace spread
{
	Slic3r::TriangleMesh* trimesh2Slic3rTriangleMesh(trimesh::TriMesh* mesh, ccglobal::Tracer* tracer = nullptr);
	void trimesh2IndexTriangleSet(trimesh::TriMesh* mesh, indexed_triangle_set& indexTriangleSet, ccglobal::Tracer* tracer = nullptr);
	Slic3r::TriangleMesh* constructTriangleMeshFromIndexTriangleSet(const indexed_triangle_set& itset, ccglobal::Tracer* tracer = nullptr);


	trimesh::TriMesh* slic3rMesh2TriMesh(const Slic3r::TriangleMesh& mesh);
}

#endif // SLIC3RWRAPPER_UTIL_1632383314974_H