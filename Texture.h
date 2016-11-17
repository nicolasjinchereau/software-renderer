/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Windows.h>
#include <string>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include "Math.h"
#include "Mem.h"
using namespace std;

class RenderingContext;

class Mipmap
{
public:
    Color32* pixels;
    int width;
    int height;
};

enum class FilterMode
{
    Point,
    Bilinear,
    Trilinear,
};

class Texture
{
    unique_ptr<Color32[], AlignedDeleter<Color32>> _pixels;
    vector<Mipmap> _mipmaps;
    
    uint32_t _width;
    uint32_t _height;
    uint32_t _channels;
    FilterMode _filterMode;
    float _mipmapBias;

    static void MipDown(Color32* pixels, int w, int h);

public:
    Texture(const string &filename, FilterMode filterMode);
    ~Texture();

    Color GetPixel(const Vec2 &uv, float mipLevel = 0);
    Color GetPoint(const Vec2 &uv, float mipLevel = 0);
    Color GetBilinear(const Vec2 &uv, float mipLevel = 0);
    Color GetTrilinear(const Vec2 &uv, float mipLevel = 0);
    
    void filterMode(FilterMode mode);
    FilterMode filterMode() const;

    uint32_t width() const { return _width; }
    uint32_t height() const { return _height; }
    uint32_t channels() const { return _channels; }

    Vec2 size() const { return Vec2((float)_width, (float)_height); }
    Vec2 size(int mipLevel) const {
        Mipmap mm = _mipmaps[mipLevel];
        return Vec2((float)mm.width, (float)mm.height);
    }
    
    int mipmapCount() const { return (int)_mipmaps.size(); }
    float mipmapBias() const { return _mipmapBias; }
};
