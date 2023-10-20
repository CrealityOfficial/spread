#include "meshspread.h"
#include "util.h"
#include "Slice3rBase/TriangleMesh.hpp"
#include "Slice3rBase/TriangleSelector.hpp"

namespace spread
{

    MeshSpreadWrapper::MeshSpreadWrapper()
    {

    }

    MeshSpreadWrapper::~MeshSpreadWrapper()
    {

    }

    void MeshSpreadWrapper::setInputs(trimesh::TriMesh* mesh, ccglobal::Tracer* tracer)
    {
        if (mesh == nullptr)
            return;

        if (m_triangle_selector != nullptr)
             m_triangle_selector->reset();

        m_mesh.reset(trimesh2Slic3rTriangleMesh(mesh, tracer));

        m_triangle_selector.reset(new Slic3r::TriangleSelector(*m_mesh));
    }

    void MeshSpreadWrapper::setColorPlane(const std::vector<trimesh::vec>& color_plane)
    {
        if (!color_plane.empty())
        {
            m_color_plane = color_plane;
        }
    }

    int isInMesh(const trimesh::vec& center, const trimesh::vec& normal)
    {
        return 0;
    }

    void MeshSpreadWrapper::triangle_factory(int facet_start, int colorIndex, const CursorType& cursor_type)
    {
        m_curFacet = facet_start;
        m_curCursor_type = cursor_type;
        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(colorIndex);
        m_triangle_selector->set_facet(facet_start, new_state);

        updateData();
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

    void MeshSpreadWrapper::cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, 
        const float& cursor_radius, const CursorType& cursor_type, 
        const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        //_clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
        //_clipping_plane.offset = clipping_plane.offset;

        bool triangle_splitting_enabled = true;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

        Slic3r::Transform3d trafo_no_translate = Slic3r::Transform3d::Identity();

        Slic3r::Transform3d _matrix;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                _matrix(i, j) = trafo_matrix(i, j);
            }
        }

        std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::SinglePointCursor::cursor_factory(Slic3r::Vec3f(center),
            Slic3r::Vec3f(camera_pos), cursor_radius,
            _cursor_type, _matrix, _clipping_plane);

        m_triangle_selector->select_patch(int(clipping_plane.facet_idx), std::move(cursor), new_state, trafo_no_translate,
            triangle_splitting_enabled);

        updateData();
    }

    void MeshSpreadWrapper::cursor_factory(const trimesh::vec& first_center, const trimesh::vec& second_center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
        Slic3r::Transform3d _matrix;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                _matrix(i, j) = trafo_matrix(i, j);
            }
        }
        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        _clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
        _clipping_plane.offset = clipping_plane.offset;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

        bool triangle_splitting_enabled = true;

        std::unique_ptr<Slic3r::TriangleSelector::Cursor> cursor = Slic3r::TriangleSelector::DoublePointCursor::cursor_factory(
            Slic3r::Vec3f(first_center),
            Slic3r::Vec3f(second_center),
            Slic3r::Vec3f(camera_pos),
            cursor_radius,
            _cursor_type,
            _matrix, _clipping_plane);

        m_triangle_selector->select_patch(int(clipping_plane.facet_idx), std::move(cursor), new_state, _matrix,
            triangle_splitting_enabled);
    }

    void MeshSpreadWrapper::updateData()
    {
        m_data.first.shrink_to_fit();
        m_data.second.shrink_to_fit();
        m_data = m_triangle_selector->serialize();
        //m_triangle_selector->deserialize(m_data);
    }

    void MeshSpreadWrapper::bucket_fill_select_triangles(const trimesh::vec& center, const ClippingPlane& clipping_plane, const CursorType& cursor_type)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
        float seed_fill_angle = 30.f;
        bool propagate = true;
        bool force_reselection = true;

        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        //_clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
        _clipping_plane.normal = Slic3r::Vec3f(0.f,0.f,1.f);
        //_clipping_plane.offset = 0.0f;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);


        //m_triangle_selector->bucket_fill_select_triangles(
        //    Slic3r::Vec3f(center)
        //    , clipping_plane.facet_idx
        //    , _clipping_plane
        //    , seed_fill_angle
        //    , propagate
        //    , false);


        m_triangle_selector->seed_fill_apply_on_triangles(new_state);

        //m_triangle_selector->bucket_fill_select_triangles(
        //    Slic3r::Vec3f(center)
        //    , clipping_plane.facet_idx
        //    , _clipping_plane
        //    , seed_fill_angle
        //    , propagate
        //    , force_reselection);

        updateData();
    }

    void MeshSpreadWrapper::bucket_fill_select_triangles_preview(const trimesh::vec& center, const ClippingPlane& clipping_plane, const trimesh::vec& rayDir, std::vector<std::vector<trimesh::vec3>>& contour, const CursorType& cursor_type)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
        float seed_fill_angle = 30.f;
        bool propagate = true;
        bool force_reselection = true;

        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        //_clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
        _clipping_plane.normal = Slic3r::Vec3f(0.f, 0.f, 1.f);
        //_clipping_plane.offset = 0.0f;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

        if (clipping_plane.facet_idx >= 0 && clipping_plane.facet_idx < m_triangle_selector->getFacetsNum())
        {
            m_triangle_selector->bucket_fill_select_triangles(
                Slic3r::Vec3f(center)
                , clipping_plane.facet_idx
                , _clipping_plane
                , seed_fill_angle
                , propagate
                , false);


            trimesh::vec offset = rayDir * -1;
            std::vector<Slic3r::Vec2i> contour_edges = m_triangle_selector->get_seed_fill_contour();
            contour.reserve(contour_edges.size());
            for (const Slic3r::Vec2i& edge : contour_edges) {
                std::vector<trimesh::vec> line;
                int index= edge(0);
                auto vector = m_triangle_selector->getVectors(index);
                line.emplace_back(trimesh::vec3(vector.x() + offset.x, 
                                                vector.y() + offset.y, 
                                                vector.z() + offset.z));

                index = edge(1);
                vector = m_triangle_selector->getVectors(index);
                line.emplace_back(trimesh::vec3(vector.x() + offset.x, 
                                                vector.y() + offset.y, 
                                                vector.z() + offset.z));

                contour.emplace_back(line);
            }
        }
    }

    trimesh::TriMesh* MeshSpreadWrapper::getTrimesh(const TrimeshType& type)
    {
        if (m_color_plane.empty())
        {
            return nullptr;
        }

        trimesh::TriMesh* triMesh = new trimesh::TriMesh();
        triangle_selector2trimesh(triMesh, m_triangle_selector.get());

        if (type == TrimeshType::ALL)
        {
            m_color_plane;
            triMesh->colors.clear();
            triMesh->colors.reserve(triMesh->faces.size());
            for (int i = 0; i < triMesh->faces.size(); i++)
            {
                triMesh->colors.push_back(m_color_plane[triMesh->flags.at(i)% m_color_plane.size()]);
            }
        }
        return triMesh;
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