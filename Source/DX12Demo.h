#pragma once

#include "Engine/Engine.h"
#include <memory>
#include "Game Timer/GameTimer.h"

// in debug mode let's us know about any memory we are leaking
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

class Game;
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
    std::unique_ptr<Game> mGame;

    HINSTANCE mhAppInst = nullptr; // application instance handle
    HWND      mhMainWnd = nullptr; // main window handle
    UINT mClientWidth = 1280;
    UINT mClientHeight = 720;
    const std::wstring mMainWndCaption = L"DX12 Demo";

    bool InitializeGame();
    bool InitMainWindow();
};