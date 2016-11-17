/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Model.h"

Model::Model(const string &filename)
{
    LoadFromFBXFile(filename);
}

Model::~Model()
{
    
}

void Model::LoadFromFBXFile(const string &filename)
{
    FbxManager *sdkManager = NULL;
    FbxImporter* importer = NULL;
    FbxScene* scene = NULL;

    try
    {
        sdkManager = FbxManager::Create();
        sdkManager->SetIOSettings( FbxIOSettings::Create(sdkManager, IOSROOT) );
        importer = FbxImporter::Create(sdkManager, "");

        if(!importer->Initialize(filename.c_str(), -1, sdkManager->GetIOSettings()))
            throw "Failed to initialize the FBX importer";
        
        scene = FbxScene::Create(sdkManager, "scene");
        bool didImport = importer->Import(scene);
        
        if(!importer->Import(scene))
            throw "Failed to import the FBX scene";

        FbxNode* rootNode = scene->GetRootNode();

        bool foundMesh = false;
        int nChildren = rootNode->GetChildCount();

        for(int i = 0; i < nChildren; i++)
        {
            if(LoadFirstMeshNode_R(sdkManager, rootNode->GetChild(i)))
            {
                foundMesh = true;
                break;
            }
        }

        if(!foundMesh)
            throw "Mesh node not found.";

        RecalcBounds();
    }
    catch(const char *exception)
    {
        cout << "Failed to load FBX file: " << exception << endl;
    }

    if(importer)    importer->Destroy();
    if(scene)       scene->Destroy();
    if(sdkManager)  sdkManager->Destroy();
}

bool Model::LoadFirstMeshNode_R(FbxManager *sdkManager, FbxNode *pNode)
{
    for(int a = 0; a < pNode->GetNodeAttributeCount(); a++)
    {
        FbxNodeAttribute *attrib = pNode->GetNodeAttributeByIndex(a);
        FbxNodeAttribute::EType type = attrib->GetAttributeType();

        if(type == FbxNodeAttribute::eMesh)
        {
            FbxAMatrix globalTransform = pNode->EvaluateGlobalTransform();
            
            defaultTransfrom.SetPosition( FromFbxType(globalTransform.GetT()) );
            defaultTransfrom.SetScale( FromFbxType(globalTransform.GetS()) );
            defaultTransfrom.SetRotation( FromFbxType(globalTransform.GetQ()) );

            LoadMeshData((FbxMesh*)attrib);

            return true;
        }
    }

    int nChildren = pNode->GetChildCount();

    for(int i = 0; i < nChildren; i++)
    {
        if(LoadFirstMeshNode_R(sdkManager, pNode->GetChild(i)))
            return true;
    }

    return false;
}

void Model::LoadMeshData(FbxMesh *fbxMesh)
{
    FbxVector4 *fbxVertices = fbxMesh->GetControlPoints();
    int triCount = fbxMesh->GetPolygonCount();
    int fbxVertCount = fbxMesh->GetControlPointsCount();

    vertices.reserve(triCount * 3);

    FbxStringList uvSetNames;
    fbxMesh->GetUVSetNames(uvSetNames);
    FbxLayerElementUV *uvLayer = fbxMesh->GetLayer(0)->GetUVs();
    
    for(int p = 0; p < triCount; ++p)
    {
        for(int v = 0; v < 3; ++v)
        {
            // Get vertex position
            int vertIndex = fbxMesh->GetPolygonVertex(p, v);
            Vec3 vertex = FromFbxType(fbxVertices[vertIndex]);
            
            // Get normal
            FbxVector4 fbxNormal;
            fbxMesh->GetPolygonVertexNormal(p, v, fbxNormal);
            fbxNormal.Normalize();
            Vec3 normal = FromFbxType(fbxNormal);

            // Get texture coordinate
            FbxVector2 fbxTexcoord(0, 0);
            
            if(uvLayer)
            {
                switch(uvLayer->GetMappingMode())
                {
                case FbxLayerElement::eByControlPoint:
                    fbxTexcoord = uvLayer->mDirectArray->GetAt(vertIndex);
                    break;

                case FbxLayerElement::eByPolygonVertex:
                    const char *setName = uvSetNames[0];
                    bool unmapped = false;
                    fbxMesh->GetPolygonVertexUV(p, v, setName, fbxTexcoord, unmapped);
                    break;
                }
            }

            Vec2 texcoord = FromFbxType(fbxTexcoord);

            vertices.emplace_back(vertex, normal, texcoord);
        }
    }
}

void Model::RecalcBounds()
{
    size_t vertCount = vertices.size();
    if(vertCount > 0)
    {
        Vec3 center = vertices[0].position;
        bbox.vmin = vertices[0].position;
        bbox.vmax = vertices[0].position;

        for(uint32_t v = 1; v < vertCount; v++)
        {
            bbox.vmin.x = min(bbox.vmin.x, vertices[v].position.x);
            bbox.vmin.y = min(bbox.vmin.y, vertices[v].position.y);
            bbox.vmin.z = min(bbox.vmin.z, vertices[v].position.z);
            bbox.vmax.x = max(bbox.vmax.x, vertices[v].position.x);
            bbox.vmax.y = max(bbox.vmax.y, vertices[v].position.y);
            bbox.vmax.z = max(bbox.vmax.z, vertices[v].position.z);

            center += vertices[v].position;
        }

        center /= (float)vertCount;

        float radius = 0.0f;

        for(uint32_t v = 0; v < vertCount; v++)
        {
            radius = max(radius, (vertices[v].position - center).LengthSq());
        }

        if(radius > 0.0f)
        {
            radius = sqrt(radius);
        }

        bsphere = Sphere(center, radius);
    }
}

Vec3 Model::FromFbxType(const FbxVector4 &v)
{
    return Vec3((float)v[0] * 0.01f, (float)v[2] * 0.01f, (float)v[1] * 0.01f);
}

Vec2 Model::FromFbxType(const FbxVector2 &v)
{
    return Vec2((float)v[0], 1.0f - (float)v[1]);
}

Quat Model::FromFbxType(const FbxQuaternion &q)
{
    return Quat((float)q[0], (float)q[1], (float)q[2], (float)q[3]);
}

Mat4 Model::FromFbxType(const FbxAMatrix &m)
{
    FbxVector4    p = m.GetT();
    FbxVector4    s = m.GetS();
    FbxQuaternion q = m.GetQ();
    
    return Mat4::Transform(Vec3((float)p[0], (float)p[1], (float)p[2]),
                           Vec3((float)s[0], (float)s[1], (float)s[2]),
                           Quat((float)q[0], (float)q[1], (float)q[2], (float)q[3]));
}
