/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "RenderingContext.h"
#include <cassert>

class Scene;
class SceneObject;
class Vertex;
class ShaderVisitor;

class alignas(16) Shader
{
public:
    virtual ~Shader(){}
    virtual void Accept(ShaderVisitor* visitor) = 0;
    virtual void Prepare(Scene* scene, SceneObject* obj) = 0;
    virtual Vertex ProcessVertex(const Vertex &in) = 0;
    virtual Color ProcessPixel(const Vertex &in, float mipLevel) = 0;
};

class ShaderVisitor final
{
    vector<uint8_t, AlignedAllocator<uint8_t, 16>>* pInstanceData;
public:
    ShaderVisitor(vector<uint8_t, AlignedAllocator<uint8_t, 16>>* pInstanceData)
        : pInstanceData(pInstanceData)
    {
        assert(pInstanceData);
    }

    template<class T>
    void Visit(T* shader)
    {
        assert(shader);
        size_t size = pInstanceData->size();
        pInstanceData->resize(size + sizeof(T));
        new (pInstanceData->data() + size) T(*shader);
    }
};
