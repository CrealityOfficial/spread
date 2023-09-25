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

        m_mesh.reset(trimesh2Slic3rTriangleMesh(mesh, tracer));
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

    trimesh::TriMesh* MeshSpreadWrapper::cursor_factory(const trimesh::vec& center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
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

        trimesh::TriMesh* mesh = new trimesh::TriMesh();
        triangle_selector2trimesh(mesh, m_triangle_selector.get());
        return mesh;
    }

    trimesh::TriMesh* MeshSpreadWrapper::cursor_factory(const trimesh::vec& first_center, const trimesh::vec& second_center, const trimesh::vec& camera_pos, const float& cursor_radius, const CursorType& cursor_type, const trimesh::fxform& trafo_matrix, const ClippingPlane& clipping_plane)
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

        //static std::unique_ptr<Cursor> cursor_factory(const Vec3f & first_center, const Vec3f & second_center, const Vec3f & camera_pos, const float cursor_radius, const CursorType cursor_type, const Transform3d & trafo_matrix, const ClippingPlane & clipping_plane)
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

        trimesh::TriMesh* mesh = new trimesh::TriMesh();
        triangle_selector2trimesh(mesh, m_triangle_selector.get());
        return mesh;
    }
}