/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <Windows.h>

class Time
{
    static Time& that();

    long long _frequency;
    long long _initTime;
    long long _then;
    float _lastDeltaTime;

    long long _lastFPSUpdate;
    uint32_t _frames;
    uint32_t _lastFps;

    Time();

public:
    static void update();
    static float time();
    static uint32_t fps();
    static float deltaTime();
};
