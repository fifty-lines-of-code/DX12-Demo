// DX12Demo.cpp : Defines the entry point for the application.

#include "DX12Demo.h"

#include "Helper/Helper.h"

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return DX12Demo::GetDemo()->MsgProc(hwnd, msg, wParam, lParam);
}

DX12Demo* DX12Demo::mDemo = nullptr;

DX12Demo::DX12Demo(HINSTANCE hInstance) : 
    mhAppInst(hInstance),
    mGame(mhAppInst, mClientWidth, mClientHeight, mMainWndCaption)
{
    ENGINE_ASSERT(mDemo == nullptr, L"Demo should be nullptr");
    mDemo = this;
}

DX12Demo::~DX12Demo() {}

bool DX12Demo::Initialize() {
    if (!InitMainWindow()) { return false; }

    if (!InitializeGame()) { return false; }

    return true;
}

bool DX12Demo::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = mhAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, mClientWidth, mClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
    if (!mhMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(mhMainWnd, SW_SHOW);
    UpdateWindow(mhMainWnd);

    return true;
}

bool DX12Demo::InitializeGame() {

    return mGame.Initialize(mhMainWnd);
}

int DX12Demo::Run() {
    return mGame.Run();
}

DX12Demo* DX12Demo::GetDemo() { return mDemo; }

LRESULT DX12Demo::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return mGame.MsgProc(hwnd, msg, wParam, lParam);
}