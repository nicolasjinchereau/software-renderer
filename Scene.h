/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "SceneObject.h"
#include "Light.h"
#include <vector>
#include <memory>

class Scene
{
public:
    vector<shared_ptr<SceneObject>> objects;
    vector<shared_ptr<Light>> lights;
    shared_ptr<Camera> camera;

    void ApplySettings(const string& filename);
    shared_ptr<SceneObject> FindObject(const string& name);
    shared_ptr<Light> FindLight(const string& name);
};
