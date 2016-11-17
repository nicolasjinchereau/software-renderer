/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Transform.h"

Transform::Transform()
    : _position(Vec3::zero),
    _scale(1, 1, 1),
    _rotation(Quat::identity),
    _matrix(Mat4::identity),
    _inverseMatrix(Mat4::identity),
    _matrixDirty(false),
    _inverseDirty(false)
{
    
}

void Transform::SetPosition(const Vec3 &position)
{
    _position = position;
    SetDirty();
}

void Transform::SetPosition(float x, float y, float z)
{
    _position = Vec3(x, y, z);
    SetDirty();
}

void Transform::SetScale(const Vec3 &scale)
{
    _scale = scale;
    SetDirty();
}

void Transform::SetScale(float x, float y, float z)
{
    _scale = Vec3(x, y, z);
    SetDirty();
}

void Transform::SetRotation(const Quat &rotation)
{
    _rotation = rotation;
    SetDirty();
}

void Transform::SetRotation(float x, float y, float z)
{
    _rotation = Quat(x, y, z);
    SetDirty();
}

Vec3 Transform::GetPosition() const
{
    return _position;
}

Vec3 Transform::GetScale() const
{
    return _scale;
}

Quat Transform::GetRotation() const
{
    return _rotation;
}

Vec3 Transform::Right()
{
    return Vec3::right * _rotation;
}

Vec3 Transform::Up()
{
    return Vec3::up * _rotation;
}

Vec3 Transform::Forward()
{
    return Vec3::forward * _rotation;
}

void Transform::AddObserver(ITransformObserver *observer)
{
    _observers.push_back(observer);
}

void Transform::RemoveObserver(ITransformObserver *observer)
{
    for(vector<ITransformObserver*>::iterator it = _observers.begin();
        it != _observers.end();
        ++it)
    {
        if(*it == observer)
        {
            _observers.erase(it);
            break;
        }
    }
}

void Transform::SetDirty()
{
    _matrixDirty = true;
    _inverseDirty = true;

    for(uint32_t i = 0; i < _observers.size(); ++i)
    {
        _observers[i]->OnTransformChanged(this);
    }
}

const Mat4 &Transform::GetMatrix() const
{
    if(_matrixDirty)
    {
        _matrix = _rotation.ToMatrix();

        _matrix.m11 *= _scale.x;
        _matrix.m12 *= _scale.x;
        _matrix.m13 *= _scale.x;
        
        _matrix.m21 *= _scale.y;
        _matrix.m22 *= _scale.y;
        _matrix.m23 *= _scale.y;
        
        _matrix.m31 *= _scale.z;
        _matrix.m32 *= _scale.z;
        _matrix.m33 *= _scale.z;
        
        _matrix.m41 = _position.x;
        _matrix.m42 = _position.y;
        _matrix.m43 = _position.z;

        _matrixDirty = false;
    }

    return _matrix;
}

const Mat4 &Transform::GetInverseMatrix() const
{
    if(_inverseDirty)
    {
        Quat rot = _rotation.Inverse();
        Vec3 pos = -_position;
        Vec3 scl(1.0f / _scale.x, 1.0f / _scale.y, 1.0f / _scale.z);

        _inverseMatrix = rot.ToMatrix();

        _inverseMatrix.m11 *= scl.x;
        _inverseMatrix.m12 *= scl.x;
        _inverseMatrix.m13 *= scl.x;
        
        _inverseMatrix.m21 *= scl.y;
        _inverseMatrix.m22 *= scl.y;
        _inverseMatrix.m23 *= scl.y;
        
        _inverseMatrix.m31 *= scl.z;
        _inverseMatrix.m32 *= scl.z;
        _inverseMatrix.m33 *= scl.z;

        _inverseMatrix.m41 = (pos.x * _inverseMatrix.m11) + (pos.y * _inverseMatrix.m21) + (pos.z * _inverseMatrix.m31);
        _inverseMatrix.m42 = (pos.x * _inverseMatrix.m12) + (pos.y * _inverseMatrix.m22) + (pos.z * _inverseMatrix.m32);
        _inverseMatrix.m43 = (pos.x * _inverseMatrix.m13) + (pos.y * _inverseMatrix.m23) + (pos.z * _inverseMatrix.m33);

        _inverseDirty = false;
    }

    return _inverseMatrix;
}
