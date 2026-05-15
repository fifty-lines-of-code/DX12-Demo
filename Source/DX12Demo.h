#pragma once

#include "Engine/Engine.h"
#include <memory>
#include "Game Timer/GameTimer.h"

// in debug mode let's us know about any memory we are leaking
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

class GameTimer;

class DX12Demo {
public:
    DX12Demo(HINSTANCE hInstance);
    DX12Demo(const DX12Demo& rhs) = delete;
    DX12Demo& operator=(const DX12Demo& rhs) = delete;
    ~DX12Demo();

    bool Initialize();
    int Run();

    static DX12Demo* GetDemo();

    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private: 
    static DX12Demo* mDemo;
    std::unique_ptr<Engine> mEngine;
    GameTimer mTimer;

    HINSTANCE mhAppInst = nullptr; // application instance handle
    HWND      mhMainWnd = nullptr; // main window handle
    UINT mClientWidth = 1280;
    UINT mClientHeight = 720;
    std::wstring mMainWndCaption = L"DX12 Demo";

    bool      mAppPaused = false;  // is the application paused?
    bool      mMinimized = false;  // is the application minimized?
    bool      mMaximized = false;  // is the application maximized?
    bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

    bool InitializeEngine();
    bool InitMainWindow();
    void Update();
    void Draw();
};