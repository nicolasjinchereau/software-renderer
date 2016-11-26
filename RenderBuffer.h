/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <memory>
#include <type_traits>
#include "Mem.h"
using namespace std;

template<typename T>
class RenderBuffer
{
protected:
    unique_ptr<T, AlignedDeleter<T>> _data;
    int _width = 0;
    int _height = 0;
    int _sampleCount = 0;
public:
    RenderBuffer(const RenderBuffer&) = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;

    RenderBuffer(){}
    RenderBuffer(int width, int height, int sampleCount)
        : _data(AlignedAlloc<T>(width * height * sampleCount, 16)),
          _width(width),
          _height(height),
          _sampleCount(sampleCount){}

    RenderBuffer(RenderBuffer&& rb)
        : _data(move(rb._data)),
          _width(rb._width),
          _height(rb._heigh),
          _sampleCount(rb._sampleCount)
    {
        rb._width = 0;
        rb._height = 0;
        rb._sampleCount = 0;
    }

    RenderBuffer& operator=(RenderBuffer&& rb)
    {
        _data = move(rb._data);
        _width = rb._width;
        _height = rb._height;
        _sampleCount = rb._sampleCount;
        rb._width = 0;
        rb._height = 0;
        rb._sampleCount = 0;
        return *this;
    }

    void Resize(int width, int height, int sampleCount)
    {
        if(_width == width && _height == height && _sampleCount == sampleCount)
            return;

        if(width * height * sampleCount > 0)
            _data.reset(AlignedAlloc<T>(width * height * sampleCount, 16));
        else
            _data.reset();

        _width = width;
        _height = height;
        _sampleCount = sampleCount;
    }

    void Clear() {
        std::fill(_data.get(), _data.get() + (_width * _height * _sampleCount), T());
    }

    void Fill(T value) {
        std::fill(_data.get(), _data.get() + (_width * _height * _sampleCount), value);
    }

    T* data() {
        return _data.get();
    }
    
    const T* data() const {
        return _data.get();
    }

    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

    int sampleCount() const {
        return _sampleCount;
    }
    
    int GetSampleOffset(int x, int y, int i) const {
        return (y * _width + x) * _sampleCount + i;
    }

    T& GetSample(int x, int y, int i) {
        return *(_data.get() + GetSampleOffset(x, y, i));
    }

    const T GetSample(int x, int y, int i) const {
        return *(_data.get() + GetSampleOffset(x, y, i));
    }

    template<unsigned X>
    int GetSuperSampleOffset(int x, int y) const
    {
        assert(_sampleCount == X * X);
        constexpr unsigned RECIPROCAL = (0x10000 + X - 1) / X;
        unsigned xx = (x * RECIPROCAL) >> 16;  //   x / X
        unsigned xi = x - xx * X;              //   x % X
        unsigned yy = (y * RECIPROCAL) >> 16;  //   y / X
        unsigned yi = y - yy * X;              //   y % X
        return (yy * _width + xx) * (X * X) + (yi * X + xi);
    }

    template<unsigned X>
    int GetSuperSampleRowOffset(int y) const
    {
        assert(_sampleCount == X * X);
        constexpr unsigned RECIPROCAL = (0x10000 + X - 1) / X;
        unsigned yy = (y * RECIPROCAL) >> 16;   //   y / X
        unsigned yi = y - yy * X;               //   y % X
        return yy * _width * (X * X) + (yi * X);
    }

    template<unsigned X>
    int GetSuperSampleColumnOffset(int x) const
    {
        assert(_sampleCount == X * X);
        constexpr unsigned RECIPROCAL = (0x10000 + X - 1) / X;
        unsigned xx = (x * RECIPROCAL) >> 16;  //   x / X
        unsigned xi = x - xx * X;              //   x % X
        return xx * (X * X) + xi;
    }
};
