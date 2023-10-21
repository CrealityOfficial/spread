#ifndef MESH_SPREAD_1595984973500_H
#define MESH_SPREAD_1595984973500_H

#include "spread/header.h"

namespace Slic3r {
    class TriangleMesh;
    class TriangleSelector;
    namespace sla {
        class IndexedMesh;
    }
}

namespace spread
{
    struct SPREAD_API ClippingPlane
    {
        size_t facet_idx;//面索引
        trimesh::vec3 normal;//法线
        int extruderIndex;

        float offset;
    };

    enum SPREAD_API TrimeshType {
        ADD,
        ALL
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

        //初始化数据
        void setInputs(trimesh::TriMesh* mesh, ccglobal::Tracer* tracer = nullptr);

        void testChunk();
        int chunkCount();
        void chunk(int index, std::vector<trimesh::vec3>& positions, std::vector<int>& flags, std::vector<int>& splitIndices);

        //设置颜色
        void setColorPlane(const std::vector<trimesh::vec>& color_plane);

        //POINTER, //三角片 
        void triangle_factory(int facet_start, int colorIndex, const CursorType& cursor_type = CursorType::POINTER);
        void triangle_factory1(int facet, int colorIndex);

        //细分
        void cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane);
        void cursor_factory(const trimesh::vec& first_center, const trimesh::vec& second_center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane);
   
        //填充
        void bucket_fill_select_triangles(const trimesh::vec& center, const ClippingPlane& clipping_plane, const CursorType& cursor_type = CursorType::GAP_FILL);
        
        //预填充
        void bucket_fill_select_triangles_preview(const trimesh::vec& center, const ClippingPlane& clipping_plane, const trimesh::vec& rayDir, std::vector<std::vector<trimesh::vec3>>& contour, const CursorType& cursor_type = CursorType::GAP_FILL);

        //
        void updateData();

        //返回
        trimesh::TriMesh* getTrimesh(const TrimeshType& type = TrimeshType::ALL);

        std::string get_triangle_as_string(int triangle_idx) const;
        void set_triangle_from_string(int triangle_id, const std::string& str);
        std::vector<std::string> get_data_as_string() const;
        void set_triangle_from_data(std::vector<std::string> strList);
        void updateTriangle();
    private:
        void triangle_selector2trimesh(trimesh::TriMesh* mesh, Slic3r::TriangleSelector* triangle_selector);
    private:
        int m_curFacet;
        CursorType  m_curCursor_type;
        std::vector<trimesh::vec> m_color_plane;
        std::unique_ptr<Slic3r::TriangleMesh> m_mesh;
        std::unique_ptr <Slic3r::TriangleSelector> m_triangle_selector;
        std::pair<std::vector<std::pair<int, int>>, std::vector<bool>> m_data;

        std::vector<int> m_faceChunkIDs;
        std::vector<std::vector<int>> m_chunkFaces;
    };

}
#endif // MESH_SPREAD_1595984973500_H