/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "Math.h"
#include "Transform.h"

class Camera : public ITransformObserver
{
public:
    enum FrustumPlanes
    {
        FP_LEFT,
        FP_RIGHT,
        FP_TOP,
        FP_BOTTOM,
        FP_NEAR,
        FP_FAR,
    };

    Transform transform;

    Camera();
    Camera(float fov, float aspect, float zNear, float zFar);

    void SetFieldOfView(float fov);
    void SetAspectRatio(float aspect);
    void SetNearPlane(float zNear);
    void SetFarPlane(float zFar);
    
    virtual void OnTransformChanged(Transform *sender);

    const Mat4 &GetVPMatrix() const;
    const Mat4 &GetProjectionMatrix() const;
    bool CanSee(const Sphere &bounds);

private:
    Mat4 _matrix;
    float _fov;
    float _aspect;
    float _nearPlane;
    float _farPlane;
    
    mutable bool _updateProjection;
    mutable bool _updateView;
    mutable Mat4 _mtxProjection;
    mutable Mat4 _mtxVP;
    mutable Plane _frustum[6];

    void Update() const;
};
