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
    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const = 0;
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

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        return color * intensity;
    }
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

    virtual Color Apply(const Vec3& surfPos, const Vec3& surfNorm, const Vec3& eyePos, const Vec3& eyeDir) const override
    {
        float cn = surfNorm.Dot(-direction);
        if(cn < 0)
            return Color::clear;

        return color * cn * intensity;
    }
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
};

class SpotLight : public Light
{
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

        float ang = Math::Acos(max(direction.Dot(lightDir), 0.0f));
        if(ang > h_angAttenMaxRad)
            return Color::clear;

        float ca = Math::NormalizedClamp(ang, h_angAttenMinRad, h_angAttenMaxRad);
        ca = (1.0f - ca * ca);

        float cd = Math::NormalizedClamp(dist, distAttenMin, distAttenMax);
        cd = (1.0f - cd * cd);

        return color * ca * cd * cn * intensity;
    }
};
