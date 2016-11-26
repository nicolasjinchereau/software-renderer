/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "Math.h"
#include "SIMD.h"

class alignas(16) Vertex
{
public:
    Vec4 position;
    Vec3 normal;
    Vec2 texcoord;
    Vec3 worldPos;

    Vertex(){}

    Vertex(const Vec4 &pos, const Vec3 &norm, const Vec2 &tex)
        : position(pos), normal(norm), texcoord(tex), worldPos(pos){}

    Vertex(const Vec4 &pos, const Vec3 &norm, const Vec2 &tex, const Vec3& worldPos)
        : position(pos), normal(norm), texcoord(tex), worldPos(worldPos){}

    Vertex operator+(const Vertex &other) const
    {
        return Vertex(position + other.position,
                      normal   + other.normal,
                      texcoord + other.texcoord,
                      worldPos + other.worldPos);
    }

    Vertex operator-(const Vertex &other) const
    {
        return Vertex(position - other.position,
                      normal   - other.normal,
                      texcoord - other.texcoord,
                      worldPos - other.worldPos);
    }

    Vertex operator*(float scale) const
    {
        return Vertex(position * scale,
                      normal   * scale,
                      texcoord * scale,
                      worldPos * scale);
    }

    Vertex operator/(float denom) const
    {
        float num = 1.0f / denom;
        return Vertex(position * num,
                      normal   * num,
                      texcoord * num,
                      worldPos * num);
    }

    Vertex &operator+=(const Vertex &other)
    {
        position += other.position;
        normal   += other.normal;
        texcoord += other.texcoord;
        worldPos += other.worldPos;
        return *this;
    }

    Vertex &operator-=(const Vertex &other)
    {
        position -= other.position;
        normal   -= other.normal;
        texcoord -= other.texcoord;
        worldPos -= other.worldPos;
        return *this;
    }

    Vertex &operator*=(float scale)
    {
        position *= scale;
        normal   *= scale;
        texcoord *= scale;
        worldPos *= scale;
        return *this;
    }

    Vertex &operator/=(float denom)
    {
        float num = 1.0f / denom;
        position *= num;
        normal   *= num;
        texcoord *= num;
        worldPos *= num;
        return *this;
    }
};
