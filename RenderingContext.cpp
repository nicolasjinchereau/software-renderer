/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "RenderingContext.h"
#include "Application.h"
#include "RenderThread.h"
#include "Model.h"
#include "Shader.h"
#include "Scene.h"
#include <cmath>
#include <array>

RenderingContext::RenderingContext(Application* app, uint32_t width, uint32_t height, size_t threadCount)
{
    _hWndTarget = (HWND)app->nativeWindowHandle();
    _hDCTarget = GetDC(_hWndTarget);

    _width = width;
    _height = height;
    
    _clearColor = Color::clear;
    _rasterizationMode = RasterizationMode::Halfspace;
    _mipmapsEnabled = true;
    _antiAliasingMode = AntiAliasingMode::Off;
    
    _renderWidth = _width;
    _renderHeight = _height;
    _colorBuffer.Resize(width, height, 1);
    _depthBuffer.Resize(width, height, 1);

    for(size_t i = 0; i < threadCount; ++i)
        _renderThreads.push_back(make_unique<RenderThread>());
}

RenderingContext::~RenderingContext()
{

}

void RenderingContext::clearColor(const Color &color) {
    _clearColor = color;
}

Color RenderingContext::clearColor() const {
    return _clearColor;
}

void RenderingContext::rasterizationMode(RasterizationMode mode) {
    _rasterizationMode = mode;
}

RasterizationMode RenderingContext::rasterizationMode() const {
    return _rasterizationMode;
}

void RenderingContext::antiAliasingMode(AntiAliasingMode mode)
{
    if(mode == AntiAliasingMode::Off
    || (mode == AntiAliasingMode::MSAA_4X && _rasterizationMode == RasterizationMode::Scanline))
    {
        _renderWidth = _width;
        _renderHeight = _height;
        _colorBuffer.Resize(_renderWidth, _renderHeight, 1);
        _depthBuffer.Resize(_renderWidth, _renderHeight, 1);
        _aaBuffer.Resize(0, 0, 0);
    }
    else if(mode == AntiAliasingMode::SSAA_2X)
    {
        _renderWidth = _width * 2;
        _renderHeight = _height * 2;
        _colorBuffer.Resize(_width, _height, 1);
        _depthBuffer.Resize(_width, _height, 4);
        _aaBuffer.Resize(_width, _height, 4);
    }
    else if(mode == AntiAliasingMode::SSAA_4X)
    {
        _renderWidth = _width * 4;
        _renderHeight = _height * 4;
        _colorBuffer.Resize(_width, _height, 1);
        _depthBuffer.Resize(_width, _height, 16);
        _aaBuffer.Resize(_width, _height, 16);
    }
    else if(mode == AntiAliasingMode::MSAA_4X)
    {
        _renderWidth = _width;
        _renderHeight = _height;
        _colorBuffer.Resize(_width, _height, 1);
        _depthBuffer.Resize(_width, _height, 4);
        _aaBuffer.Resize(_width, _height, 4);
    }

    _antiAliasingMode = mode;
}

AntiAliasingMode RenderingContext::antiAliasingMode() const
{
    return _antiAliasingMode;
}

void RenderingContext::mipmapsEnabled(bool enabled) {
    _mipmapsEnabled = enabled;
}

bool RenderingContext::mipmapsEnabled() const {
    return _mipmapsEnabled;
}

HWND RenderingContext::targetWindow() const {
    return _hWndTarget;
}

uint32_t RenderingContext::width() const {
    return _width;
}

uint32_t RenderingContext::height() const {
    return _height;
}

void RenderingContext::Draw(const shared_ptr<Scene>& scene)
{
    _drawCalls.reserve(scene->objects.size());

    for(auto obj : scene->objects)
    {
        if(scene->camera->CanSee(obj->GetWorldBoundingSphere()))
        {
            obj->shader->Prepare(scene.get(), obj.get());

            if(obj->model->vertices.empty())
                continue;
            
            size_t sz = obj->model->vertices.size();

            _xverts.reserve(sz);

            for(auto& v : obj->model->vertices)
                _xverts.push_back(obj->shader->ProcessVertex(v));
            
            size_t start = _cverts.size();
            _cverts.reserve(start + _xverts.size());

            for(auto it = _xverts.begin(); it != _xverts.end(); )
            {
                // clip near/far planes
                Vertex tmp[9];
                tmp[0] = *it++;
                tmp[1] = *it++;
                tmp[2] = *it++;

                int nVerts = 3;

                nVerts = ClipDepth(tmp, nVerts);
                if(nVerts < 3)
                    continue;

                for(int i = 0; i < nVerts; ++i)
                {
                    // perspective divide -> normalized device coordinates
                    float zr = 1.0f / tmp[i].position.w;
                    tmp[i] *= zr;
                    tmp[i].position.w = zr;

                    // viewport transformation -> screen space
                    tmp[i].position.x = (tmp[i].position.x + 1.0f) * 0.5f * (float)_renderWidth;
                    tmp[i].position.y = (tmp[i].position.y + 1.0f) * 0.5f * (float)_renderHeight;
                    tmp[i].position.y = (float)_renderHeight - tmp[i].position.y;
                }

                nVerts = ClipScreen(tmp, nVerts);
                if(nVerts < 3)
                    continue;

                for(int i = 1; i < nVerts - 1; i++)
                {
                    _cverts.push_back(tmp[0]);
                    _cverts.push_back(tmp[i]);
                    _cverts.push_back(tmp[i + 1]);
                }
            }

            _xverts.clear();
            size_t end = _cverts.size();
            obj->shader->CopyTo(_shaders);

            _drawCalls.push_back({ start, end, obj.get(), nullptr });
        }
    }
    
    auto st = _shaders.begin();
    for(auto it = _drawCalls.begin(); it != _drawCalls.end(); ++it, ++st) {
        it->shader = *st;
    }

    size_t threadCount = _renderThreads.size();
    int segment = _renderHeight / threadCount;
    int lastseg = _renderHeight - segment * (threadCount - 1);

    size_t i = 0;
    for( ; i < threadCount - 1; ++i)
        _renderThreads[i]->Execute(this, Rect(0, segment * i, _renderWidth, segment));

    if(i < threadCount)
        _renderThreads[i]->Execute(this, Rect(0, segment * i, _renderWidth, lastseg));

    for(size_t j = 0; j < threadCount; ++j)
        _renderThreads[j]->Wait();

    _drawCalls.clear();
    _shaders.clear();
    _cverts.clear();
}

int RenderingContext::ClipDepth(Vertex (&verts)[9], int count)
{
    Vertex tmp[9];
    int newCount = 0;

    // clip near plane
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = verts[i];
        Vertex &p1 = verts[(i + 1) % count];

        bool in0 = p0.position.z > 0;
        bool in1 = p1.position.z > 0;
        if(in0 != in1)
        {
            float t = -p0.position.z / (p1.position.z - p0.position.z);
            tmp[newCount++] = p0 + (p1 - p0) * t;
        }

        if(in1)
        {
            tmp[newCount++] = p1;
        }
    }

    count = newCount;
    newCount = 0;

    // clip far plane
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = tmp[i];
        Vertex &p1 = tmp[(i + 1) % count];

        bool in0 = p0.position.z <= p0.position.w;
        bool in1 = p1.position.z <= p1.position.w;

        if(in0 != in1)
        {
            float t = (p0.position.w - p0.position.z) / (p1.position.z - p0.position.z - p1.position.w + p0.position.w);
            verts[newCount++] = p0 + (p1 - p0) * t;
        }

        if(in1)
        {
            verts[newCount++] = p1;
        }
    }

    return newCount;
}

int RenderingContext::ClipScreen(Vertex (&verts)[9], int count)
{
    Vertex tmp[9];
    int newCount = 0;

    // clip left edge of screen
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = verts[i];
        Vertex &p1 = verts[(i + 1) % count];

        float left = 0;
        bool in0 = p0.position.x >= left;
        bool in1 = p1.position.x >= left;
        
        if(in0 != in1)
        {
            float t = (left - p0.position.x) / (p1.position.x - p0.position.x);
            tmp[newCount] = p0 + (p1 - p0) * t;
            tmp[newCount++].position.x = left;
        }

        if(in1)
        {
            tmp[newCount++] = p1;
        }
    }
    
    count = newCount;
    newCount = 0;

    // clip right edge of screen
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = tmp[i];
        Vertex &p1 = tmp[(i + 1) % count];

        float right = (float)_renderWidth;
        bool in0 = p0.position.x <= right;
        bool in1 = p1.position.x <= right;
        
        if(in0 != in1)
        {
            float t = (right - p0.position.x) / (p1.position.x - p0.position.x);
            verts[newCount] = p0 + (p1 - p0) * t;
            verts[newCount++].position.x = right;
        }

        if(in1)
        {
            verts[newCount++] = p1;
        }
    }

    count = newCount;
    newCount = 0;

    // clip top edge of screen
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = verts[i];
        Vertex &p1 = verts[(i + 1) % count];

        float top = 0;
        bool in0 = p0.position.y >= top;
        bool in1 = p1.position.y >= top;
        
        if(in0 != in1)
        {
            float t = (top - p0.position.y) / (p1.position.y - p0.position.y);
            tmp[newCount] = p0 + (p1 - p0) * t;
            tmp[newCount++].position.y = top;
        }

        if(in1)
        {
            tmp[newCount++] = p1;
        }
    }

    count = newCount;
    newCount = 0;

    // clip bottom edge of screen
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = tmp[i];
        Vertex &p1 = tmp[(i + 1) % count];

        float bot = (float)_renderHeight;
        bool in0 = p0.position.y <= bot;
        bool in1 = p1.position.y <= bot;
        
        if(in0 != in1)
        {
            float t = (bot - p0.position.y) / (p1.position.y - p0.position.y);
            verts[newCount] = p0 + (p1 - p0) * t;
            verts[newCount++].position.y = bot;
        }

        if(in1)
        {
            verts[newCount++] = p1;
        }
    }

    return newCount;
}

float RenderingContext::CalcMipLevel(const Vec2& uv00, const Vec2& uv01, const Vec2& uv10, const Vec2& texSize, float mipBias, int mipCount)
{
    Vec2 uvDx = (uv01 - uv00).Scale(texSize);
    Vec2 uvDy = (uv10 - uv00).Scale(texSize);
    float mipLevel = 0.5f * Math::Log2(max(uvDx.LengthSq(), uvDy.LengthSq()));
    return Math::Clamp(mipLevel + mipBias, 0.0f, (float)(mipCount - 1));
}

void RenderingContext::Rasterize(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall)
{
    switch(_rasterizationMode)
    {
    case RasterizationMode::Scanline:
        RasterizeScanline(rect, v0, v1, v2, drawCall);
        break;

    case RasterizationMode::Halfspace:
        if(_antiAliasingMode == AntiAliasingMode::MSAA_4X)
            RasterizeHalfSpaceMSAA(rect, v0, v1, v2, drawCall);
        else
            RasterizeHalfSpace(rect, v0, v1, v2, drawCall);
        break;
    }
}

void RenderingContext::RasterizeHalfSpace(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall)
{
    Vec2 sv1 = v0.position;
    Vec2 sv2 = v1.position;
    Vec2 sv3 = v2.position;
    
    int minx = Math::Floor(Math::Min(sv1.x, sv2.x, sv3.x));
    int miny = Math::Floor(Math::Min(sv1.y, sv2.y, sv3.y));
    int maxx = Math::Ceil(Math::Max(sv1.x, sv2.x, sv3.x));
    int maxy = Math::Ceil(Math::Max(sv1.y, sv2.y, sv3.y));
    
    minx = Math::Clamp(minx, rect.x, rect.x + rect.w);
    maxx = Math::Clamp(maxx, rect.x, rect.x + rect.w);
    miny = Math::Clamp(miny, rect.y, rect.y + rect.h);
    maxy = Math::Clamp(maxy, rect.y, rect.y + rect.h);

    if(maxx - minx < 1 || maxy - miny < 1)
        return;

    BarycentricTriangle tri(sv1, sv2, sv3);
    if(tri.empty())
        return;

    Vec2 minPt = Vec2((float)minx + 0.5f, (float)miny + 0.5f);
    Vertex v00 = tri.Interpolate(v0, v1, v2, minPt);
    Vertex v01 = tri.Interpolate(v0, v1, v2, minPt + Vec2(100, 0));
    Vertex v10 = tri.Interpolate(v0, v1, v2, minPt + Vec2(0, 100));
    Vertex xDelta = (v01 - v00) * 0.01f;
    Vertex yDelta = (v10 - v00) * 0.01f;

    // det > 0 for any point 'p' that is to the left of (v2 - v1) in screen space
    // float det = (v2.y - v1.y) * (p.x - v1.x) - (v2.x - v1.x) * (p.y - v1.y);

    Vec3 Dx(sv2.y - sv1.y,
            sv3.y - sv2.y,
            sv1.y - sv3.y);

    Vec3 Dy(-(sv2.x - sv1.x),
            -(sv3.x - sv2.x),
            -(sv1.x - sv3.x));

    Vec3 orig(Dx.x * -sv1.x + Dy.x * -sv1.y,
              Dx.y * -sv2.x + Dy.y * -sv2.y,
              Dx.z * -sv3.x + Dy.z * -sv3.y);
    
    Vec3 off = Vec3::zero;
    if(sv2.y > sv1.y || (abs(sv2.y - sv1.y) < FLT_EPSILON && sv2.x < sv1.x)) off.x += FLT_EPSILON;
    if(sv3.y > sv2.y || (abs(sv3.y - sv2.y) < FLT_EPSILON && sv3.x < sv2.x)) off.y += FLT_EPSILON;
    if(sv1.y > sv3.y || (abs(sv1.y - sv3.y) < FLT_EPSILON && sv1.x < sv3.x)) off.z += FLT_EPSILON;
    orig += off;
    off *= -2.0f;
    
    Vec3 Cy = orig + Dx * (float)(minx + 0.5f) + Dy * (float)(miny + 0.5f);

    Vertex yv = v00;

    CullMode cullMode = drawCall->obj->cullMode;
    Texture* tex = drawCall->obj->texture.get();
    Vec2 texSize = tex->size();
    float mipBias = tex->mipmapBias();
    int mipCount = tex->mipmapCount();
    Shader* shader = drawCall->shader;
    RenderBuffer<uint32_t>& outBuffer = _antiAliasingMode == AntiAliasingMode::Off ?
                                            _colorBuffer : _aaBuffer;

    for(int y = miny; y < maxy; y++)
    {
        Vec3 Cx = Cy;
        Vertex xv;

        int x = minx;

        for( ; x < maxx; ++x)
        {
            bool visible = false;
            
            if(cullMode != CullMode::Front)
            {
                visible |= Cx.x > 0 && Cx.y > 0 && Cx.z > 0;
            }

            if(cullMode != CullMode::Back)
            {
                Vec3 CxBack = Cx + off;
                visible |= CxBack.x < 0 && CxBack.y < 0 && CxBack.z < 0;
            }

            if(visible)
            {
                xv = yv + xDelta * (float)(x - minx);
                break;
            }

            Cx += Dx;
        }

        uint32_t rowOffset;
        if(_antiAliasingMode == AntiAliasingMode::SSAA_2X)
            rowOffset = outBuffer.GetSuperSampleRowOffset<2>(y);
        else if(_antiAliasingMode == AntiAliasingMode::SSAA_4X)
            rowOffset = outBuffer.GetSuperSampleRowOffset<4>(y);
        else
            rowOffset = y * _width;

        for( ; x < maxx; ++x)
        {
            bool visible = false;
            
            if(cullMode != CullMode::Front)
                visible |= Cx.x > 0 && Cx.y > 0 && Cx.z > 0;

            if(cullMode != CullMode::Back)
            {
                Vec3 CxBack = Cx + off;
                visible |= CxBack.x < 0 && CxBack.y < 0 && CxBack.z < 0;
            }

            if(visible)
            {
                uint32_t offset;
                if(_antiAliasingMode == AntiAliasingMode::SSAA_2X)
                    offset = rowOffset + outBuffer.GetSuperSampleColumnOffset<2>(x);
                else if(_antiAliasingMode == AntiAliasingMode::SSAA_4X)
                    offset = rowOffset + outBuffer.GetSuperSampleColumnOffset<4>(x);
                else
                    offset = rowOffset + x;

                uint32_t *colorBuffer = outBuffer.data() + offset;
                float *depthBuffer = _depthBuffer.data() + offset;

                if(xv.position.w > *depthBuffer)
                {
                    Vertex frag = xv / xv.position.w;
                    Vec2 uv00 = frag.texcoord;
                    Vec2 uv01 = (xv.texcoord + xDelta.texcoord) / (xv.position.w + xDelta.position.w);
                    Vec2 uv10 = (xv.texcoord + yDelta.texcoord) / (xv.position.w + yDelta.position.w);
                    float mipLevel = _mipmapsEnabled ?
                        CalcMipLevel(uv00, uv01, uv10, texSize, mipBias, mipCount) : 0;
                    
                    bool discard = false;
                    Color output = Color::Clamp(shader->ProcessPixel(frag, mipLevel, discard));
                    if(!discard) {
                        *colorBuffer = output;
                        *depthBuffer = xv.position.w;
                    }
                }

                xv += xDelta;
                Cx += Dx;
            }
            else
            {
                break;
            }
        }
        
        yv += yDelta;
        Cy += Dy;
    }
}

void RenderingContext::RasterizeHalfSpaceMSAA(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall)
{
    Vec2 sv1 = v0.position;
    Vec2 sv2 = v1.position;
    Vec2 sv3 = v2.position;
    
    int minx = Math::Floor(Math::Min(sv1.x, sv2.x, sv3.x));
    int miny = Math::Floor(Math::Min(sv1.y, sv2.y, sv3.y));
    int maxx = Math::Ceil(Math::Max(sv1.x, sv2.x, sv3.x));
    int maxy = Math::Ceil(Math::Max(sv1.y, sv2.y, sv3.y));

    minx = Math::Clamp(minx, rect.x, rect.x + rect.w);
    maxx = Math::Clamp(maxx, rect.x, rect.x + rect.w);
    miny = Math::Clamp(miny, rect.y, rect.y + rect.h);
    maxy = Math::Clamp(maxy, rect.y, rect.y + rect.h);

    if((maxx - minx < 1) || (maxy - miny < 1))
        return;

    BarycentricTriangle tri(sv1, sv2, sv3);
    if(tri.empty())
        return;

    Vec2 minPt = Vec2((float)minx + 0.5f, (float)miny + 0.5f);
    Vertex v00 = tri.Interpolate(v0, v1, v2, minPt);
    Vertex v01 = tri.Interpolate(v0, v1, v2, minPt + Vec2(100, 0));
    Vertex v10 = tri.Interpolate(v0, v1, v2, minPt + Vec2(0, 100));
    Vertex xDelta = (v01 - v00) * 0.01f;
    Vertex yDelta = (v10 - v00) * 0.01f;

    // det > 0 for any point 'p' that is to the left of (v2 - v1) in screen space
    // float det = (v2.y - v1.y) * (p.x - v1.x) - (v2.x - v1.x) * (p.y - v1.y);

    Vec3 Dx(sv2.y - sv1.y,
            sv3.y - sv2.y,
            sv1.y - sv3.y);

    Vec3 Dy(-(sv2.x - sv1.x),
            -(sv3.x - sv2.x),
            -(sv1.x - sv3.x));

    Vec3 orig(Dx.x * -sv1.x + Dy.x * -sv1.y,
              Dx.y * -sv2.x + Dy.y * -sv2.y,
              Dx.z * -sv3.x + Dy.z * -sv3.y);
    
    Vec3 off = Vec3::zero;
    if(sv2.y > sv1.y || (abs(sv2.y - sv1.y) < FLT_EPSILON && sv2.x < sv1.x)) off.x += FLT_EPSILON;
    if(sv3.y > sv2.y || (abs(sv3.y - sv2.y) < FLT_EPSILON && sv3.x < sv2.x)) off.y += FLT_EPSILON;
    if(sv1.y > sv3.y || (abs(sv1.y - sv3.y) < FLT_EPSILON && sv1.x < sv3.x)) off.z += FLT_EPSILON;
    orig += off;
    off *= -2.0f;

    constexpr int SAMPLE_COUNT = 4;
    Vec2 sampleOffset[SAMPLE_COUNT]{
        {  0.375f, -0.125f },
        { -0.125f, -0.375f },
        { -0.375f,  0.125f },
        {  0.125f,  0.375f },
    };

    array<Vec3, SAMPLE_COUNT> Cy;
    for(int i = 0; i < SAMPLE_COUNT; ++i)
        Cy[i] = orig + Dx * (minx + 0.5f + sampleOffset[i].x) + Dy * (miny + 0.5f + sampleOffset[i].y);
    
    Vertex yv = v00;

    CullMode cullMode = drawCall->obj->cullMode;
    Texture* tex = drawCall->obj->texture.get();
    Vec2 texSize = tex->size();
    float mipBias = tex->mipmapBias();
    int mipCount = tex->mipmapCount();
    Shader* shader = drawCall->shader;
    RenderBuffer<uint32_t>& outBuffer = _antiAliasingMode == AntiAliasingMode::Off ?
                                            _colorBuffer : _aaBuffer;

    for(int y = miny; y < maxy; ++y)
    {
        array<Vec3, SAMPLE_COUNT> Cx = Cy;
        
        Vertex xv;
        array<float, SAMPLE_COUNT> ws;
        uint32_t *colorBuffer = nullptr;
        float *depthBuffer = nullptr;
        
        int x = minx;

        for( ; x < maxx; ++x)
        {
            uint8_t coverage = 0;

            if(cullMode != CullMode::Front)
            {
                for(int i = 0; i < SAMPLE_COUNT; ++i)
                    coverage |= (Cx[i].x > 0 && Cx[i].y > 0 && Cx[i].z > 0) << i;
            }

            if(cullMode != CullMode::Back)
            {
                for(int i = 0; i < SAMPLE_COUNT; ++i)
                {
                    Vec3 CxBack = Cx[i] + off;
                    coverage |= (CxBack.x < 0 && CxBack.y < 0 && CxBack.z < 0) << i;
                }
            }

            if(coverage)
            {
                xv = yv + xDelta * (float)(x - minx);

                for(int i = 0; i < SAMPLE_COUNT; ++i)
                    ws[i] = xv.position.w + xDelta.position.w * sampleOffset[i].x + yDelta.position.w * sampleOffset[i].y;

                uint32_t offset = (y * _renderWidth + x) * SAMPLE_COUNT;
                colorBuffer = outBuffer.data() + offset;
                depthBuffer = _depthBuffer.data() + offset;

                break;
            }

            for(int i = 0; i < SAMPLE_COUNT; ++i)
                Cx[i] += Dx;
        }
        
        for( ; x < maxx; ++x)
        {
            uint8_t coverage = 0;

            if(cullMode != CullMode::Front)
            {
                for(int i = 0; i < SAMPLE_COUNT; ++i)
                    coverage |= (Cx[i].x > 0 && Cx[i].y > 0 && Cx[i].z > 0) << i;
            }

            if(cullMode != CullMode::Back)
            {
                for(int i = 0; i < SAMPLE_COUNT; ++i)
                {
                    Vec3 CxBack = Cx[i] + off;
                    coverage |= (CxBack.x < 0 && CxBack.y < 0 && CxBack.z < 0) << i;
                }
            }

            if(coverage)
            {
                uint8_t depth = 0;
                for(int i = 0; i < SAMPLE_COUNT; ++i)
                    depth |= (ws[i] > depthBuffer[i]) << i;
                
                uint8_t fill = coverage & depth;
                if(fill)
                {
                    Vertex frag = xv / xv.position.w;
                    Vec2 uv00 = frag.texcoord;
                    Vec2 uv01 = (xv.texcoord + xDelta.texcoord) / (xv.position.w + xDelta.position.w);
                    Vec2 uv10 = (xv.texcoord + yDelta.texcoord) / (xv.position.w + yDelta.position.w);
                    float mipLevel = _mipmapsEnabled ?
                        CalcMipLevel(uv00, uv01, uv10, texSize, mipBias, mipCount) : 0;
                    
                    bool discard = false;
                    Color output = Color::Clamp(shader->ProcessPixel(frag, mipLevel, discard));
                    if(!discard)
                    {
                        for(int i = 0; i < SAMPLE_COUNT; ++i)
                        {
                            if(fill & (1 << i)) {
                                colorBuffer[i] = output;
                                depthBuffer[i] = ws[i];
                            }
                        }
                    }
                }
            }
            else
            {
                // Not sure why yet, but MSAA sampling causes intermittent
                // visibility, which means bailing early skips covered pixels.
                //break;
            }

            xv += xDelta;

            for(int i = 0; i < SAMPLE_COUNT; ++i) {
                Cx[i] += Dx;
                ws[i] += xDelta.position.w;
            }
            colorBuffer += SAMPLE_COUNT;
            depthBuffer += SAMPLE_COUNT;
        }
        
        yv += yDelta;

        for(int i = 0; i < SAMPLE_COUNT; ++i)
            Cy[i] += Dy;
    }
}

void RenderingContext::RasterizeScanline(const Rect& rect, const Vertex& _v0, const Vertex& _v1, const Vertex& _v2, DrawCall* drawCall)
{
    auto cullMode = drawCall->obj->cullMode;
    if(cullMode != CullMode::None)
    {
        // cross product in screen space -> triangle back-facing?
        Vec2 a = Vec2(_v2.position) - Vec2(_v1.position);
        Vec2 b = Vec2(_v0.position) - Vec2(_v1.position);
        float det = a.Det(b);

        if(cullMode == CullMode::Back && det > 0)
            return;
        else if(cullMode == CullMode::Front && det < 0)
            return;
    }

    Vertex v0 = _v0;
    Vertex v1 = _v1;
    Vertex v2 = _v2;

    if(v2.position.y < v1.position.y) swap(v2, v1);
    if(v2.position.y < v0.position.y) swap(v2, v0);
    if(v1.position.y < v0.position.y) swap(v1, v0);

    Vec2 sv1 = v0.position;
    Vec2 sv2 = v1.position;
    Vec2 sv3 = v2.position;

    BarycentricTriangle tri(sv1, sv2, sv3);
    if(tri.empty())
        return;

    Vertex v00 = tri.Interpolate(v0, v1, v2, sv1);
    Vertex v01 = tri.Interpolate(v0, v1, v2, sv1 + Vec2(100, 0));
    Vertex v10 = tri.Interpolate(v0, v1, v2, sv1 + Vec2(0, 100));
    Vertex xDelta = (v01 - v00) * 0.01f;
    Vertex yDelta = (v10 - v00) * 0.01f;

    float t = (v1.position.y - v0.position.y) / (v2.position.y - v0.position.y);
    Vertex v1b = v0 + (v2 - v0) * t;
    if(v1b.position.x < v1.position.x) swap(v1, v1b);
    
    if(Math::Ceil(v0.position.y) < Math::Ceil(v1.position.y))
        FillSpans(rect, v0, v1, v0, v1b, xDelta, yDelta, drawCall);

    if(Math::Ceil(v1.position.y) < Math::Ceil(v2.position.y))
        FillSpans(rect, v1, v2, v1b, v2, xDelta, yDelta, drawCall);
}

void RenderingContext::FillSpans(const Rect& rect, const Vertex& _l0, const Vertex& _l1, const Vertex& _r0, const Vertex& _r1, const Vertex& _xDelta, const Vertex& _yDelta, DrawCall* drawCall)
{
    Vertex l0 = _l0;
    Vertex l1 = _l1;
    Vertex r0 = _r0;
    Vertex r1 = _r1;

    Vertex xDelta = _xDelta;
    Vertex yDelta = _yDelta;

    int y0 = Math::Ceil(l0.position.y);
    int y1 = Math::Min(Math::Ceil(l1.position.y), (int)_renderHeight, rect.y + rect.h);

    // calculate the vertical deltas down the edges of the triangle
    Vertex yDeltaLeft = (l1 - l0) / (l1.position.y - l0.position.y);
    Vertex yDeltaRight = (r1 - r0) / (r1.position.y - r0.position.y);

    l0 += yDeltaLeft * (Math::Ceil(l0.position.y) - l0.position.y);
    r0 += yDeltaRight * (Math::Ceil(r0.position.y) - r0.position.y);

    Texture* tex = drawCall->obj->texture.get();
    Vec2 texSize = tex->size();
    float mipBias = tex->mipmapBias();
    int mipCount = tex->mipmapCount();
    Shader* shader = drawCall->shader;
    RenderBuffer<uint32_t>& outBuffer =
        _antiAliasingMode == AntiAliasingMode::Off || _antiAliasingMode == AntiAliasingMode::MSAA_4X ?
            _colorBuffer : _aaBuffer;

    int yStart = min(rect.y, y1);
    int startOff = max(yStart - y0, 0);
    l0 += yDeltaLeft * (float)startOff;
    r0 += yDeltaRight * (float)startOff;
    int y = y0 + startOff;

    for( ; y < y1; ++y)
    {
        int x = Math::Ceil(l0.position.x);
        int end = min(Math::Ceil(r0.position.x), (int)_renderWidth);

        Vertex xv = l0;
        xv += xDelta * (Math::Ceil(l0.position.x) - l0.position.x);

        uint32_t rowOffset;
        if(_antiAliasingMode == AntiAliasingMode::SSAA_2X)
            rowOffset = outBuffer.GetSuperSampleRowOffset<2>(y);
        else if(_antiAliasingMode == AntiAliasingMode::SSAA_4X)
            rowOffset = outBuffer.GetSuperSampleRowOffset<4>(y);
        else
            rowOffset = y * _width;

        for( ; x < end; ++x)
        {
            uint32_t offset;
            if(_antiAliasingMode == AntiAliasingMode::SSAA_2X)
                offset = rowOffset + outBuffer.GetSuperSampleColumnOffset<2>(x);
            else if(_antiAliasingMode == AntiAliasingMode::SSAA_4X)
                offset = rowOffset + outBuffer.GetSuperSampleColumnOffset<4>(x);
            else
                offset = rowOffset + x;

            uint32_t *colorBuffer = outBuffer.data() + offset;
            float *depthBuffer = _depthBuffer.data() + offset;

            if(xv.position.w > *depthBuffer)
            {
                Vertex frag = xv / xv.position.w;

                Vec2 uv00 = frag.texcoord;
                Vec2 uv01 = (xv.texcoord + xDelta.texcoord) / (xv.position.w + xDelta.position.w);
                Vec2 uv10 = (xv.texcoord + yDelta.texcoord) / (xv.position.w + yDelta.position.w);
                float mipLevel = _mipmapsEnabled ?
                        CalcMipLevel(uv00, uv01, uv10, texSize, mipBias, mipCount) : 0;

                bool discard = false;
                Color output = Color::Clamp(shader->ProcessPixel(frag, mipLevel, discard));
                if(!discard)
                {
                    *colorBuffer = output;
                    *depthBuffer = xv.position.w;
                }
            }

            xv += xDelta;
        }

        l0 += yDeltaLeft;
        r0 += yDeltaRight;
    }
}

void RenderingContext::Resolve(const Rect& rect)
{
    if(_antiAliasingMode == AntiAliasingMode::SSAA_2X)
        ResolveSSAA2X(rect);
    else if(_antiAliasingMode == AntiAliasingMode::SSAA_4X)
        ResolveSSAA4X(rect);
    else if(_antiAliasingMode == AntiAliasingMode::MSAA_4X && _rasterizationMode == RasterizationMode::Halfspace)
        ResolveMSAA4X(rect);
}

void RenderingContext::ResolveSSAA2X(const Rect& rect)
{
    int destY = rect.y / 2;
    int destW = rect.w / 2;
    int destH = rect.h / 2;

    uint32_t* src = _aaBuffer.data() + rect.y * rect.w;
    uint32_t* dst = _colorBuffer.data() + destY * destW;
    
    int count = destW * destH;
    while(count--)
    {
#if USE_SSE
        __m128i c = _mm_load_si128((__m128i*)src);
        c = _mm_avg_epu8(c, _mm_srli_si128(c, 4));
        c = _mm_avg_epu8(c, _mm_srli_si128(c, 8));
        *dst = _mm_cvtsi128_si32(c);
#else
        uint32_t bgra[4]{0, 0, 0, 0};

        for(int i = 0; i < 4; ++i)
        {
            ColorBGRA* p = (ColorBGRA*)src + i;
            bgra[0] += p->b;
            bgra[1] += p->g;
            bgra[2] += p->r;
            bgra[3] += p->a;
        }
        
        ColorBGRA* c = (ColorBGRA*)dst;
        c->b = (uint8_t)(bgra[0] / 4);
        c->g = (uint8_t)(bgra[1] / 4);
        c->r = (uint8_t)(bgra[2] / 4);
        c->a = (uint8_t)(bgra[3] / 4);
#endif
        src += 4;
        dst += 1;
    }
}

void RenderingContext::ResolveSSAA4X(const Rect& rect)
{
    int ry = rect.y / 4;
    int rw = rect.w / 4;
    int rh = rect.h / 4;

    auto* src = _aaBuffer.data() + rect.y * rect.w;
    auto* dst = _colorBuffer.data() + ry * rw;
    
    int count = rw * rh;
    while(count--)
    {
#if USE_SSE
        __m128i c = _mm_load_si128((__m128i*)src);
        __m128i r = _mm_cvtepu8_epi32(c);
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 4)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 8)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 12)));
        src += 4;
        
        c = _mm_load_si128((__m128i*)src);
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(c));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 4)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 8)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 12)));
        src += 4;

        c = _mm_load_si128((__m128i*)src);
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(c));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 4)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 8)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 12)));
        src += 4;

        c = _mm_load_si128((__m128i*)src);
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(c));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 4)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 8)));
        r = _mm_add_epi32(r, _mm_cvtepu8_epi32(_mm_srli_si128(c, 12)));
        src += 4;

        r = _mm_srli_epi32(r, 4);
        r = _mm_packus_epi32(r, _mm_setzero_si128());
        r = _mm_packus_epi16(r, _mm_setzero_si128());
        *dst++ = _mm_cvtsi128_si32(r);
#else
        uint32_t bgra[4]{0, 0, 0, 0};

        for(int i = 0; i < 16; ++i)
        {
            ColorBGRA* p = (ColorBGRA*)src + i;
            bgra[0] += p->b;
            bgra[1] += p->g;
            bgra[2] += p->r;
            bgra[3] += p->a;
        }
        
        ColorBGRA* c = (ColorBGRA*)dst;
        c->b = (uint8_t)(bgra[0] / 16);
        c->g = (uint8_t)(bgra[1] / 16);
        c->r = (uint8_t)(bgra[2] / 16);
        c->a = (uint8_t)(bgra[3] / 16);
        src += 16;
        dst += 1;
#endif
    }
}

void RenderingContext::ResolveMSAA4X(const Rect& rect)
{
    uint32_t* src = _aaBuffer.data() + rect.y * rect.w * 4;
    uint32_t* dst = _colorBuffer.data() + rect.y * rect.w;

    int count = rect.w * rect.h;
    while(count--)
    {
#if USE_SSE
        __m128i c = _mm_load_si128((__m128i*)src);
        c = _mm_avg_epu8(c, _mm_srli_si128(c, 4));
        c = _mm_avg_epu8(c, _mm_srli_si128(c, 8));
        *dst = _mm_cvtsi128_si32(c);
#else
        uint32_t bgra[4]{0, 0, 0, 0};

        for(int i = 0; i < 4; ++i)
        {
            ColorBGRA* p = (ColorBGRA*)src + i;
            bgra[0] += p->b;
            bgra[1] += p->g;
            bgra[2] += p->r;
            bgra[3] += p->a;
        }
        
        ColorBGRA* c = (ColorBGRA*)dst;
        c->b = (uint8_t)(bgra[0] / 4);
        c->g = (uint8_t)(bgra[1] / 4);
        c->r = (uint8_t)(bgra[2] / 4);
        c->a = (uint8_t)(bgra[3] / 4);
#endif
        src += 4;
        dst += 1;
    }
}

void RenderingContext::Clear(bool colorBuffer, bool depthBuffer)
{
    if(colorBuffer)
    {
        if(_antiAliasingMode == AntiAliasingMode::Off
        || (_antiAliasingMode == AntiAliasingMode::MSAA_4X && _rasterizationMode == RasterizationMode::Scanline))
            _colorBuffer.Fill(_clearColor);
        else
            _aaBuffer.Fill(_clearColor);
    }

    if(depthBuffer)
        _depthBuffer.Fill(0);
}

void RenderingContext::Present()
{
    RECT tmp;
    GetClientRect(_hWndTarget, &tmp);
    Rect window(tmp.left, tmp.top, tmp.right - tmp.left, tmp.bottom - tmp.top);

    Rect rc(0, 0, _width, _height);
    rc.FitInto(window);

    struct BITMAPINFOEX : public BITMAPINFO {
        // extend bmiColors[1] to make room for color channel masks
        RGBQUAD bmiColorsExt[3];
    } bmi;

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = _width;
    bmi.bmiHeader.biHeight = -(int)_height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    StretchDIBits(_hDCTarget, rc.x, rc.y, rc.w, rc.h, 0, 0, _width, _height, _colorBuffer.data(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS, SRCCOPY);
    //SetDIBitsToDevice(hDC, 0, 0, _width, _height, 0, 0, 0, _height, _colorBuffer.data(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
}
