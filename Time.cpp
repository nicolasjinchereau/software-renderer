/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Time.h"

Time& Time::that() {
    static Time instance;
    return instance;
}

Time::Time()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&_frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&_initTime);
    _then = _initTime;
    _lastDeltaTime = 0;
    _lastFPSUpdate = _initTime;
    _frames = 0;
    _lastFps = 0;
}

void Time::update()
{
    long long now;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);

    Time& t = that();
    
    t._lastDeltaTime = (float)(now - t._then) / (float)t._frequency;
    t._then = now;

    if(now - t._lastFPSUpdate >= t._frequency)
    {
        t._lastFPSUpdate = now;
        t._lastFps = t._frames;
        t._frames = 0;
    }

    t._frames++;
}

float Time::time()
{
    Time& t = that();

    long long now;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);

    return (float)(now - t._initTime) / (float)t._frequency;
}

uint32_t Time::fps()
{
    return that()._lastFps;
}

float Time::deltaTime()
{
    return that()._lastDeltaTime;
}
