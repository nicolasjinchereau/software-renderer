/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Camera.h"

Camera::Camera()
{
    _fov = 70.0f;
    _aspect = 4.0f / 3.0f;
    _nearPlane = 0.1f;
    _farPlane = 1000.0f;
    
    _updateProjection = true;
    _updateView = true;

    transform.AddObserver(this);
}

Camera::Camera(float fov, float aspect, float zNear, float zFar)
{
    _fov = fov;
    _aspect = aspect;
    _nearPlane = zNear;
    _farPlane = zFar;
    
    _updateProjection = true;
    _updateView = true;

    transform.AddObserver(this);
}

void Camera::SetFieldOfView(float fov)
{
    _fov = fov;
    _updateProjection = true;
}

void Camera::SetAspectRatio(float aspect)
{
    _aspect = aspect;
    _updateProjection = true;
}

void Camera::SetNearPlane(float zNear)
{
    _nearPlane = zNear;
    _updateProjection = true;
}

void Camera::SetFarPlane(float zFar)
{
    _farPlane = zFar;
    _updateProjection = true;
}

void Camera::OnTransformChanged(Transform *sender)
{
    _updateView = true;
}

const Mat4 &Camera::GetVPMatrix() const
{
    Update();
    return _mtxVP;
}

const Mat4 &Camera::GetProjectionMatrix() const
{
    Update();
    return _mtxProjection;
}

void Camera::Update() const
{
    if(_updateProjection)
    {
        _mtxProjection = Mat4::Project3D(_fov, _aspect, _nearPlane, _farPlane);
    }
    
    if(_updateView || _updateProjection)
    {
        _mtxVP = transform.GetInverseMatrix() * _mtxProjection;

        _frustum[FP_LEFT].a = _mtxVP.m14 + _mtxVP.m11;
        _frustum[FP_LEFT].b = _mtxVP.m24 + _mtxVP.m21;
        _frustum[FP_LEFT].c = _mtxVP.m34 + _mtxVP.m31;
        _frustum[FP_LEFT].d = _mtxVP.m44 + _mtxVP.m41;
        _frustum[FP_LEFT].Normalize();

        _frustum[FP_RIGHT].a = _mtxVP.m14 - _mtxVP.m11;
        _frustum[FP_RIGHT].b = _mtxVP.m24 - _mtxVP.m21;
        _frustum[FP_RIGHT].c = _mtxVP.m34 - _mtxVP.m31;
        _frustum[FP_RIGHT].d = _mtxVP.m44 - _mtxVP.m41;
        _frustum[FP_RIGHT].Normalize();

        _frustum[FP_TOP].a = _mtxVP.m14 - _mtxVP.m12;
        _frustum[FP_TOP].b = _mtxVP.m24 - _mtxVP.m22;
        _frustum[FP_TOP].c = _mtxVP.m34 - _mtxVP.m32;
        _frustum[FP_TOP].d = _mtxVP.m44 - _mtxVP.m42;
        _frustum[FP_TOP].Normalize();

        _frustum[FP_BOTTOM].a = _mtxVP.m14 + _mtxVP.m12;
        _frustum[FP_BOTTOM].b = _mtxVP.m24 + _mtxVP.m22;
        _frustum[FP_BOTTOM].c = _mtxVP.m34 + _mtxVP.m32;
        _frustum[FP_BOTTOM].d = _mtxVP.m44 + _mtxVP.m42;
        _frustum[FP_BOTTOM].Normalize();

        _frustum[FP_NEAR].a = /* _mtxVP.m14 + */ _mtxVP.m13;
        _frustum[FP_NEAR].b = /* _mtxVP.m24 + */ _mtxVP.m23;
        _frustum[FP_NEAR].c = /* _mtxVP.m34 + */ _mtxVP.m33;
        _frustum[FP_NEAR].d = /* _mtxVP.m44 + */ _mtxVP.m43;
        _frustum[FP_NEAR].Normalize();

        _frustum[FP_FAR].a = _mtxVP.m14 - _mtxVP.m13;
        _frustum[FP_FAR].b = _mtxVP.m24 - _mtxVP.m23;
        _frustum[FP_FAR].c = _mtxVP.m34 - _mtxVP.m33;
        _frustum[FP_FAR].d = _mtxVP.m44 - _mtxVP.m43;
        _frustum[FP_FAR].Normalize();
    }

    _updateView = false;
    _updateProjection = false;
}

bool Camera::CanSee(const Sphere &bounds)
{
    Update();

    if(bounds.radius < FLT_EPSILON)
        return false;

    for(int p = 0; p < 6; p++)
    {
        if(_frustum[p].InBack(bounds))
            return false;
    }

    return true;
}
