/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Application.h"

Application::Application(string windowTitle, int clientWidth, int clientHeight)
{
    _windowTitle = windowTitle;
    _clientWidth = clientWidth;
    _clientHeight = clientHeight;
    _wakeAt = 0;
}

int Application::Run()
{
    CreateAppWindow();

    while(true) {
        if(!UpdateAppWindow()) break;
        if(!OnUpdate()) break;
    }

    DestroyAppWindow();

    return 0;
}

uintptr_t Application::nativeWindowHandle() const {
    return (uintptr_t)_window;
}

string Application::windowTitle() const {
    return _windowTitle;
}

void Application::setWindowTitle(const string& title) {
    _windowTitle = title;
    SetWindowText(_window, _windowTitle.c_str());
}

int Application::clientWidth() const {
    return _clientWidth;
}

int Application::clientHeight() const {
    return _clientHeight;
}

int Application::windowWidth() const {
    return _windowWidth;
}

int Application::windowHeight() const {
    return _windowHeight;
}

KeyCode Application::TranslateKey(int key)
{
    if(isalnum(key))
        return (KeyCode)tolower(key);

    if(isdigit(key) || isspace(key))
        return (KeyCode)key;

    switch(key)
    {
    case VK_UP:
        return KeyCode::UpArrow;
    case VK_DOWN:
        return KeyCode::DownArrow;
    case VK_LEFT:
        return KeyCode::LeftArrow;
    case VK_RIGHT:
        return KeyCode::RightArrow;
    default:
        return (KeyCode)(KeyCode::Unsupported | key);
    }
}

bool Application::CreateAppWindow()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = _windowClass.c_str();
    wcex.hIconSm        = NULL;
    RegisterClassEx(&wcex);

    RECT windowRect = { 0, 0, _clientWidth, _clientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

    _windowWidth = windowRect.right - windowRect.left;
    _windowHeight = windowRect.bottom - windowRect.top;

    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (desktopWidth - _windowWidth) / 2;
    int y = (desktopHeight - _windowHeight) / 2;

    _window = CreateWindow(_windowClass.c_str(), _windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                           x, y, _windowWidth, _windowHeight, NULL, NULL, hInstance, this);

    if(_window == NULL)
    {
        MessageBox(NULL, "failed to create a window", "Error", NULL);
        return false;
    }

    ShowWindow(_window, SW_SHOW);
    UpdateWindow(_window);

    return true;
}

void Application::DestroyAppWindow()
{
    if(_window)
    {
        DestroyWindow(_window);
        _window = NULL;
    }
}

void Application::SleepFor(float seconds)
{
    _wakeAt = GetTickCount() + (DWORD)(seconds * 1000.0f + 0.5f);
}

bool Application::UpdateAppWindow()
{
    bool shouldSleep = false;

    MSG Msg;
    while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
        if(Msg.message == WM_QUIT) return false;
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    DWORD now = GetTickCount();
    if(now < _wakeAt)
        MsgWaitForMultipleObjects(0, NULL, FALSE, _wakeAt - now, QS_ALLEVENTS);

    return true;
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == WM_CREATE)
    {
        auto cs = (CREATESTRUCT*)lParam;
        auto app = (Application*)cs->lpCreateParams;
        app->_window = hWnd;

        SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)app);
    }

    auto app = (Application*)GetWindowLongPtr(hWnd, GWL_USERDATA);
    if(app != nullptr)
        return app->HandleWindowMessage(hWnd, msg, wParam, lParam);

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Application::HandleWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        this->OnInitialize();
        break;

    case WM_DESTROY:
        this->OnTerminate();
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        this->OnKeyDown(TranslateKey(wParam));
        break;

    case WM_KEYUP:
        this->OnKeyUp(TranslateKey(wParam));
        break;

    case WM_LBUTTONDOWN:
        if(_pointersDown++ == 0) SetCapture(hWnd);
        this->OnPointerDown((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), 0);
        break;

    case WM_RBUTTONDOWN:
        if(_pointersDown++ == 0) SetCapture(hWnd);
        this->OnPointerDown((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), 1);
        break;

    case WM_MOUSEMOVE:
        this->OnPointerMove((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), 0);
        break;

    case WM_LBUTTONUP:
        this->OnPointerUp((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), 0);
        if(--_pointersDown == 0) ReleaseCapture();
        break;

    case WM_RBUTTONUP:
        this->OnPointerUp((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), 1);
        if(--_pointersDown == 0) ReleaseCapture();
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
