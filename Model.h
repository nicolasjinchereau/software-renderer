/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "Math.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <fbxsdk.h>
#include "Transform.h"
#include "Vertex.h"
#include "Mem.h"

using namespace std;

class Texture;

class Model
{
public:
    vector<Vertex, AlignedAllocator<Vertex, 16>> vertices;
    Transform defaultTransfrom;

    Box bbox;
    Sphere bsphere;

    Model(const string &filename);
    ~Model();

    void RecalcBounds();

private:
    void LoadFromFBXFile(const string &filename);
    bool LoadFirstMeshNode_R(FbxManager *sdkManager, FbxNode *pNode);
    void LoadMeshData(FbxMesh *fbxMesh);

    Vec3 FromFbxType(const FbxVector4 &v);
    Vec2 FromFbxType(const FbxVector2 &v);
    Quat FromFbxType(const FbxQuaternion &q);
    Mat4 FromFbxType(const FbxAMatrix &m);
};
