/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <memory>
#include <exception>
#include "Math.h"
using namespace std;

enum class TargaType : uint8_t
{
    None = 0,
    ColorMapped = 1,    // 0b1
    TrueColor = 2,      // 0b2
    Grayscale = 3,      // 0b3
    ColorMappedRLE = 9, // 0b1001
    TrueColorRLE = 10,  // 0b1010
    GrayscaleRLE = 11   // 0b1011
};

struct TargaHeader
{
    uint8_t idLength;        // length of image ID section
    uint8_t colorMapType;    // 1<<0 set if color map present
    TargaType imageType;
    uint16_t colorMapStartIndex;
    uint16_t colorMapLength;
    uint8_t colorMapBitDepth;
    uint16_t imageOriginX;
    uint16_t imageOriginY;
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t imageBitDepth;
    uint8_t imageDescriptor; // alpha depth(0-3), unused(4), upper-left-origin(5), interleaving(6-7)

    static TargaHeader Unpack(uint8_t (&bytes)[18])
    {
        return {
            bytes[0],
            bytes[1],
            (TargaType)bytes[2],
            (uint16_t)(bytes[4] << 8 | bytes[3]),
            (uint16_t)(bytes[6] << 8 | bytes[5]),
            bytes[7],
            (uint16_t)(bytes[9]  << 8 | bytes[8]),
            (uint16_t)(bytes[11] << 8 | bytes[10]),
            (uint16_t)(bytes[13] << 8 | bytes[12]),
            (uint16_t)(bytes[15] << 8 | bytes[14]),
            bytes[16],
            bytes[17],
        };
    }
};

class TargaReadError : public std::runtime_error
{
public:
    TargaReadError() : runtime_error("Failed to read image data."){}
};

class TargaImage
{
public:
    unique_ptr<Color32[]> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;

    TargaImage(unique_ptr<Color32[]>&& pixels, int width, int height, int channels)
        : pixels(std::forward<unique_ptr<Color32[]>>(pixels)), width(width), height(height), channels(channels){}

    static TargaImage Load(const string& filename)
    {
        ifstream fin(filename, ios::in | ios::binary);
        if(!fin)
            throw runtime_error("Failed to load file.");

        auto sz = sizeof(TargaHeader);

        uint8_t hdrBytes[18];
        if(!fin.read((char*)hdrBytes, sizeof(hdrBytes)))
            throw TargaReadError();

        TargaHeader hdr = TargaHeader::Unpack(hdrBytes);

        bool hasColorMap = (hdr.colorMapType & 1) != 0;
        TargaType type = hdr.imageType;
        int bytesPerPixel = hdr.imageBitDepth / 8;
        
        if(type != TargaType::TrueColor
           && type != TargaType::TrueColorRLE)
            throw runtime_error("Invalid file format. Only true-color TGA files are supported.");

        if(hdr.imageBitDepth != 24
        && hdr.imageBitDepth != 32)
            throw runtime_error("Invalid file format. Only 24 and 32 bit files are supported.");

        if(hdr.idLength > 0)
            fin.ignore(hdr.idLength);

        if(hdr.colorMapLength > 0)
            fin.ignore(hdr.colorMapLength * (hdr.colorMapBitDepth / 8));

        int totalPixels = hdr.imageWidth * hdr.imageHeight;
        unique_ptr<Color32[]> data = make_unique<Color32[]>(totalPixels);

        if(type == TargaType::TrueColor)
        {
            unique_ptr<uint8_t[]> tmp = make_unique<uint8_t[]>(totalPixels * bytesPerPixel);
            if(!fin.read((char*)tmp.get(), totalPixels * bytesPerPixel))
                throw TargaReadError();

            int count = 0;

            for(int i = 0; i < totalPixels; ++i)
            {
                uint8_t bgra[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
                memcpy(bgra, &tmp[i * bytesPerPixel], bytesPerPixel);
                data[count++] = { bgra[2], bgra[1], bgra[0], bgra[3] };
            }
        }
        else if(type == TargaType::TrueColorRLE)
        {
            int count = 0;

            while(count < totalPixels)
            {
                uint8_t chunkHdr;
                if(!fin.read((char*)&chunkHdr, 1))
                    throw TargaReadError();

                bool isRLE = (chunkHdr & 0b10000000) != 0;
                int chunkLength = (chunkHdr & 0b01111111) + 1;

                if(isRLE)
                {
                    uint8_t bgra[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
                    if(!fin.read((char*)bgra, bytesPerPixel))
                        throw TargaReadError();

                    Color32 pixel = { bgra[2], bgra[1], bgra[0], bgra[3] };
                    for(int i = 0; i < chunkLength; ++i)
                        data[count++] = pixel;
                }
                else
                {
                    uint8_t tmp[128 * 4];
                    if(!fin.read((char*)tmp, bytesPerPixel * chunkLength))
                        throw TargaReadError();

                    uint8_t* src = tmp;

                    for(int i = 0; i < chunkLength; ++i)
                    {
                        uint8_t bgra[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
                        std::copy(src, src + bytesPerPixel, bgra);
                        data[count++] = { bgra[2], bgra[1], bgra[0], bgra[3] };
                        src += bytesPerPixel;
                    }
                }
            }
        }

        bool vflipped = (hdr.imageDescriptor & 0b10000) == 0;
        if(vflipped)
        {
            int h = hdr.imageHeight;
            int w = hdr.imageWidth;

            for(size_t y1 = 0, ye = h / 2;
                y1 < ye; ++y1)
            {
                size_t y2 = h - y1 - 1;

                uint32_t *r1_beg = (uint32_t*)data.get() + y1 * w;
                uint32_t *r1_end = r1_beg + w;

                uint32_t *r2_beg = (uint32_t*)data.get() + y2 * w;
                uint32_t *r2_end = r2_beg + w;

                while(r1_beg != r1_end)
                {
                    uint32_t p = *r1_beg;
                    *r1_beg = *r2_beg;
                    *r2_beg = p;

                    ++r1_beg;
                    ++r2_beg;
                }
            }
        }

        // ignore extension section
        // ignore footer section

        return { std::forward<unique_ptr<Color32[]>>(data), hdr.imageWidth, hdr.imageHeight, bytesPerPixel };
    }
};
