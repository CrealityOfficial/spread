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

        m_triangle_selector->deserialize(m_triangle_selector->serialize());
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
    }

    void MeshSpreadWrapper::cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
    {
        Slic3r::TriangleSelector::CursorType _cursor_type = Slic3r::TriangleSelector::CursorType(cursor_type);
        Slic3r::TriangleSelector::ClippingPlane _clipping_plane;
        _clipping_plane.normal = Slic3r::Vec3f(clipping_plane.normal);
        _clipping_plane.offset = clipping_plane.offset;

        bool triangle_splitting_enabled = true;

        Slic3r::EnforcerBlockerType new_state = Slic3r::EnforcerBlockerType(clipping_plane.extruderIndex);

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

        m_triangle_selector->select_patch(int(clipping_plane.facet_idx), std::move(cursor), new_state, _matrix,
            triangle_splitting_enabled);

       m_triangle_selector->deserialize(m_triangle_selector->serialize());  
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

        m_triangle_selector->deserialize(m_triangle_selector->serialize());

    }

    trimesh::TriMesh* MeshSpreadWrapper::getTrimesh(TrimeshType type)
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
}