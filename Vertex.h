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
#if USE_SSE
        Vertex ret;
        vadd(&position.x, &other.position.x, &ret.position.x);
        vadd(&normal.x, &other.normal.x, &ret.normal.x);
        vadd(&texcoord.y, &other.texcoord.y, &ret.texcoord.y);
        return ret;
#else
        return Vertex(position + other.position,
                      normal   + other.normal,
                      texcoord + other.texcoord,
                      worldPos + other.worldPos);
#endif
    }

    Vertex operator-(const Vertex &other) const
    {
#if USE_SSE
        Vertex ret;
        vsub(&position.x, &other.position.x, &ret.position.x);
        vsub(&normal.x, &other.normal.x, &ret.normal.x);
        vsub(&texcoord.y, &other.texcoord.y, &ret.texcoord.y);
        return ret;
#else
        return Vertex(position - other.position,
                      normal   - other.normal,
                      texcoord - other.texcoord,
                      worldPos - other.worldPos);
#endif
    }

    Vertex operator*(float scale) const
    {
#if USE_SSE
        Vertex ret;
        vmul(&position.x, scale, &ret.position.x);
        vmul(&normal.x, scale, &ret.normal.x);
        vmul(&texcoord.y, scale, &ret.texcoord.y);
        return ret;
#else
        return Vertex(position * scale,
                      normal   * scale,
                      texcoord * scale,
                      worldPos * scale);
#endif
    }

    Vertex operator/(float denom) const
    {
        float num = 1.0f / denom;
#if USE_SSE
        Vertex ret;
        vmul(&position.x, num, &ret.position.x);
        vmul(&normal.x, num, &ret.normal.x);
        vmul(&texcoord.y, num, &ret.texcoord.y);
        return ret;
#else
        return Vertex(position * num,
                      normal   * num,
                      texcoord * num,
                      worldPos * num);
#endif
    }

    Vertex &operator+=(const Vertex &other)
    {
#if USE_SSE
        vadd(&position.x, &other.position.x, &position.x);
        vadd(&normal.x, &other.normal.x, &normal.x);
        vadd(&texcoord.y, &other.texcoord.y, &texcoord.y);
#else
        position += other.position;
        normal   += other.normal;
        texcoord += other.texcoord;
        worldPos += other.worldPos;
#endif
        return *this;
    }

    Vertex &operator-=(const Vertex &other)
    {
#if USE_SSE
        vsub(&position.x, &other.position.x, &position.x);
        vsub(&normal.x, &other.normal.x, &normal.x);
        vsub(&texcoord.y, &other.texcoord.y, &texcoord.y);
#else
        position -= other.position;
        normal   -= other.normal;
        texcoord -= other.texcoord;
        worldPos -= other.worldPos;
#endif
        return *this;
    }

    Vertex &operator*=(float scale)
    {
#if USE_SSE
        vmul(&position.x, scale, &position.x);
        vmul(&normal.x, scale, &normal.x);
        vmul(&texcoord.y, scale, &texcoord.y);
#else
        position *= scale;
        normal   *= scale;
        texcoord *= scale;
        worldPos *= scale;
#endif
        return *this;
    }

    Vertex &operator/=(float denom)
    {
        float num = 1.0f / denom;
#if USE_SSE
        vmul(&position.x, num, &position.x);
        vmul(&normal.x, num, &normal.x);
        vmul(&texcoord.y, num, &texcoord.y);
#else
        position *= num;
        normal   *= num;
        texcoord *= num;
        worldPos *= num;
#endif
        return *this;
    }
};
