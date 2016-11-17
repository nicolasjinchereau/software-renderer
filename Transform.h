/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "Math.h"
#include <vector>
using namespace std;

class Transform;
class ITransformObserver
{
public:
    virtual void OnTransformChanged(Transform *sender) = 0;
};

class alignas(16) Transform
{
public:
    Transform();

    void SetPosition(const Vec3 &position);
    void SetPosition(float x, float y, float z);

    void SetScale(const Vec3 &scale);
    void SetScale(float x, float y, float z);

    void SetRotation(const Quat &rotation);
    void SetRotation(float x, float y, float z);

    Vec3 GetPosition() const;
    Vec3 GetScale() const;
    Quat GetRotation() const;
    
    Vec3 Right();
    Vec3 Up();
    Vec3 Forward();

    const Mat4& GetMatrix() const;
    const Mat4& GetInverseMatrix() const;

    void AddObserver(ITransformObserver *observer);
    void RemoveObserver(ITransformObserver *observer);

private:
    Vec3 _position;
    Vec3 _scale;
    Quat _rotation;
    
    mutable Mat4 _matrix;
    mutable Mat4 _inverseMatrix;
    mutable bool _matrixDirty;
    mutable bool _inverseDirty;

    vector<ITransformObserver*> _observers;

    void SetDirty();
};
