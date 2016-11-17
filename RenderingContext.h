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
using namespace std;

class Application;
class RenderThread;
class Texture;
class Shader;
class Scene;
class SceneObject;

enum class RasterizationMode
{
    Scanline,
    Halfspace
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

    void RasterizeHalfSpace(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void RasterizeScanline(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, DrawCall* drawCall);
    void FillTriangle(const Rect& rect, const Vertex& v0, const Vertex& v1, const Vertex& v2, bool isTop, DrawCall* drawCall);
    float CalcMipLevel(const Vertex& fCurr, const Vertex& xNext, const Vertex& yNext, const Vec2& texSize, float mipBias, int mipCount);
    void ExtrapolatePlane(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                      const Vec2& corner00, const Vec2& corner01, const Vec2& corner10,
                      Vertex& v00, Vertex& v01, Vertex& v10);
private:
    uint32_t _width;
    uint32_t _height;
    RasterizationMode _rasterizationMode;
    bool _mipmapsEnabled;
    Color _clearColor;
    unique_ptr<uint32_t, AlignedDeleter<uint32_t>> _colorBuffer;
    unique_ptr<float, AlignedDeleter<float>> _depthBuffer;
    vector<Vertex, AlignedAllocator<Vertex, 16>> _xverts;
    vector<Vertex, AlignedAllocator<Vertex, 16>> _cverts;
    vector<DrawCall, AlignedAllocator<DrawCall, 16>> _drawCalls;
    vector<unique_ptr<RenderThread>> _renderThreads;
    vector<uint8_t, AlignedAllocator<uint8_t, 16>> _shaderStates;
    HWND _hWndTarget;
};
