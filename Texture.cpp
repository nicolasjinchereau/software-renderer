/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Texture.h"
#include "RenderingContext.h"
#include "TargaImage.h"
#include "BitmapImage.h"
#include "SIMD.h"
#include <fstream>
#include <string>
#include <locale>
using namespace std;

Texture::Texture(const string &filename, FilterMode filterMode)
{
    unique_ptr<Color32[]> tmp;
    
    string ext = filename.substr(filename.find_last_of('.'));
    for(auto& c : ext) c = tolower(c);

    if(ext == ".bmp")
    {
        auto img = BitmapImage::Load(filename);
        tmp = move(img.pixels);
        _width = img.width;
        _height = img.height;
        _channels = img.channels;
    }
    else if(ext == ".tga")
    {
        auto img = TargaImage::Load(filename);
        tmp = move(img.pixels);
        _width = img.width;
        _height = img.height;
        _channels = img.channels;
    }
    else
    {
        throw runtime_error("Invalid file type. Only 24 and 32 bit BMP and TGA files are supported.");
    }

    int pixelCount = 0;
    int w = (int)_width;
    int h = (int)_height;

    while(true)
    {
        _mipmaps.push_back(Mipmap{nullptr, w, h});
        pixelCount += w * h;

        if(w == 1 && h == 1)
            break;

        if(w > 1) w >>= 1;
        if(h > 1) h >>= 1;
    }
    
    _pixels.reset(AlignedAlloc<Color32>(pixelCount, 16));

    Color32* pPixels = _pixels.get();

    for(auto& mip : _mipmaps)
    {
        int count = mip.width * mip.height;
        for(int i = 0; i < count; ++i)
            pPixels[i] = tmp[i];

        mip.pixels = pPixels;

        MipDown(tmp.get(), mip.width, mip.height);
        pPixels += count;
    }

    _filterMode = filterMode;
    _mipmapBias = 0.0f;
}

void Texture::MipDown(Color32* pixels, int w, int h)
{
    if(w > 1 && h > 1)
    {
        int destWidth = w >> 1;
        int destHeight = h >> 1;

        for(int y = 0; y < destHeight; ++y)
        {
            for(int x = 0; x < destWidth; ++x)
            {
                Color32* p = pixels + ((y * 2) * w + (x * 2));

                uint32_t r = p->r;
                uint32_t g = p->g;
                uint32_t b = p->b;
                uint32_t a = p->a;
                ++p;

                r += p->r;
                g += p->g;
                b += p->b;
                a += p->a;
                p += (w - 1);

                r += p->r;
                g += p->g;
                b += p->b;
                a += p->a;
                ++p;

                r += p->r;
                g += p->g;
                b += p->b;
                a += p->a;

                pixels[y * destWidth + x] = Color32(r >> 2, g >> 2, b >> 2, a >> 2);
            }
        }
    }
    else if(w > 1)
    {
        int destWidth = w >> 1;
        int destHeight = h;

        for(int x = 0; x < destWidth; ++x)
        {
            Color32* p = pixels + (x * 2);
            
            uint32_t r = p->r;
            uint32_t g = p->g;
            uint32_t b = p->b;
            uint32_t a = p->a;
            ++p;
            
            r += p->r;
            g += p->g;
            b += p->b;
            a += p->a;

            pixels[destWidth + x] = Color32(r >> 1, g >> 1, b >> 1, a >> 1);
        }
    }
    else if(h > 1)
    {
        int destWidth = w;
        int destHeight = h >> 1;

        for(int y = 0; y < destHeight; ++y)
        {
            Color32* p = pixels + (y * 2);

            uint32_t r = p->r;
            uint32_t g = p->g;
            uint32_t b = p->b;
            uint32_t a = p->a;
            ++p;

            r += p->r;
            g += p->g;
            b += p->b;
            a += p->a;

            pixels[destHeight + y] = Color32(r >> 1, g >> 1, b >> 1, a >> 1);
        }
    }
}

Texture::~Texture()
{
    
}

Color Texture::GetPixel(const Vec2 &uv, float mipLevel)
{
    switch(_filterMode)
    {
    default:
    case FilterMode::Point:
        return GetPoint(uv, mipLevel);
    case FilterMode::Bilinear:
        return GetBilinear(uv, mipLevel);
    case FilterMode::Trilinear:
        return GetTrilinear(uv, mipLevel);
    }
}

Color Texture::GetPoint(const Vec2 &uv, float mipLevel)
{
    Mipmap& mm = _mipmaps[Math::Floor(mipLevel)];
    uint32_t map_w = mm.width;
    uint32_t map_h = mm.height;

    float u = Math::Clamp(uv.x, 0.0f, 1.0f);
    float v = Math::Clamp(uv.y, 0.0f, 1.0f);
    uint32_t x = (uint32_t)((float)(map_w - 1) * u + 0.5f);
    uint32_t y = (uint32_t)((float)(map_h - 1) * v + 0.5f);
    return mm.pixels[y * map_w + x];
}

Color Texture::GetBilinear(const Vec2 &uv, float mipLevel)
{
    Mipmap& mm = _mipmaps[Math::Floor(mipLevel)];
    uint32_t map_w = mm.width;
    uint32_t map_h = mm.height;

    float x = uv.x * (float)map_w;
    float y = uv.y * (float)map_h;
    uint32_t ix = min((uint32_t)x, map_w - 1);
    uint32_t iy = min((uint32_t)y, map_h - 1);

    uint32_t xoff = (ix < map_w - 1) ? 1 : 0;
    uint32_t yoff = (iy < map_h - 1) ? 1 : 0;

    Color32* p00 = mm.pixels + iy * map_w + ix;
    Color32* p01 = p00 + xoff;
    Color32* p10 = p00 + map_w * yoff;
    Color32* p11 = p10 + xoff;

    float u1 = x - (float)ix;
    float u0 = 1 - u1;
    float v1 = y - (float)iy;
    float v0 = 1 - v1;

#if USE_SSE
    alignas(16) float weights[4]{ v0 * u0, v0 * u1, v1 * u0, v1 * u1 };
    int colors[4]{ *(int*)p00, *(int*)p01, *(int*)p10, *(int*)p11 };

    Color ret;
    vblend(colors, weights, &ret.r);
    return ret;
#else
    float w00 = v0 * u0;
    float w01 = v0 * u1;
    float w10 = v1 * u0;
    float w11 = v1 * u1;

    return Color(*p00) * w00
         + Color(*p01) * w01
         + Color(*p10) * w10
         + Color(*p11) * w11;
#endif
}

Color Texture::GetTrilinear(const Vec2 &uv, float mipLevel)
{
    int mip1 = Math::Floor(mipLevel);
    int mip2 = Math::Ceil(mipLevel);

    if(mip1 == mip2)
    {
        return Texture::GetBilinear(uv, mipLevel);
    }
    else
    {
        float t = mipLevel - (int)mipLevel;
        return Color::Lerp(Texture::GetBilinear(uv, (float)mip1),
                           Texture::GetBilinear(uv, (float)mip2),
                           t);
    }
}

void Texture::filterMode(FilterMode mode) {
    _filterMode = mode;
}

FilterMode Texture::filterMode() const {
    return _filterMode;
}
