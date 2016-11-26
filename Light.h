/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include "Math.h"
#include <cstdlib>

enum class LightType
{
    Ambient,
    Directional,
    Point,
    Spot
};

class alignas(16) Light
{
public:
    string name;
    Light(const string& name) : name(name){}

    virtual LightType type() const = 0;
    virtual bool CanAffect(const Sphere& bounds) const = 0;
    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const = 0;
    virtual void Update() = 0;
};

class AmbientLight : public Light
{
public:
    Color color = Color::white;
    float intensity = 1.0f;

    AmbientLight(const string& name) : Light(name){}
    AmbientLight(const string& name,
                 const Color& color,
                 float intensity)
        : Light(name),
        color(color),
        intensity(intensity){}

    virtual LightType type() const override {
        return LightType::Ambient;
    }

    virtual bool CanAffect(const Sphere& bounds) const {
        return true;
    }

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        return color * intensity;
    }

    virtual void Update() override{}
};

class DirectionalLight : public Light
{
public:
    Color color = Color::white;
    float intensity = 1.0f;
    Vec3 direction = Vec3::forward;

    DirectionalLight(const string& name) : Light(name){}
    DirectionalLight(const string& name,
                     const Color& color,
                     float intensity,
                     const Vec3& direction)
        : Light(name),
        color(color),
        intensity(intensity),
        direction(direction){}

    virtual LightType type() const override {
        return LightType::Directional;
    }

    virtual bool CanAffect(const Sphere& bounds) const {
        return true;
    }

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        float cn = surfNorm.Dot(-direction);
        if(cn < 0)
            return Color::clear;

        return color * cn * intensity;
    }

    virtual void Update() override{}
};

class PointLight : public Light
{
public:
    Color color = Color::white;
    float intensity = 1.0f;
    Vec3 position = Vec3::zero;
    float distAttenMin = 4.0f;
    float distAttenMax = 5.0f;
    
    PointLight(const string& name) : Light(name){}
    PointLight(const string& name,
               const Color& color,
               float intensity,
               const Vec3& position,
               float distAttenMin,
               float distAttenMax)
        : Light(name),
        color(color),
        intensity(intensity),
        position(position),
        distAttenMin(distAttenMin),
        distAttenMax(distAttenMax){}

    virtual LightType type() const override {
        return LightType::Point;
    }

    virtual bool CanAffect(const Sphere& bounds) const {
        float r = distAttenMax + bounds.radius;
        float distSq = position.DistanceSq(bounds.center);
        return distSq < r * r;
    }

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        Vec3 lightVec = surfPos - position;
        float lenSq = lightVec.LengthSq();
        
        if(lenSq > distAttenMax * distAttenMax)
            return Color::clear;

        float dist = sqrt(lenSq);
        Vec3 lightDir = lightVec / dist;

        float cn = surfNorm.Dot(-lightDir);
        if(cn < 0)
            return Color::clear;

        float cd = Math::NormalizedClamp(dist, distAttenMin, distAttenMax);
        cd = (1.0f - cd * cd);

        return color * cd * cn * intensity;
    }

    virtual void Update() override{}
};

class SpotLight : public Light
{
    // near, far, left, right, top, bottom
    Plane _frustum[6]{};
public:
    Color color = Color::white;
    float intensity = 1.0f;
    Vec3 position = Vec3::zero;
    Vec3 direction = Vec3::forward;
    float angAttenMin = 40.0f;
    float angAttenMax = 45.0f;
    float distAttenMin = 8.0f;
    float distAttenMax = 10.0f;

    SpotLight(const string& name) : Light(name){}
    SpotLight(const string& name,
              const Color& color,
              float intensity,
              const Vec3& position,
              const Vec3& direction,
              float angAttenMin,
              float angAttenMax,
              float distAttenMin,
              float distAttenMax)
        : Light(name),
        position(position),
        direction(direction),
        color(color),
        intensity(intensity),
        angAttenMin(angAttenMin),
        angAttenMax(angAttenMax),
        distAttenMin(distAttenMin),
        distAttenMax(distAttenMax){}

    virtual LightType type() const override {
        return LightType::Spot;
    }

    virtual bool CanAffect(const Sphere& bounds) const
    {
        for(int i = 0; i < 6; ++i)
        {
            if(_frustum[i].Distance(bounds.center) < -bounds.radius)
                return false;
        }

        return true;
    }

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        Vec3 lightVec = surfPos - position;
        float lenSq = lightVec.LengthSq();
        
        if(lenSq > distAttenMax * distAttenMax)
            return Color::clear;

        if(surfNorm.Dot(-lightVec) < 0)
            return Color::clear;

        float dist = sqrt(lenSq);
        Vec3 lightDir = lightVec / dist;
        
        float cn = surfNorm.Dot(-lightDir);

        float h_angAttenMinRad = Math::DegToRad * angAttenMin * 0.5f;
        float h_angAttenMaxRad = Math::DegToRad * angAttenMax * 0.5f;

        float ang = direction.MaxAcuteAngle(lightDir);
        if(ang > h_angAttenMaxRad)
            return Color::clear;

        float ca = Math::NormalizedClamp(ang, h_angAttenMinRad, h_angAttenMaxRad);
        ca = (1.0f - ca * ca);

        float cd = Math::NormalizedClamp(dist, distAttenMin, distAttenMax);
        cd = (1.0f - cd * cd);

        return color * ca * cd * cn * intensity;
    }

    virtual void Update() override
    {
        float hAng = angAttenMax * 0.5f;
        Quat hRot = Quat::AngleAxis(hAng, Vec3::up);
        Quat vRot = Quat::AngleAxis(hAng, direction.Cross(Vec3::up));

        Vec3 ln = direction * hRot;
        Vec3 rn = direction * hRot.Inverse();
        Vec3 tn = direction * vRot;
        Vec3 bn = direction * vRot.Inverse();

        _frustum[0] = Plane(direction, position);
        _frustum[1] = Plane(-direction, position + direction * distAttenMax);
        _frustum[2] = Plane(ln, position);
        _frustum[3] = Plane(rn, position);
        _frustum[4] = Plane(tn, position);
        _frustum[5] = Plane(bn, position);
    }
};
