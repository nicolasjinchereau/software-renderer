/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cassert>
#include "poly_vector.h"

class Shader;
class Scene;
class SceneObject;
class Vertex;

typedef poly_vector<Shader, AlignedAllocator<uint8_t, 16>> ShaderList;

class alignas(16) Shader
{
public:
    virtual ~Shader(){}
    virtual void CopyTo(ShaderList& copies) = 0;
    virtual void Prepare(Scene* scene, SceneObject* obj) = 0;
    virtual Vertex ProcessVertex(const Vertex &in) = 0;
    virtual Color ProcessPixel(const Vertex &in, float mipLevel, bool& discard) = 0;
};
