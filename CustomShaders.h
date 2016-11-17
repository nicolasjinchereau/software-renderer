/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Math.h"
#include "Camera.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Light.h"
#include <memory>
#include <vector>

// a pixel lit shader used for rendering all models
class LitShader : public Shader
{
public:
    Scene* scene = nullptr;
    Texture* texture;
    Mat4 mtxModel = Mat4::identity;
    Mat4 mtxMVP = Mat4::identity;
    Mat4 mtxNormal = Mat4::identity;
    Vec3 eyePos = Vec3::zero;
    Vec3 eyeDir = Vec3::zero;
    bool enableLighting = true;
    
    virtual void Accept(ShaderVisitor* visitor) override {
        visitor->Visit(this);
    }

    virtual void Prepare(Scene* scene, SceneObject* obj) override
    {
        this->scene = scene;
        texture = obj->texture.get();
        mtxModel = obj->transform.GetMatrix();
        mtxMVP = mtxModel * scene->camera->GetVPMatrix();
        mtxNormal = obj->transform.GetInverseMatrix().Transposed();
        eyePos = scene->camera->transform.GetPosition();
        eyeDir = Vec3::forward * scene->camera->transform.GetRotation();
        int channels = texture->channels();
    }

    virtual Vertex ProcessVertex(const Vertex &in) override
    {
        Vertex out;
        out.position = Vec4(in.position, 1.0f) * mtxMVP;
        out.normal = Vec4(in.normal, 1.0f) * mtxNormal;
        out.texcoord = in.texcoord;
        out.worldPos = Vec4(in.worldPos, 1.0f) * mtxModel;
        return out;
    }

    virtual Color ProcessPixel(const Vertex &in, float mipLevel) override
    {
        Color tex = texture->GetPixel(in.texcoord, mipLevel);
        
        if(enableLighting)
        {
            if(texture->channels() == 4 && tex.a > 0.5f)
                return tex;

            Color lum = Color::black;

            for(auto& light : scene->lights)
                lum += light->Apply(in.worldPos, in.normal, eyePos, eyeDir);

            return tex * lum;
        }
        else
        {
            return tex;
        }
    }
};

// a self-illuminated shader used to render the sky
class UnlitShader : public Shader
{
public:
    Texture* texture;
    Mat4 mtxMVP;

    virtual void Accept(ShaderVisitor* visitor) override {
        visitor->Visit(this);
    }

    virtual void Prepare(Scene* scene, SceneObject* obj) override
    {
        texture = obj->texture.get();
        mtxMVP = obj->transform.GetMatrix() * scene->camera->GetVPMatrix();
    }

    virtual Vertex ProcessVertex(const Vertex &in) override
    {
        Vertex out;
        out.position = Vec4(in.position, 1.0f) * mtxMVP;
        out.texcoord = in.texcoord;
        return out;
    }

    virtual Color ProcessPixel(const Vertex &in, float mipLevel) override {
        return texture->GetPixel(in.texcoord, mipLevel);
    }
};
