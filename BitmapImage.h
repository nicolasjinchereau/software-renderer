/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <fstream>
#include <Windows.h>
#include "Math.h"
using namespace std;

class BitmapImage
{
public:
    unique_ptr<Color32[]> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;

    BitmapImage(unique_ptr<Color32[]>&& pixels, int width, int height, int channels)
        : pixels(std::forward<unique_ptr<Color32[]>>(pixels)), width(width), height(height), channels(channels){}

    static BitmapImage Load(const string& filename)
    {
        ifstream fin(filename, ios::in | ios::binary);
        if(!fin)
            throw runtime_error("failed to load file");

        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        fin.read((char*)&fileHeader, sizeof(BITMAPFILEHEADER));
        union {
            WORD w;
            char c[2];
        } type = { fileHeader.bfType };

        if(memcmp(type.c, "BM", 2) != 0)
            throw runtime_error("Invalid bitmap format. Only windows bitmaps are supported.");

        fin.read((char*)&infoHeader, sizeof(BITMAPINFOHEADER));

        if(infoHeader.biSize != sizeof(BITMAPINFOHEADER))
            throw runtime_error("Failed to read bitmap info header. File may be corrupt.");

        if(infoHeader.biCompression > 0)
            throw runtime_error("Invalid bitmap format. Compression is not supported.");

        if(infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32)
            throw runtime_error("Invalid bitmap format. Only 24 bit bitmaps are supported.");

        int bytesPerPixel = infoHeader.biBitCount >> 3;
        auto w = infoHeader.biWidth;
        auto h = infoHeader.biHeight;

        int stride = w * bytesPerPixel;
        int padding = (stride % 4) > 0 ? 4 - (stride % 4) : 0;

        unique_ptr<Color32[]> ret = make_unique<Color32[]>(w * h);

        DWORD dataOffset = fileHeader.bfOffBits;
        size_t pos = (size_t)fin.tellg();

        if(pos < dataOffset)
            fin.ignore(dataOffset - pos);

        Color32* r = ret.get() + w * h - w;

        for(int y = h - 1; y >= 0; --y)
        {
            Color32* p = r;

            for(int x = 0; x < w; ++x)
            {
                ColorBGRA bgra;

                fin.read((char*)&bgra, bytesPerPixel);
                if(bytesPerPixel == 3)
                    bgra.a = 255;

                *p++ = Color32(bgra);
            }

            if(padding > 0)
                fin.ignore(padding);

            r -= w;
        }

        return { std::forward<unique_ptr<Color32[]>>(ret), w, h, bytesPerPixel };
    }
};
