/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "SceneObject.h"

SceneObject::SceneObject(const string& name,
                         const shared_ptr<Model>& model,
                         const shared_ptr<Texture>& texture,
                         const shared_ptr<Shader>& shader)
{
    this->name = name;
    this->model = model;
    this->texture = texture;
    this->shader = shader;
    this->backfaceCullingEnabled = true;
}

Sphere SceneObject::GetWorldBoundingSphere()
{
    Sphere ret;
    const Vec3 scale = transform.GetScale();
    ret.center = Vec4(model->bsphere.center, 1) * transform.GetMatrix();
    ret.radius = max(scale.x, max(scale.y, scale.z)) * model->bsphere.radius;
    return ret;
}
