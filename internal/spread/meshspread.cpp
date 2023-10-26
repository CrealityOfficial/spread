#include "spread/meshspread.h"
#include "util.h"
#include "Slice3rBase/TriangleMesh.hpp"
#include "Slice3rBase/TriangleSelector.hpp"
#include "msbase/mesh/chunk.h"

#define MAX_RADIUS 8
#define  PI 3.141592 

namespace spread
{

    MeshSpreadWrapper::MeshSpreadWrapper()
    {

    }

    MeshSpreadWrapper::~MeshSpreadWrapper()
    {
        m_chunkFaces.clear();
        m_faceChunkIDs.clear();
        //m_data.swap(std::pair<std::vector<std::pair<int, int>>, std::vector<bool>>());
    }


    double count_triangle_area(trimesh::vec& a, trimesh::vec& b, trimesh::vec& c) {
        double area = -1;

        double side[3];//存储三条边的长度;

        side[0] = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
        side[1] = sqrt(pow(a.x - c.x, 2) + pow(a.y - c.y, 2) + pow(a.z - c.z, 2));
        side[2] = sqrt(pow(c.x - b.x, 2) + pow(c.y - b.y, 2) + pow(c.z - b.z, 2));

        //不能构成三角形;
        if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0]) return area;

        //利用海伦公式。s=sqr(p*(p-a)(p-b)(p-c)); 
        double p = (side[0] + side[1] + side[2]) / 2; //半周长;
        area = sqrt(p * (p - side[0]) * (p - side[1]) * (p - side[2]));

        return area;
    }


    void MeshSpreadWrapper::setInputs(trimesh::TriMesh* mesh, ccglobal::Tracer* tracer)
    {
        if (mesh == nullptr)
            return;

        m_chunkFaces.clear();
        m_faceChunkIDs.clear();
        //m_data.swap(std::pair<std::vector<std::pair<int, int>>, std::vector<bool>>());

        if (m_triangle_selector != nullptr)
             m_triangle_selector->reset();

        m_mesh.reset(simpleConvert(mesh, tracer));
        m_triangle_selector.reset(new Slic3r::TriangleSelector(*m_mesh));

        const std::vector<Slic3r::Vec3i>& neigbs = m_triangle_selector->originNeighbors();
        std::vector<trimesh::ivec3> _neigbs(neigbs.size());
        for (int i=0;i< neigbs.size();i++)
        {
            _neigbs[i] = trimesh::ivec3(neigbs[i].x(), neigbs[i].y(), neigbs[i].z());
        }
        const float max_area = 1.0 * PI * MAX_RADIUS * MAX_RADIUS * 5;
        msbase::generateChunk( mesh, _neigbs, max_area,m_faceChunkIDs,m_chunkFaces);

        return;

        ////split chunks
        ////const std::vector<Slic3r::Vec3i>& neigbs = m_triangle_selector->originNeighbors();
        //int faceCount = mesh->faces.size();
        //assert(faceCount == (int)neigbs.size());

        //int chunkCount = 50;
        //if (faceCount < chunkCount)
        //    chunkCount = faceCount;

        //int currentChunk = 0;
        //m_chunkFaces.resize(chunkCount);
        //m_faceChunkIDs.resize(faceCount, -1);

        ////const float max_area = 1.0 * PI * MAX_RADIUS * MAX_RADIUS * 5;
        //float area = 0.0f;
        //int chunkSize = faceCount / chunkCount;
        //for (int i = 0; i < faceCount; ++i)
        //{
        //    if (m_faceChunkIDs.at(i) >= 0)
        //        continue;

        //    std::set<int> seeds;
        //    seeds.insert(i);
        //    std::set<int> next_seeds;

        //    while (!seeds.empty())
        //    {
        //        std::vector<int>& chunks = m_chunkFaces.at(currentChunk);

        //        for (int s : seeds)
        //        {
        //            const Slic3r::Vec3i& nei = neigbs.at(s);
        //            chunks.push_back(s);

        //            area += count_triangle_area(mesh->vertices[mesh->faces[s].x]
        //                , mesh->vertices[mesh->faces[s].y]
        //                , mesh->vertices[mesh->faces[s].z]);

        //            m_faceChunkIDs.at(s) = currentChunk;

        //            for (int j = 0; j < 3; ++j)
        //            {
        //                if (nei[j] >= 0 && m_faceChunkIDs.at(nei[j]) < 0)
        //                    next_seeds.insert(nei[j]);
        //            }
        //        }

        //        if (/*(chunks.size() > chunkSize)*/  (next_seeds.empty() || area > max_area) && (currentChunk < chunkCount - 1))
        //        {
        //            currentChunk++;
        //            next_seeds.clear();
        //            area = 0.0f;
        //        }

        //        next_seeds.swap(seeds);
        //        next_seeds.clear();
        //    }
        //}

        //for (size_t i = 0; i < m_chunkFaces.size(); i++)
        //{
        //    trimesh::TriMesh _mesh;
        //    _mesh.vertices = mesh->vertices;
        //    for (size_t j = 0; j < m_chunkFaces[i].size(); j++)
        //    {
        //        _mesh.faces.push_back(mesh->faces[m_chunkFaces[i][j]]);
        //    }
        //    std::string str = "d:/_" + std::to_string(i) + ".stl";
        //    _mesh.write(str);
        //}
    }

    void MeshSpreadWrapper::testChunk()
    {
        int faceCount = m_mesh->facets_count();
        //assert(faceCount == (int)neigbs.size());

        int chunkCount = (int)m_chunkFaces.size();
        for (int i = 0; i < chunkCount; ++i)
        {
            const std::vector<int>& faces = m_chunkFaces.at(i);
            for(int j : faces)
                m_triangle_selector->set_facet(j, (Slic3r::EnforcerBlockerType)(i % 8));
        }
    }

    int MeshSpreadWrapper::chunkCount()
    {
        return (int)m_chunkFaces.size();
    }

    void MeshSpreadWrapper::chunk(int index, std::vector<trimesh::vec3>& positions, std::vector<int>& flags, std::vector<int>& splitIndices)
    {
        assert(index >= 0 && index < m_chunkFaces.size());
        indexed_triangle_set indexed;
        m_triangle_selector->get_chunk_facets(index, m_faceChunkIDs, indexed, flags, splitIndices);

        indexed2TriangleSoup(indexed, positions);
    }

    int isInMesh(const trimesh::vec& center, const trimesh::vec& normal)
    {
        return 0;
    }

    //void MeshSpreadWrapper::triangle_factory(int facet_start, int colorIndex, const CursorType& cursor_type)
    //{
    //    Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
    //    m_triangle_selector->set_facet(facet_start, new_state);

    //    updateData();
    //}

    void MeshSpreadWrapper::triangle(int facet, int colorIndex, std::vector<int>& dirty_chunks)
    {
        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        m_triangle_selector->set_facet(facet, new_state);

        std::vector<int> dirty_source_triangles;
        m_triangle_selector->clear_dirty_source_triangles(dirty_source_triangles);
        dirty_source_triangles_2_chunks(dirty_source_triangles, dirty_chunks);
    }

    void MeshSpreadWrapper::triangle_selector2trimesh(trimesh::TriMesh* mesh, Slic3r::TriangleSelector* triangle_selector)
    {
        std::vector<stl_vertex> vertexs;
        std::vector<std::array<int, 5>> facets;
        triangle_selector->getVectors(vertexs);
        triangle_selector->getFacets(facets);

        mesh->vertices.reserve(vertexs.size());
        mesh->faces.reserve(facets.size());
        mesh->flags.reserve(facets.size());
        for (auto ver : vertexs)
        {
            mesh->vertices.push_back(trimesh::vec3(ver.x(), ver.y(), ver.z()));
        }
        for (auto ver : facets)
        {
            mesh->faces.push_back(trimesh::TriMesh::Face(ver[0], ver[1], ver[2]));
            mesh->flags.push_back(ver[3]);
        }

        //m_data = triangle_selector->serialize();
    }

    //void MeshSpreadWrapper::cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, 
    //    const float& cursor_radius, const CursorType& cursor_type, 
    //    const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
    //{
    //    Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
    //    Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
    //    //_clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
    //    //_clipping_plane.offset = clipping_plane.offset;

    //    bool triangle_splitting_enabled = true;

    //    Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

    //    Slic3r::Transform3d trafo_no_translate = Slic3r::Transform3d::Identity();

    //    Slic3r::Transform3d _matrix;
    //    for (int i = 0; i < 4; ++i)
    //    {
    //        for (int j = 0; j < 4; ++j)
    //        {
    //            _matrix(i, j) = trafo_matrix(i, j);
    //        }
    //    }

    //    std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::SinglePointCursor::cursor_factory(Slic3r::Vec3f(center),
    //        Slic3r::Vec3f(camera_pos), cursor_radius,
    //        _cursor_type, _matrix, _clipping_plane);

    //    m_triangle_selector->select_patch(int(clipping_plane.facet_idx), std::move(cursor), new_state, trafo_no_translate,
    //        triangle_splitting_enabled);

    //    updateData();
    //}

    //void MeshSpreadWrapper::cursor_factory(const trimesh::vec& first_center, const trimesh::vec& second_center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
    //{
    //    Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
    //    Slic3r::Transform3d _matrix;
    //    for (int i = 0; i < 4; ++i)
    //    {
    //        for (int j = 0; j < 4; ++j)
    //        {
    //            _matrix(i, j) = trafo_matrix(i, j);
    //        }
    //    }
    //    Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
    //    _clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
    //    _clipping_plane.offset = clipping_plane.offset;

    //    Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

    //    bool triangle_splitting_enabled = true;

    //    std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::DoublePointCursor::cursor_factory(
    //        Slic3r::Vec3f(first_center),
    //        Slic3r::Vec3f(second_center),
    //        Slic3r::Vec3f(camera_pos),
    //        cursor_radius,
    //        _cursor_type,
    //        _matrix, _clipping_plane);

    //    m_triangle_selector->select_patch(int(clipping_plane.facet_idx), std::move(cursor), new_state, _matrix,
    //        triangle_splitting_enabled);
    //}

    void MeshSpreadWrapper::circile_factory(const trimesh::vec& center, const trimesh::vec3& camera_pos, float radius, int facet_start, int colorIndex, std::vector<int>& dirty_chunks)
    {
        Slic3r::Vec3f cursor_center(center.x, center.y, center.z);
        Slic3r::Vec3f source(camera_pos.x, camera_pos.y, camera_pos.z);
        float radius_world = radius;
        Slic3r::Transform3d trafo_no_translate = Slic3r::Transform3d::Identity();
        Slic3r::TriangleSelector::ClippingPlane clipping_plane;

        std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::Circle::cursor_factory(cursor_center,
            source, radius_world, Slic3r::TriangleSelector::CursorType::CIRCLE, trafo_no_translate, clipping_plane);

        bool triangle_splitting_enabled = true;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        //Slic3r::Transform3d trafo_no_translate = Slic3r::Transform3d::Identity();

        m_triangle_selector->select_patch(facet_start, std::move(cursor), new_state, trafo_no_translate,
            triangle_splitting_enabled);

        std::vector<int> dirty_source_triangles;
        m_triangle_selector->clear_dirty_source_triangles(dirty_source_triangles);
        dirty_source_triangles_2_chunks(dirty_source_triangles, dirty_chunks);
    }

    void MeshSpreadWrapper::double_circile_factory(const trimesh::vec& center, const trimesh::vec& second_center, const trimesh::vec3& camera_pos,
        float radius, int facet_start, int colorIndex, std::vector<int>& dirty_chunks)
    {
        Slic3r::Vec3f cursor_center(center.x, center.y, center.z);
        Slic3r::Vec3f second_cursor_center(second_center.x, second_center.y, second_center.z);
        Slic3r::Vec3f source(camera_pos.x, camera_pos.y, camera_pos.z);
        float radius_world = radius;
        Slic3r::Transform3d trafo = Slic3r::Transform3d::Identity();
        Slic3r::TriangleSelector::ClippingPlane clipping_plane;

        std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::DoublePointCursor::cursor_factory(cursor_center, second_cursor_center,
            source, radius_world, Slic3r::TriangleSelector::CursorType::CIRCLE, trafo, clipping_plane);

        bool triangle_splitting_enabled = true;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        Slic3r::Transform3d trafo_no_translate = Slic3r::Transform3d::Identity();

        m_triangle_selector->select_patch(facet_start, std::move(cursor), new_state, trafo_no_translate,
            triangle_splitting_enabled);

        std::vector<int> dirty_source_triangles;
        m_triangle_selector->clear_dirty_source_triangles(dirty_source_triangles);
        dirty_source_triangles_2_chunks(dirty_source_triangles, dirty_chunks);
    }

    void MeshSpreadWrapper::updateData()
    {
        m_data.first.shrink_to_fit();
        m_data.second.shrink_to_fit();
        m_data = m_triangle_selector->serialize();
    }

    //void MeshSpreadWrapper::bucket_fill_select_triangles(const trimesh::vec& center, const ClippingPlane& clipping_plane, const CursorType& cursor_type)
    //{
    //    Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
    //    float seed_fill_angle = 30.f;
    //    bool propagate = true;
    //    bool force_reselection = true;

    //    Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
    //    _clipping_plane.normal = Slic3r::Vec3f(0.f,0.f,1.f);

    //    Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

    //    m_triangle_selector->seed_fill_apply_on_triangles(new_state);

    //    updateData();
    //}

    void MeshSpreadWrapper::bucket_fill_select_triangles(const trimesh::vec& center, int colorIndex, std::vector<int>& dirty_chunks)
    {
        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        m_triangle_selector->seed_fill_apply_on_triangles(new_state);

        std::vector<int> dirty_source_triangles;
        m_triangle_selector->clear_dirty_source_triangles(dirty_source_triangles);
        dirty_source_triangles_2_chunks(dirty_source_triangles, dirty_chunks);
    }

    void MeshSpreadWrapper::bucket_fill_select_triangles_preview(const trimesh::vec& center, int facet_start, int colorIndex, std::vector<std::vector<trimesh::vec3>>& contour, bool isFill)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(Slic3r::TriangleSelector::CursorType::GAP_FILL); 
        float seed_fill_angle = 30.0f;
        bool propagate = true;
        bool force_reselection = true;

        if (!isFill)
        {
            seed_fill_angle = -1.0f;
            propagate = false;
        }

        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        if (facet_start >= 0 && facet_start < m_triangle_selector->getFacetsNum())
        {
            m_triangle_selector->bucket_fill_select_triangles(
                Slic3r::Vec3f(center)
                , facet_start
                , _clipping_plane
                , seed_fill_angle
                , propagate
                , false);

            std::vector<Slic3r::Vec2i> contour_edges = m_triangle_selector->get_seed_fill_contour();
            contour.reserve(contour_edges.size());
            for (const Slic3r::Vec2i& edge : contour_edges) {
                std::vector<trimesh::vec> line;
                int index= edge(0);
                auto vector = m_triangle_selector->getVectors(index);
                line.emplace_back(trimesh::vec3(vector.x(), 
                                                vector.y(), 
                                                vector.z()));

                index = edge(1);
                vector = m_triangle_selector->getVectors(index);
                line.emplace_back(trimesh::vec3(vector.x(), 
                                                vector.y(), 
                                                vector.z()));

                contour.emplace_back(line);
            }
        }
    }

    //void MeshSpreadWrapper::seed_fill_select_triangles_preview1(int facet_start, std::vector<trimesh::vec3>& contour)
    //{
    //    contour.clear();

    //    float seed_fill_angle = 30.f;

    //    Slic3r::Transform3d t;
    //    Slic3r::TriangleSelector::ClippingPlane clipping_plane;

    //    if (facet_start >= 0 && facet_start < m_triangle_selector->getFacetsNum())
    //    {
    //        m_triangle_selector->seed_fill_select_triangles(
    //            Slic3r::Vec3f()
    //            , facet_start
    //            , t
    //            , clipping_plane
    //            , seed_fill_angle
    //            , false);

    //        get_current_select_contours(contour);
    //    }
    //}

    void MeshSpreadWrapper::get_current_select_contours(std::vector<trimesh::vec3>& contour, const trimesh::vec3& offset)
    {
        std::vector<Slic3r::Vec2i> contour_edges = m_triangle_selector->get_seed_fill_contour();
        contour.clear();

        int size = (int)contour_edges.size();
        if (size == 0)
            return;

        contour.resize(2 * size);
        for(int i = 0; i < size; ++i)
        {
            const Slic3r::Vec2i& edge = contour_edges.at(i);
            contour.at(2 * i) = toVector(m_triangle_selector->vertex(edge(0))) + offset;
            contour.at(2 * i + 1) = toVector(m_triangle_selector->vertex(edge(1))) + offset;
        }
    }

    void MeshSpreadWrapper::dirty_source_triangles_2_chunks(const std::vector<int>& dirty_source_triangls, std::vector<int>& chunks)
    {
        chunks.clear();
        int size = (int)m_chunkFaces.size();
        std::vector<bool> chunk_dirty(size, false);
        
        for (int source : dirty_source_triangls)
        {
            chunk_dirty.at(m_faceChunkIDs[source]) = true;
        }

        for (int i = 0; i < size; ++i)
        {
            if (chunk_dirty.at(i))
                chunks.push_back(i);
        }
    }

    std::string MeshSpreadWrapper::get_triangle_as_string(int triangle_idx) const
    {
        std::string out;

        auto triangle_it = std::lower_bound(m_data.first.begin(), m_data.first.end(), triangle_idx, [](const std::pair<int, int>& l, const int r) { return l.first < r; });
        if (triangle_it != m_data.first.end() && triangle_it->first == triangle_idx) {
            int offset = triangle_it->second;
            int end = ++triangle_it == m_data.first.end() ? int(m_data.second.size()) : triangle_it->second;
            while (offset < end) {
                int next_code = 0;
                for (int i = 3; i >= 0; --i) {
                    next_code = next_code << 1;
                    if (offset + i < m_data.second.size())
                        next_code |= int(m_data.second[offset + i]);
                    else 
                        next_code |= int(m_data.second[0]);
                }
                offset += 4;

                assert(next_code >= 0 && next_code <= 15);
                char digit = next_code < 10 ? next_code + '0' : (next_code - 10) + 'A';
                out.insert(out.begin(), digit);
            }
        }
        return out;
    }
    void MeshSpreadWrapper::set_triangle_from_string(int triangle_id, const std::string& str)
    {
        assert(!str.empty());
        //assert(m_data.first.empty() || m_data.first.back().first < triangle_id);
        m_data.first.emplace_back(triangle_id, int(m_data.second.size()));

        m_data.second.reserve(m_data.second.size() + str.size()*4 +1);
        for (auto it = str.crbegin(); it != str.crend(); ++it) {
            const char ch = *it;
            int dec = 0;
            if (ch >= '0' && ch <= '9')
                dec = int(ch - '0');
            else if (ch >= 'A' && ch <= 'F')
                dec = 10 + int(ch - 'A');
            else
                assert(false);

            // Convert to binary and append into code.
            for (int i = 0; i < 4; ++i)
                m_data.second.insert(m_data.second.end(), bool(dec & (1 << i)));         
        }
    }

    void MeshSpreadWrapper::updateTriangle()
    {
        m_data.first.shrink_to_fit();
        m_data.second.shrink_to_fit();
        m_triangle_selector->deserialize(m_data);
    }

    int MeshSpreadWrapper::source_triangle_index(int index)
    {
        return m_triangle_selector->source_triangle(index);
    }

    int MeshSpreadWrapper::chunkId2FaceId(int chunkId, int index)
    {
        if (chunkId>= 0 && chunkId < m_faceChunkIDs.size())
        {
            if (index >=0 && index < m_chunkFaces[chunkId].size())
            {
                return m_chunkFaces[chunkId][index];
            }
        }

        return 0;
    }

    std::vector<std::string> MeshSpreadWrapper::get_data_as_string() const
    {
        std::vector<std::string> data;
        int facets_count = m_mesh->its.indices.size();
        data.resize(facets_count);
        for (int i = 0; i < facets_count; ++i)
        {
            std::string face_data = get_triangle_as_string(i);
            if (face_data.empty())
                continue;

            data[i] = face_data;
        }
        return data;
    }

    void MeshSpreadWrapper::set_triangle_from_data(std::vector<std::string> strList)
    {
        int facets_count = m_mesh->its.indices.size();
        assert(strList.size() == facets_count);
        for (int i = 0; i < facets_count; ++i)
        {
            const std::string& str = strList[i];
            if (!str.empty())
                set_triangle_from_string(i, str);
        }

        updateTriangle();
    }
}