/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Windows.h>
#include <process.h>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <vector>
#include "Math.h"
#include "Mem.h"
#include "Vertex.h"
#include "RenderBuffer.h"
#include "Shader.h"

using namespace std;

class Application;
class RenderThread;
class Texture;
class Shader;
class Scene;
class SceneObject;
class Light;

enum class RasterizationMode
{
    Scanline,
    Halfspace
};

enum class AntiAliasingMode
{
    Off,
    MSAA_4X,
    SSAA_2X,
    SSAA_4X,
};

struct DrawCall
{
    size_t start;
    size_t end;
    SceneObject* obj;
    Shader *shader;
};

class RenderingContext
{    
public:
    friend class RenderThread;

    RenderingContext(Application* app, uint32_t width, uint32_t height, size_t threadCount);
    ~RenderingContext();

    void clearColor(const Color &color);
    Color clearColor() const;

    void rasterizationMode(RasterizationMode mode);
    RasterizationMode rasterizationMode() const;

    void antiAliasingMode(AntiAliasingMode mode);
    AntiAliasingMode antiAliasingMode() const;

    void mipmapsEnabled(bool enabled);
    bool mipmapsEnabled() const;
    
    uint32_t width() const;
    uint32_t height() const;
    HWND targetWindow() const;

    void Clear(bool colorBuffer = true, bool depthBuffer = true);
    void Draw(const shared_ptr<Scene>& scene);
    void Present();

private:
    int ClipDepth(Vertex (&verts)[9], int count);
    int ClipScreen(Vertex (&verts)[9], int count);
    static float CalcMipLevel(const Vec2& uv00, const Vec2& uv01, const Vec2& uv10, const Vec2& texSize, float mipBias, int mipCount);
    void Rasterize(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void RasterizeHalfSpace(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void RasterizeHalfSpaceMSAA(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void RasterizeScanline(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void FillSpans(const Rect& rect, const Vertex& l0, const Vertex& l1, const Vertex& r0, const Vertex& r1, const Vertex& xDelta, const Vertex& yDelta, DrawCall* drawCall);
    void Resolve(const Rect& rect);
    void ResolveSSAA2X(const Rect& rect);
    void ResolveSSAA4X(const Rect& rect);
    void ResolveMSAA4X(const Rect& rect);
private:
    uint32_t _width;
    uint32_t _height;
    uint32_t _renderWidth;
    uint32_t _renderHeight;
    RasterizationMode _rasterizationMode;
    AntiAliasingMode _antiAliasingMode;
    bool _mipmapsEnabled;
    Color _clearColor;
    RenderBuffer<uint32_t> _colorBuffer;
    RenderBuffer<uint32_t> _aaBuffer;
    RenderBuffer<float> _depthBuffer;
    vector<Vertex, AlignedAllocator<Vertex, 16>> _xverts;
    vector<Vertex, AlignedAllocator<Vertex, 16>> _cverts;
    vector<DrawCall, AlignedAllocator<DrawCall, 16>> _drawCalls;
    vector<unique_ptr<RenderThread>> _renderThreads;
    ShaderList _shaders;
    HWND _hWndTarget;
    HDC _hDCTarget;
};
