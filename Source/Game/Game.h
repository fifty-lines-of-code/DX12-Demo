#pragma once

#include "../Game Timer/GameTimer.h"
#include <memory>
#include <wtypes.h>
#include <string>

class Engine;

class Game {
public:
	Game(HINSTANCE hInstance, int clientWidth, int clientHeight, std::wstring caption);
	bool Initialize(HWND hwnd);
	~Game();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int Run();

private:
	HWND mMainHwnd;
	UINT mClientWidth;
	UINT mClientHeight;
	const std::wstring mMainWndCaption;
	std::unique_ptr<Engine> mEngine;
	GameTimer mTimer;

	// todo: move below to GameState
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

private:
	void Update();
	void Draw();
};