/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <memory>
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
using namespace std;

enum class CullMode
{
    None,
    Back,
    Front
};

class alignas(16) SceneObject
{
public:
    string name;
    Transform transform;
    shared_ptr<Model> model;
    shared_ptr<Texture> texture;
    shared_ptr<Shader> shader;
    CullMode cullMode;

    SceneObject(const string& name,
                const shared_ptr<Model>& model,
                const shared_ptr<Texture>& texture,
                const shared_ptr<Shader>& shader,
                CullMode cullMode = CullMode::Back);

    Sphere GetWorldBoundingSphere();
};
