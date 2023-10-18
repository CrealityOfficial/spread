#ifndef SPREADHEADER_1632383314974_H
#define SPREADHEADER_1632383314974_H

#include "interface.h"
#include "trimesh2/TriMesh.h"
#include "trimesh2/XForm.h"
#include <vector>
#include <memory>
#include <string>

namespace spread
{

    struct SPREAD_API SceneData
    {
        int faceId;
        trimesh::vec center;
        trimesh::vec cameraPos;
        trimesh::vec normal;
        trimesh::fxform pose;
        float radius;
    };

    enum SPREAD_API CursorType {
        POINTER, //����Ƭ 
        CIRCLE,//2dԲ
        GAP_FILL,
        SPHERE,//3d��
        // BBS
        HEIGHT_RANGE,
    };

}


#endif // SPREADHEADER_1632383314974_H