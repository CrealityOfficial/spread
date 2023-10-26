#ifndef MESH_SPREAD_1595984973500_H
#define MESH_SPREAD_1595984973500_H

#include "spread/header.h"
#include <set>

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

        //add
        void triangle(int facet, int colorIndex, std::vector<int>& dirty_chunks);
        void circile_factory(const trimesh::vec& center, const trimesh::vec3& camera_pos, float radius, int facet_start, int colorIndex, std::vector<int>& dirty_chunks);
        void double_circile_factory(const trimesh::vec& center, const trimesh::vec& second_center, const trimesh::vec3& camera_pos,
            float radius, int facet_start, int colorIndex, std::vector<int>& dirty_chunks);
        void bucket_fill_select_triangles_preview(const trimesh::vec& center, int facet_start, int colorIndex, std::vector<std::vector<trimesh::vec3>>& contour,bool isFill=true);
        void bucket_fill_select_triangles(int colorIndex, std::vector<int>& dirty_chunks);

        //获取序列化数据
        void updateData();
        //解序列化数据
        void updateTriangle();

        void reset();

        std::string get_triangle_as_string(int triangle_idx) const;
        void set_triangle_from_string(int triangle_id, const std::string& str);
        std::vector<std::string> get_data_as_string() const;
        void set_triangle_from_data(std::vector<std::string> strList);
        
        int source_triangle_index(int index);
        int chunkId2FaceId(int chunkId, int index);
    private:
        void get_current_select_contours(std::vector<trimesh::vec3>& contour, const trimesh::vec3& offset = trimesh::vec3());
        void dirty_source_triangles_2_chunks(const std::vector<int>& dirty_source_triangls, std::vector<int>& chunks);
    private:
        std::unique_ptr<Slic3r::TriangleMesh> m_mesh;
        std::unique_ptr <Slic3r::TriangleSelector> m_triangle_selector;
        std::pair<std::vector<std::pair<int, int>>, std::vector<bool>> m_data;

        std::vector<int> m_faceChunkIDs;
        std::vector<std::vector<int>> m_chunkFaces;
    };

}
#endif // MESH_SPREAD_1595984973500_H