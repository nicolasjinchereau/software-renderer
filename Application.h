/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <string>
using namespace std;

enum KeyCode : int
{
    UpArrow,
    DownArrow,
    LeftArrow,
    RightArrow,
    Space = ' ',
    A = 'a',
    B = 'b',
    C = 'c',
    D = 'd',
    E = 'e',
    F = 'f',
    G = 'g',
    H = 'h',
    I = 'i',
    J = 'j',
    K = 'k',
    L = 'l',
    M = 'm',
    N = 'n',
    O = 'o',
    P = 'p',
    Q = 'q',
    R = 'r',
    S = 's',
    T = 't',
    U = 'u',
    V = 'v',
    W = 'w',
    X = 'x',
    Y = 'y',
    Z = 'z',
    Num0 = '0',
    Num1 = '1',
    Num2 = '2',
    Num3 = '3',
    Num4 = '4',
    Num5 = '5',
    Num6 = '6',
    Num7 = '7',
    Num8 = '8',
    Num9 = '9',
    Unsupported = 1 << 31,
};

class Application
{
    string _windowClass = "App Window";
    string _windowTitle;
    int _clientWidth = 0;
    int _clientHeight = 0;
    int _windowWidth = 0;
    int _windowHeight = 0;
    int _pointersDown = 0;
    HWND _window = NULL;
    uint32_t _wakeAt;
public:
    Application(string windowTitle, int clientWidth, int clientHeight);
    int Run();

    uintptr_t nativeWindowHandle() const;

    string windowTitle() const;
    void setWindowTitle(const string& title);

    int clientWidth() const;
    int clientHeight() const;

    int windowWidth() const;
    int windowHeight() const;

    void SleepFor(float seconds);

    virtual void OnInitialize(){}
    virtual bool OnUpdate(){ return true; }
    virtual void OnTerminate(){}
    virtual void OnKeyDown(KeyCode key){}
    virtual void OnKeyUp(KeyCode key){}
    virtual void OnPointerDown(float x, float y, int id){}
    virtual void OnPointerMove(float x, float y, int id){}
    virtual void OnPointerUp(float x, float y, int id){}
private:
    KeyCode TranslateKey(int key);
    bool CreateAppWindow();
    void DestroyAppWindow();
    bool UpdateAppWindow();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
