#ifndef MESH_SPREAD_1595984973500_H
#define MESH_SPREAD_1595984973500_H

#include "interface.h"
#include "trimesh2/TriMesh.h"
#include "trimesh2/XForm.h"
#include <vector>
#include <memory>


namespace trimesh {
    class TriMesh;
}

namespace Slic3r {
    class TriangleMesh;
    class TriangleSelector;
    namespace sla {
        class IndexedMesh;
    }
}

namespace ccglobal
{
    class Tracer;
}

namespace spread
{
    enum SPREAD_API CursorType {
        CIRCLE,//2d圆
        SPHERE,//3d球
        POINTER, //三角片 
        // BBS
        HEIGHT_RANGE,
        GAP_FILL,
    };

    struct SPREAD_API ClippingPlane
    {
        size_t facet_idx;//面索引
        trimesh::vec3 normal;//法线
        int extruderIndex;

        float offset;
    };

    enum class SPREAD_API EnforcerBlockerType : int8_t {
        // Maximum is 3. The value is serialized in TriangleSelector into 2 bits.
        NONE = 0,
        ENFORCER = 1,
        BLOCKER = 2,
        // Maximum is 15. The value is serialized in TriangleSelector into 6 bits using a 2 bit prefix code.
        Extruder1 = ENFORCER,
        Extruder2 = BLOCKER,
        Extruder3,
        Extruder4,
        Extruder5,
        Extruder6,
        Extruder7,
        Extruder8,
        Extruder9,
        Extruder10,
        Extruder11,
        Extruder12,
        Extruder13,
        Extruder14,
        Extruder15,
        ExtruderMax
    };


    class SPREAD_API MeshSpreadWrapper
    {
    public:
        MeshSpreadWrapper();
        ~MeshSpreadWrapper();

        void setInputs(trimesh::TriMesh* mesh, ccglobal::Tracer* tracer = nullptr);

        //POINTER, //三角片 
        void triangle_factory(int facet_start, int colorIndex);

        void cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane);
        void cursor_factory(const trimesh::vec& first_center, const trimesh::vec& second_center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane);
   
        trimesh::TriMesh* getTrimesh();
    private:
        void triangle_selector2trimesh(trimesh::TriMesh* mesh, Slic3r::TriangleSelector* triangle_selector);

    private:
        std::unique_ptr<Slic3r::TriangleMesh> m_mesh;
        std::unique_ptr<trimesh::TriMesh> m_triMesh;
        std::unique_ptr <Slic3r::TriangleSelector> m_triangle_selector;
    };

}
#endif // MESH_SPREAD_1595984973500_H