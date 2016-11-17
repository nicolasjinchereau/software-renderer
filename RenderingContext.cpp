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

RenderingContext::RenderingContext(Application* app, uint32_t width, uint32_t height, size_t threadCount)
{
    this->_hWndTarget = (HWND)app->nativeWindowHandle();

    _width = width;
    _height = height;

    _depthBuffer.reset(AlignedAlloc<float>(_width * _height, 16));
    _colorBuffer.reset(AlignedAlloc<uint32_t>(_width * _height, 16));
    _clearColor = Color::clear;
    _rasterizationMode = RasterizationMode::Halfspace;
    _mipmapsEnabled = true;
    
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
                    tmp[i].position.x = (tmp[i].position.x + 1.0f) * 0.5f * (float)_width + 0.5f;
                    tmp[i].position.y = (tmp[i].position.y + 1.0f) * 0.5f * (float)_height + 0.5f;
                    tmp[i].position.y = (float)_height - tmp[i].position.y - 1.0f;
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
            
            uintptr_t offset = _shaderStates.size();
            ShaderVisitor vistor(&_shaderStates);
            obj->shader->Accept(&vistor);

            _drawCalls.push_back(DrawCall{ start, end, obj.get(), (Shader*)offset });
        }
    }

    for(auto& call : _drawCalls) {
        uintptr_t offset = (uintptr_t)call.shader;
        call.shader = (Shader*)(_shaderStates.data() + offset);
    }

    size_t threadCount = _renderThreads.size();
    int segment = _height / threadCount;
    int lastseg = _height - segment * (threadCount - 1);

    size_t i = 0;
    for( ; i < threadCount - 1; ++i)
        _renderThreads[i]->Execute(this, Rect(0, segment * i, _width, segment));

    if(i < threadCount)
        _renderThreads[i]->Execute(this, Rect(0, segment * i, _width, lastseg));

    for(size_t j = 0; j < threadCount; ++j)
        _renderThreads[j]->Wait();

    for(auto& call : _drawCalls)
        call.shader->~Shader();

    _drawCalls.clear();
    _shaderStates.clear();
    _cverts.clear();
}

int RenderingContext::ClipDepth(Vertex (&verts)[9], int count)
{
    Vertex tmp[9];
    int newCount = 0;

    // clip near plan
    for(int i = 0; i < count; ++i)
    {
        Vertex &p0 = verts[i];
        Vertex &p1 = verts[(i + 1) % count];

        bool in0 = p0.position.z >= -p0.position.w;
        bool in1 = p1.position.z >= -p1.position.w;

        if(in0 != in1)
        {
            float t = (-p0.position.w - p0.position.z) / (p1.position.z - p0.position.z + p1.position.w - p0.position.w);
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
            tmp[newCount++].position.x = 0.0f;
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

        float right = (float)(_width - 1);
        bool in0 = p0.position.x <= right;
        bool in1 = p1.position.x <= right;
        
        if(in0 != in1)
        {
            float t = (right - p0.position.x) / (p1.position.x - p0.position.x);
            verts[newCount] = p0 + (p1 - p0) * t;
            verts[newCount++].position.x = (float)(_width - 1);
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
            tmp[newCount++].position.y = 0.0f;
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

        float bot = (float)(_height - 1);
        bool in0 = p0.position.y <= bot;
        bool in1 = p1.position.y <= bot;
        
        if(in0 != in1)
        {
            float t = (bot - p0.position.y) / (p1.position.y - p0.position.y);
            verts[newCount] = p0 + (p1 - p0) * t;
            verts[newCount++].position.y = (float)(_height - 1);
        }

        if(in1)
        {
            verts[newCount++] = p1;
        }
    }

    return newCount;
}

void RenderingContext::ExtrapolatePlane(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                        const Vec2& corner00, const Vec2& corner01, const Vec2& corner10,
                                        Vertex& v00, Vertex& v01, Vertex& v10)
{
    Vec2 a = v0.position;
    Vec2 b = v1.position;
    Vec2 c = v2.position;

    Vec2 e0 = b - a;
    Vec2 e1 = c - a;
    Vec2 e2a = corner00 - a;
    Vec2 e2b = corner01 - a;
    Vec2 e2c = corner10 - a;

    float d00 = e0.Dot(e0);
    float d01 = e0.Dot(e1);
    float d11 = e1.Dot(e1);
    float d20a = e2a.Dot(e0);
    float d20b = e2b.Dot(e0);
    float d20c = e2c.Dot(e0);
    float d21a = e2a.Dot(e1);
    float d21b = e2b.Dot(e1);
    float d21c = e2c.Dot(e1);

    float denom = d00 * d11 - d01 * d01;
    float num = 1.0f / denom;

    float va = (d11 * d20a - d01 * d21a) * num;
    float wa = (d00 * d21a - d01 * d20a) * num;
    float ua = 1.0f - va - wa;
    v00 = v0 * ua + v1 * va + v2 * wa;

    float vb = (d11 * d20b - d01 * d21b) * num;
    float wb = (d00 * d21b - d01 * d20b) * num;
    float ub = 1.0f - vb - wb;
    v01 = v0 * ub + v1 * vb + v2 * wb;

    float vc = (d11 * d20c - d01 * d21c) * num;
    float wc = (d00 * d21c - d01 * d20c) * num;
    float uc = 1.0f - vc - wc;
    v10 = v0 * uc + v1 * vc + v2 * wc;
}

float RenderingContext::CalcMipLevel(const Vertex& fCurr, const Vertex& xNext, const Vertex& yNext, const Vec2& texSize, float mipBias, int mipCount)
{
    if(!_mipmapsEnabled)
        return 0.0f;

    Vec2 uv00 = fCurr.texcoord * (1.0f / fCurr.position.w);
    float uv01 = xNext.texcoord.x / xNext.position.w;
    float uv10 = yNext.texcoord.y / yNext.position.w;
    Vec2 uvDt = Vec2(uv01 - uv00.x, uv10 - uv00.y).Scale(texSize);
    
    float mipLevel = 0.5f * Math::Log2(max(uvDt.x * uvDt.x, uvDt.y * uvDt.y));
    return Math::Clamp(mipLevel + mipBias, 0.0f, mipCount - 1.0f);
}

void RenderingContext::RasterizeHalfSpace(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall)
{
    Texture* tex = drawCall->obj->texture.get();
    Shader* shader = drawCall->shader;
    bool backfaceCullingEnabled = drawCall->obj->backfaceCullingEnabled;

    Vec2 sv1 = v0.position;
    Vec2 sv2 = v1.position;
    Vec2 sv3 = v2.position;
    
    int minx = Math::Ceil(Math::Min(sv1.x, sv2.x, sv3.x));
    int miny = Math::Ceil(Math::Min(sv1.y, sv2.y, sv3.y));
    int maxx = Math::Ceil(Math::Max(sv1.x, sv2.x, sv3.x));
    int maxy = Math::Ceil(Math::Max(sv1.y, sv2.y, sv3.y));
    
    minx = Math::Clamp(minx, rect.x, rect.x + rect.w);
    maxx = Math::Clamp(maxx, rect.x, rect.x + rect.w);
    miny = Math::Clamp(miny, rect.y, rect.y + rect.h);
    maxy = Math::Clamp(maxy, rect.y, rect.y + rect.h);

    if(maxx - minx < 1 || maxy - miny < 1)
        return;

    Vertex v00;
    Vertex v01;
    Vertex v10;

    ExtrapolatePlane(v0, v1, v2,
                    {(float)minx, (float)miny},
                    {(float)maxx, (float)miny},
                    {(float)minx, (float)maxy},
                    v00, v01, v10);

    Vertex hEdge = v01 - v00;
    Vertex vEdge = v10 - v00;
    float dx = 1.0f / hEdge.position.x;
    float dy = 1.0f / vEdge.position.y;
    Vertex xDelta = hEdge * dx;
    Vertex yDelta = vEdge * dy;

    Vec4 Dx(sv1.x - sv2.x,
            sv2.x - sv3.x,
            sv3.x - sv1.x, 0);

    Vec4 Dy(sv1.y - sv2.y,
            sv2.y - sv3.y,
            sv3.y - sv1.y, 0);

    Vec4 C(Dy.x * sv1.x - Dx.x * sv1.y,
           Dy.y * sv2.x - Dx.y * sv2.y,
           Dy.z * sv3.x - Dx.z * sv3.y, 0);

    if(Dy.x < 0 || (abs(Dy.x) < Math::FloatTolerance && Dx.x > 0)) C.x++;
    if(Dy.y < 0 || (abs(Dy.y) < Math::FloatTolerance && Dx.y > 0)) C.y++;
    if(Dy.z < 0 || (abs(Dy.z) < Math::FloatTolerance && Dx.z > 0)) C.z++;

    Vec4 Cy = C + Dx * (float)miny - Dy * (float)minx;
    
    Vertex yv = v00;

    Vec2 texSize = tex->size();
    float mipBias = tex->mipmapBias();
    int mipCount = tex->mipmapCount();
    
    for(int y = miny; y < maxy; y++)
    {
        Vec4 Cx = Cy;

        Vertex xv;
        uint32_t *colorBuffer = nullptr;
        float *depthBuffer = nullptr;

        int x = minx;

        for( ; x < maxx; ++x)
        {
            if(Cx.x > 0 && Cx.y > 0 && Cx.z > 0 || (!backfaceCullingEnabled && Cx.x < 0 && Cx.y < 0 && Cx.z < 0))
            {
                uint32_t offset = y * _width + x;
                colorBuffer = _colorBuffer.get() + offset;
                depthBuffer = _depthBuffer.get() + offset;

                xv = yv + xDelta * (float)(x - minx);
                break;
            }

            Cx -= Dy;
        }

        for( ; x < maxx; ++x)
        {
            if(!(Cx.x > 0 && Cx.y > 0 && Cx.z > 0 || (!backfaceCullingEnabled && Cx.x < 0 && Cx.y < 0 && Cx.z < 0)))
                break;

            if(xv.position.w > *depthBuffer)
            {
                float mipLevel = CalcMipLevel(xv, xv + xDelta, xv + yDelta, texSize, mipBias, mipCount);
                Vertex frag = xv / xv.position.w;
                frag.normal.Normalize();

                bool discard = false;
                Color output = Color::Clamp(shader->ProcessPixel(frag, mipLevel, discard));
                if(!discard)
                {
                    *colorBuffer = output;
                    *depthBuffer = xv.position.w;
                }
            }

            ++colorBuffer;
            ++depthBuffer;
            xv += xDelta;
            Cx -= Dy;
        }
        
        yv += yDelta;
        Cy += Dx;
    }
}

void RenderingContext::RasterizeScanline(const Rect& rect, const Vertex& _v0, const Vertex& _v1, const Vertex& _v2, DrawCall* drawCall)
{
    if(drawCall->obj->backfaceCullingEnabled)
    {
        // cross product in screen space -> triangle back-facing?
        Vec2 left(_v0.position.x - _v1.position.x, _v0.position.y - _v1.position.y);
        Vec2 right(_v2.position.x - _v1.position.x, _v2.position.y - _v1.position.y);
        
        if(right.x * left.y - right.y * left.x > 0)
            return;
    }

    Vertex v0 = _v0;
    Vertex v1 = _v1;
    Vertex v2 = _v2;

    // sort vertices by Y coordinate
    if(v2.position.y < v1.position.y) swap(v2, v1);
    if(v2.position.y < v0.position.y) swap(v2, v0);
    if(v1.position.y < v0.position.y) swap(v1, v0);

    // render top half of triangle if non-empty
    if(v1.position.y - v0.position.y >= 0.00001f)
    {
        float t = (v1.position.y - v0.position.y) / (v2.position.y - v0.position.y);
        Vertex center = v0 + (v2 - v0) * t;
        
        if(center.position.x < v1.position.x)
        {
            //   v0
            // cen  v1
            FillTriangle(rect, v0, center, v1, true, drawCall);
        }
        else
        {
            //   v0
            // v1  cen
            FillTriangle(rect, v0, v1, center, true, drawCall);
        }
    }
    
    // render bottom half of triangle if non-empty
    if(v2.position.y - v1.position.y >= 0.00001f)
    {
        float t = (v1.position.y - v0.position.y) / (v2.position.y - v0.position.y);
        Vertex center = v0 + (v2 - v0) * t;

        if(center.position.x < v1.position.x)
        {
            // cen  v1
            //    v2
            FillTriangle(rect, center, v1, v2, false, drawCall);
        }
        else
        {
            // v1  cen
            //   v2
            FillTriangle(rect, v1, center, v2, false, drawCall);
        }
    }
}

void RenderingContext::FillTriangle(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, bool isTop, DrawCall* drawCall)
{
    Texture* tex = drawCall->obj->texture.get();
    Shader* shader = drawCall->shader;
    bool backfaceCullingEnabled = drawCall->obj->backfaceCullingEnabled;

    Vertex p0l;
    Vertex p0r;
    Vertex p1l;
    Vertex p1r;

    Vertex xDelta;
    Vertex yDelta;

    if(isTop)
    {
        //   v0
        // v1  v2
        p0l = v0;
        p0r = v0;
        p1l = v1;
        p1r = v2;

        Vertex hEdge = v2 - v1;
        
        float dx = 1.0f / hEdge.position.x;
        xDelta = hEdge * dx;

        Vec4 p = v0.position - v1.position;
        Vec4 n = xDelta.position;

        float t = p.Dot(n) / n.Dot(n);
        Vertex y2 = v1 + xDelta * t;

        Vertex vEdge = y2 - v0;

        float dy = 1.0f / vEdge.position.y;
        yDelta = vEdge * dy;
    }
    else
    {
        // v0  v1
        //   v2
        p0l = v0;
        p0r = v1;
        p1l = v2;
        p1r = v2;

        Vertex hEdge = v1 - v0;
        
        float dx = 1.0f / hEdge.position.x;
        xDelta = hEdge * dx;

        Vec4 p = v2.position - v0.position;
        Vec4 n = xDelta.position;

        float t = p.Dot(n) / n.Dot(n);
        Vertex y2 = v0 + xDelta * t;

        Vertex vEdge = v2 - y2;
        
        float dy = 1.0f / vEdge.position.y;
        yDelta = vEdge * dy;
    }

    int y0 = (int)p0l.position.y;
    int y1 = min(Math::Ceil(p1l.position.y), (int)_height - 1);
    
    // calculate the vertical deltas down the edges of the triangle
    float dy = 1.0f / (float)(y1 - y0);
    Vertex ldy = (p1l - p0l) * dy;
    Vertex rdy = (p1r - p0r) * dy;
    
    Vec2 texSize = tex->size();
    float mipBias = tex->mipmapBias();
    int mipCount = tex->mipmapCount();

    // fill scanlines
    for(int y = y0; y <= y1; ++y)
    {
        int x = (int)p0l.position.x;
        int end = min((int)Math::Ceil(p0r.position.x), (int)_width - 1);

        uint32_t offset = y * _width + x;
        uint32_t *colorBuffer = _colorBuffer.get() + offset;
        float *depthBuffer = _depthBuffer.get() + offset;

        Vertex xv = p0l;

        for( ; x <= end; ++x)
        {
            if(xv.position.w > *depthBuffer)
            {
                float mipLevel = CalcMipLevel(xv, xv + xDelta, xv + yDelta, texSize, mipBias, mipCount);
                Vertex frag = xv / xv.position.w;
                frag.normal.Normalize();
                bool discard = false;
                Color output = Color::Clamp(shader->ProcessPixel(frag, mipLevel, discard));
                if(!discard)
                {
                    *colorBuffer = output;
                    *depthBuffer = xv.position.w;
                }
            }

            ++colorBuffer;
            ++depthBuffer;
            xv += xDelta;
        }

        p0l += ldy;
        p0r += rdy;
    }
}

void RenderingContext::Clear(bool colorBuffer, bool depthBuffer)
{
    uint32_t pixelCount = _width * _height;
    Color alignedClearColor = _clearColor;
    if(colorBuffer) std::fill(_colorBuffer.get(), _colorBuffer.get() + pixelCount, (uint32_t)alignedClearColor);
    if(depthBuffer) std::fill(_depthBuffer.get(), _depthBuffer.get() + pixelCount, 0.0f);
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

    HDC hDC = GetDC(_hWndTarget);
    StretchDIBits(hDC, rc.x, rc.y, rc.w, rc.h, 0, 0, _width, _height, _colorBuffer.get(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS, SRCCOPY);
    //SetDIBitsToDevice(hDC, 0, 0, _width, _height, 0, 0, 0, _height, _colorBuffer.get(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
    ReleaseDC(_hWndTarget, hDC);
}

