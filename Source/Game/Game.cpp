#include "Game.h"

#include "../Engine/Camera/Camera.h"
#include "../Engine/Debug System/DebugSystem.h"
#include "../Engine/World Manager/Scene Manager/Entity/Entity.h"

Game::Game(HINSTANCE hInstance, int windowedClientWidth, int windowedClientHeight, const std::wstring caption) :
	mhMainWnd(nullptr),
	mMainWndCaption(caption),
	mEngineCore(hInstance, mMainWndCaption),
	mGameState(GameState(windowedClientWidth, windowedClientHeight))
{}

Game::~Game() {}

bool Game::Initialize(HWND hwnd) {
	mhMainWnd = hwnd;

	// get the fullscreen dimensions
	CalculateFullscreenDimensions();

	// get window width and height
	bool isFullscreen = mGameState.GetIsFullscreen();
	UINT width = isFullscreen ? mGameState.GetFullscreenClientWidth() : mGameState.GetWindowedClientWidth();
	UINT height = isFullscreen ? mGameState.GetFullscreenClientHeight() : mGameState.GetWindowedClientHeight();

	// init the engine core
	if (!mEngineCore.Initialize(hwnd, width, height)) { return false; }

	// set window to fullscreen if we need to
	if (isFullscreen) {
		SetFullscreen();
	}

	// call resize on the engine to finalize init
	mEngineCore.OnResize(width, height);

	return true;
 }

int Game::Run() {
	MSG msg = { 0 };

	mTimer.Reset();

	while (msg.message != WM_QUIT) {
		// 1. Process ALL pending Windows messages immediately
		// This keeps the window snappy and responsive.
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
		}

		// 2. Tick the clock
		mTimer.Tick();

		// 3. Handle Paused State
		if (mGameState.GetIsPaused()) {
			Sleep(100);
			continue; // Skip the rest of the loop
		}

		// 4. THE CORE VISION
		// This is where your breathing math lives
		Update();

		// 5. DRAWING
		// We call Draw(), which tells the Renderer to work its magic
		Draw();
	}

	return (int)msg.wParam;
}

void Game::CalculateFullscreenDimensions() {
	// identify the monitor of the window
	HMONITOR hMonitor = MonitorFromWindow(mhMainWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mInfo = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMonitor, &mInfo);

	// get width, height
	int width = mInfo.rcMonitor.right - mInfo.rcMonitor.left;
	int height = mInfo.rcMonitor.bottom - mInfo.rcMonitor.top;

	mGameState.SetFullscreenClientWidth(width);
	mGameState.SetFullscreenClientHeight(height);
}

void Game::SetFullscreen() {

	// make sure we have a main handle
	assert(mhMainWnd != nullptr);

	// strip all borders, captions, resize styles
	SetWindowLongPtr(mhMainWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

	// position and scale the window
	SetWindowPos(
		mhMainWnd,
		HWND_TOP,
		0,
		0,
		mGameState.GetFullscreenClientWidth(),
		mGameState.GetFullscreenClientHeight(),
		SWP_NOOWNERZORDER | SWP_FRAMECHANGED
	);
}

void Game::SetWindowed() {
	// restore standard window decorations (borders, title bar, close buttons)
	SetWindowLongPtr(
		mhMainWnd,
		GWL_STYLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE
	);

	// calculate the Window Rect based on your desired client size
	RECT windowRect = { 0, 0, mGameState.GetWindowedClientWidth(), mGameState.GetWindowedClientHeight()};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int physicalWidth = windowRect.right - windowRect.left;
	int physicalHeight = windowRect.bottom - windowRect.top;

	// center the window on the user's primary screen using the new dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posX = (screenWidth - physicalWidth) / 2;
	int posY = (screenHeight - physicalHeight) / 2;

	// position the window and force a frame style update
	SetWindowPos(
		mhMainWnd,
		HWND_NOTOPMOST,
		posX,
		posY,
		physicalWidth,
		physicalHeight,
		SWP_FRAMECHANGED | SWP_SHOWWINDOW
	);
}

void Game::CalculateFrameStats() {
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	static int previousFPS = 0;
	static float previousMSPF = 0.f;

	frameCnt++;

	std::string fpsStr = 
		"FPS:" + 
		std::to_string(previousFPS) + 
		",MSPF:" +
		std::to_string(previousMSPF) +
		"\n";
	Engine::DebugSystem::DebugSystem::GetInstance().LogText(fpsStr);

	// Compute averages over one second period.
	if ((mTimer.GetTotalTime() - timeElapsed) >= 1.0f)
	{
		// fps = frameCnt / 1
		float mspf = 1000.0f / frameCnt;

		// create wstring for window
		std::wstring wfpsStr =
			L"FPS: " + 
			std::to_wstring(frameCnt) +
			L" MSPF: " +
			std::to_wstring(mspf);

		std::wstring windowText = mMainWndCaption + wfpsStr;

		SetWindowText(mhMainWnd, windowText.c_str());

		// Reset for next average.
		previousFPS = frameCnt;
		previousMSPF = mspf;
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

LRESULT Game::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	UINT clientWidth;
	UINT clientHeight;
	bool isFullscreen = false;

	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mGameState.SetIsPaused(true);
			mTimer.Stop();
		}
		else
		{
			mGameState.SetIsPaused(false);
			mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		clientWidth = LOWORD(lParam);
		clientHeight = HIWORD(lParam);

		if (wParam != SIZE_MINIMIZED && clientWidth > 0 && clientHeight > 0) {
			if (mGameState.GetIsFullscreen()) {
				isFullscreen = true;
			}
			else {
				mGameState.SetWindowedClientWidth(clientWidth);
				mGameState.SetWindowedClientHeight(clientHeight);
			}
		}

		if (wParam == SIZE_MINIMIZED)
		{
			mGameState.SetIsPaused(true);
			mGameState.SetIsMinimized(true);
			mGameState.SetIsMaximized(false);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			mGameState.SetIsPaused(false);
			mGameState.SetIsMinimized(false);
			mGameState.SetIsMaximized(true);
			
			mEngineCore.OnResize(clientWidth, clientHeight);
		}
		else if (wParam == SIZE_RESTORED)
		{
			// Restoring from minimized state?
			if (mGameState.GetIsMinimized())
			{
				mGameState.SetIsPaused(false);
				mGameState.SetIsMinimized(false);
				mEngineCore.OnResize(clientWidth, clientHeight);
			}
			// Restoring from maximized state?
			else if (mGameState.GetIsMaximized())
			{
				mGameState.SetIsPaused(false);
				mGameState.SetIsMaximized(false);
				mEngineCore.OnResize(clientWidth, clientHeight);
			}
			else if (mGameState.GetIsResizing())
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				mEngineCore.OnResize(clientWidth, clientHeight);
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mGameState.SetIsPaused(true);
		mGameState.SetIsResizing(true);
		mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mGameState.SetIsPaused(false);
		mGameState.SetIsResizing(false);
		mTimer.Start();
		// here we assume were windowed
		mEngineCore.OnResize(mGameState.GetWindowedClientWidth(), mGameState.GetWindowedClientHeight());
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if (wParam == 'F') {
			bool isFullscreen = mGameState.GetIsFullscreen();
			mGameState.SetIsFullscreen(!isFullscreen);

			// if we're windowed, go fullscreen
			if (!isFullscreen) {
				SetFullscreen();
			}
			else {
				SetWindowed();
			}
		}
		else if (wParam == 'D') {
			bool isDrawingDebugState = mGameState.GetIsDrawingDebugState();
			mGameState.SetIsDrawingDebugState(!isDrawingDebugState);
		}

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Game::Update() {
	// update stats
	CalculateFrameStats();

	// get delta time
	float deltaTime = mTimer.GetDeltaTime();

	// update the engine
	mEngineCore.Update(deltaTime);
}

void Game::Draw() {
	mEngineCore.Draw(mGameState.GetIsDrawingDebugState());
}