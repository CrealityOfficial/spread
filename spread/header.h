#ifndef SPREADHEADER_1632383314974_H
#define SPREADHEADER_1632383314974_H

#include "spread/interface.h"
#include "trimesh2/TriMesh.h"
#include "trimesh2/XForm.h"
#include <vector>
#include <memory>
#include <string>

#include "ccglobal/tracer.h"

namespace spread
{

    struct SPREAD_API SceneData
    {
        int faceId;
        trimesh::vec center;
        trimesh::vec cameraPos;
        trimesh::vec normal;
        trimesh::vec rayDir;
        trimesh::fxform pose;
        float radius;
        bool mouseDrag{false};
    };

    enum SPREAD_API CursorType {
        POINTER, //三角片 
        CIRCLE,//2d圆
        GAP_FILL,
        GAP_FILL_PREVIEW,
        SPHERE,//3d球
        // BBS
        HEIGHT_RANGE,
    };
}


#endif // SPREADHEADER_1632383314974_H